/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <sys/socket.h>
#include <sys/un.h>

#include "janusconnector.h"
#include "loggingcategories.h"

#include <QJsonDocument>
#include <QUuid>

JanusConnector::JanusConnector(QObject *parent) : QObject(parent)
{
    m_socket = new QLocalSocket(this);
    typedef void (QLocalSocket:: *errorSignal)(QLocalSocket::LocalSocketError);
    connect(m_socket, static_cast<errorSignal>(&QLocalSocket::error), this, &JanusConnector::onError);
    connect(m_socket, &QLocalSocket::disconnected, this, &JanusConnector::onDisconnected);
    connect(m_socket, &QLocalSocket::readyRead, this, &JanusConnector::onReadyRead);

    // When Janus crashes it will leave the socket in a very broken state which causes QLocalSocket to spin the CPU
    // So let's use a rather short heartbeat to send ping messages and clean things up in case they are not acked.
    m_pingTimer.setInterval(1000);
    connect(&m_pingTimer, &QTimer::timeout, this, &JanusConnector::heartbeat);
}

bool JanusConnector::connectToJanus()
{

    int sock = socket (PF_UNIX, SOCK_SEQPACKET, 0);
    if (sock < 0) {
        qCWarning(dcJanus) << "Failed to create socket";
        return false;
    }

    struct sockaddr_un name;
    name.sun_family = AF_UNIX;
    strncpy (name.sun_path, "/tmp/janusapi", 13);
    name.sun_path[13] = '\0';
    int ret = ::connect(sock, (const sockaddr*)&name, sizeof(name));
    if (ret < 0) {
        qCWarning(dcJanus()) << "Error connecting to socket";
        return false;
    }

    m_socket->setSocketDescriptor(sock);
    m_pingTimer.start();

    return true;
}

void JanusConnector::disconnectFromJanus()
{
    m_socket->close();
    m_pingTimer.stop();
    m_lastUnconfirmedPing = QDateTime();
}

void JanusConnector::createSession(JanusConnector::WebRtcSession *session)
{
    // Open the session
    qCDebug(dcJanus()) << "Creating new janus session:" << session;
    QString transactionId = QUuid::createUuid().toString();
    QVariantMap map;
    map.insert("transaction", transactionId);
    map.insert("janus", "create");
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(map);
    m_pendingRequests.insert(transactionId, session);
    writeToJanus(jsonDoc.toJson());
}

void JanusConnector::sendWebRtcHandshakeMessage(const QString &sessionId, const QVariantMap &message)
{
    if (!m_socket->isOpen()) {
        if (!connectToJanus()) {
            qCWarning(dcJanus()) << "Failed to establish a connection to Janus. Cannot send WebRtcHandshake.";
            return;
        }
    }
    QString messageType = message.value("type").toString();
    QString transactionId = message.value("id").toString();

    // Do we have a session for this transaction?
    if (!m_sessions.contains(sessionId)) {

        // create a new session and queue the actual offer call
        WebRtcSession *session = new WebRtcSession();
        session->sessionId = sessionId;
        if (messageType == "offer") {
            session->offer = message;
            session->offerSent = false;
        } else if (messageType == "trickle") {
            session->trickles.append(message);
        } else {
            qCWarning(dcJanus()) << "Unhandled webrtc handshake message type" << messageType << message;
        }
        m_sessions.insert(sessionId, session);

        if (messageType == "offer") {
            createSession(session);
        }
        return;
    }

    WebRtcSession *session = m_sessions.value(sessionId);
    if (messageType == "offer") {
        session->offer = message;
        session->offerSent = false;
        createSession(session);
    } else if (messageType == "trickle") {
        m_sessions.value(sessionId)->trickles.append(message);
    } else if (messageType == "webrtcup") {
        // If we got the webrtc up from Janus already, directly reply with an ack
        if (session->webRtcConnected) {
            QVariantMap ack;
            ack.insert("id", message.value("id").toString());
            ack.insert("type", "ack");
            emit webRtcHandshakeMessageReceived(session->sessionId, ack);
        } else {
            // otherwise store the request and reply when we get the webrtcup
            session->webRtcUp = message;
        }
    } else if (messageType == "ack") {
        // silence ack's we may get from the other end, janus doesn't need them...
    } else {
        qCWarning(dcJanus()) << "Unhandled message type:" << messageType << message;
    }

    processQueue();
}

bool JanusConnector::sendKeepAliveMessage(const QString &sessionId)
{
    WebRtcSession *session = m_sessions.value(sessionId);
    if (!session) {
        qCWarning(dcJanus()) << "Received a keepalive message for a session we don't know.";
        return false;
    }
    QVariantMap janusMessage;
    janusMessage.insert("janus", "keepalive");
    janusMessage.insert("session_id", session->janusSessionId);
    janusMessage.insert("handle_id", session->janusChannelId);
    janusMessage.insert("transaction", "keepalive");
    writeToJanus(QJsonDocument::fromVariant(janusMessage).toJson(QJsonDocument::Compact));
    return true;
}

void JanusConnector::processQueue()
{
    if (!m_socket->isOpen()) {
        qCWarning(dcJanus()) << "Janus socket not open. Cannot process queue";
        return;
    }

    foreach (WebRtcSession* session, m_sessions) {
        if (session->connectedToJanus) {
            if (!session->offerSent) {
                QVariantMap janusMessage;
                janusMessage.insert("janus", "message");
                janusMessage.insert("transaction", session->offer.value("id").toString());
                janusMessage.insert("session_id", session->janusSessionId);
                janusMessage.insert("handle_id", session->janusChannelId);
                QVariantMap body;
                body.insert("request", "setup");
                janusMessage.insert("body", body);
                janusMessage.insert("jsep", session->offer.value("jsep"));

                m_pendingRequests.insert(session->offer.value("id").toString(), session);

                QJsonDocument jsonDoc = QJsonDocument::fromVariant(janusMessage);
                qCDebug(dcJanus()) << "Sending offer message to session" << session << jsonDoc.toJson();
                writeToJanus(jsonDoc.toJson());
                session->offerSent = true;
                return;
            }
            while (!session->trickles.isEmpty()) {
                QVariantMap input = session->trickles.takeFirst().toMap();
                QVariantMap janusMessage;
                janusMessage.insert("janus", "trickle");
                janusMessage.insert("transaction", input.value("id").toString());
                janusMessage.insert("session_id", session->janusSessionId);
                janusMessage.insert("handle_id", session->janusChannelId);
                janusMessage.insert("candidate", input.value("candidate"));

                m_pendingRequests.insert(input.value("id").toString(), session);

                QJsonDocument jsonDoc = QJsonDocument::fromVariant(janusMessage);
                qCDebug(dcJanus()) << "sending trickle message" << jsonDoc.toJson();
                m_socket->write(jsonDoc.toJson());
                return;
            }
        }
    }
}

void JanusConnector::onDisconnected()
{
    qCDebug(dcJanus) << "Disconnected from Janus" << m_socket->isOpen();
}

void JanusConnector::onError(QLocalSocket::LocalSocketError socketError)
{
    qCWarning(dcJanus) << "Error in janus connection" << socketError << m_socket->errorString();
}

void JanusConnector::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    qCDebug(dcJanusTraffic()) << "incoming data" << data;
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning(dcJanus()) << "Cannot parse packet received by Janus:" << error.error << error.errorString();
        return;
    }

    QVariantMap map = jsonDoc.toVariant().toMap();
    if (map.value("janus").toString() == "error") {
        qCWarning(dcJanus()) << "An error happened in the janus connection:" << map.value("error").toMap().value("reason").toString();
        return;
    }

    if (map.value("janus").toString() == "timeout") {
        quint64 sessionId = map.value("session_id").toLongLong();
        foreach (WebRtcSession *session, m_sessions) {
            if (session->matchJanusSessionId(sessionId)) {
                qCDebug(dcJanus()) << "Session" << session << "timed out. Removing session";
                m_sessions.remove(session->sessionId);
                delete session;
                if (m_sessions.isEmpty()) {
                    disconnectFromJanus();
                }
                return;
            }
        }
        qCWarning(dcJanus()) << "Received a timeout event but don't have a session for it." << data << map.value("session_id").toLongLong();
        return;
    }

    if (map.value("janus").toString() == "webrtcup") {
        quint64 sessionId = map.value("session_id").toLongLong();
        foreach (WebRtcSession *session, m_sessions) {
            if (session->matchJanusSessionId(sessionId)) {
                qCDebug(dcJanus()) << "Session" << session << "is up now!";
                session->webRtcConnected = true;
                if (!session->webRtcUp.isEmpty()) {
                    QVariantMap ack;
                    ack.insert("id", session->webRtcUp.value("id").toString());
                    ack.insert("type", "ack");
                    emit webRtcHandshakeMessageReceived(session->sessionId, ack);
                }
                return;
            }
        }
        qCWarning(dcJanus()) << "Received a webrtcup event but don't have a session for it";
        return;
    }

    if (map.value("janus").toString() == "hangup") {
        quint64 sessionId = map.value("session_id").toLongLong();
        foreach (WebRtcSession *session, m_sessions) {
            if (session->matchJanusSessionId(sessionId)){
                qCDebug(dcJanus()) << "Session" << session << "hangup received. Reason:" << map.value("reason").toString();
                QVariantMap hangup;
                hangup.insert("type", "hangup");
                hangup.insert("reason", map.value("reason").toString());
                emit webRtcHandshakeMessageReceived(session->sessionId, hangup);
                m_sessions.remove(session->sessionId);
                if (m_sessions.isEmpty()) {
                    disconnectFromJanus();
                }
                return;
            }
        }
        qCWarning(dcJanus()) << "Received a hangup message but don't have a session for it";
        return;
    }

    // as of now, everything must be part of a transaction
    if (!map.contains("transaction")) {
        qCWarning(dcJanus) << "Unhandled message from Janus (missing transaction):" << data;
        return;
    }

    QString transactionId = map.value("transaction").toString();
    WebRtcSession *session = m_pendingRequests.value(transactionId);
    if (!session) {
        if (transactionId == "pingety") {
            qCDebug(dcJanus()) << "Received PONG from Janus";
            m_lastUnconfirmedPing = QDateTime();
            return;
        }
        if (transactionId == "keepalive") {
            qCDebug(dcJanus()) << "Keep alive acked by janus.";
            return;
        }
        qCWarning(dcJanus()) << "received a janus message for a session we don't know...";
        return;
    }

    if (session->janusSessionId == -1) {
        // This should be a create session response
        if (map.value("janus").toString() == "success") {
            session->janusSessionId = map.value("data").toMap().value("id").toLongLong();
            // oooohhhkaaay... now, this is gonna be dirty... So, Janus' session id is like a freakin huge number
            // so that QVariant stores it in a double instead of a longlong, which could cause rounding errors when converting it
            // back to to a long. Let's grep the raw data for the parsed session id and if not found, try to correct it one down.
            if (!data.contains(QByteArray::number(session->janusSessionId)) && data.contains(QByteArray::number(session->janusSessionId-1))) {
                session->janusSessionId--;
                qCDebug(dcJanus()) << "corrected session id after rounding error";
            }
            qCDebug(dcJanus()) << "Session" << session << "established";// << data << map;

            createChannel(session);
            return;
        }
        qCWarning(dcJanus()) << "Error establishing session";
        delete m_sessions.take(session->sessionId);
        if (m_sessions.isEmpty()) {
            disconnectFromJanus();
        }
        return;
    }

    qint64 janusSessionId = map.value("session_id").toLongLong();
    if (!session->matchJanusSessionId(janusSessionId)) {
        qCWarning(dcJanus) << "Transaction id and session id not matching!" << session->janusSessionId << "!=" << map.value("session_id").toLongLong();
        return;
    }

    if (session->janusChannelId == -1) {
        if (map.value("janus").toString() == "success") {
            session->janusChannelId = map.value("data").toMap().value("id").toLongLong();
            if (!data.contains(QByteArray::number(session->janusChannelId)) && data.contains(QByteArray::number(session->janusChannelId-1))) {
                session->janusChannelId--;
                qCDebug(dcJanus()) << "Corrected channel id after rounding error";
            }
            qCDebug(dcJanus()) << "Channel for session" << session << "established";// << data << map;
            session->connectedToJanus = true;
            processQueue();
            return;
        }
        qCWarning(dcJanus()) << "Error establishing channel" << session << data;
        return;
    }

    if (map.value("janus").toString() == "event" && map.value("jsep").toMap().value("type").toString() == "answer") {
        qCDebug(dcJanus()) << "Emitting handshake event" << data;
        QVariantMap reply;
        reply.insert("id", transactionId);
        reply.insert("type", "answer");
        reply.insert("jsep", map.value("jsep"));
        emit webRtcHandshakeMessageReceived(session->sessionId, reply);
        return;
    }

    if (map.value("janus").toString() == "ack") {
        QVariantMap ackReply;
        ackReply.insert("id", transactionId);
        ackReply.insert("type", "ack");
        emit webRtcHandshakeMessageReceived(session->sessionId, ackReply);
        return;
    }

    qCWarning(dcJanus()) << "Unhandled Janus message:" << data;
}

void JanusConnector::heartbeat()
{
    if (!m_lastUnconfirmedPing.isNull()) {
        qCWarning(dcJanus()) << "Last ping not echoed by Janus. Seems the connection broke down. Cleaning up...";
        while (!m_sessions.isEmpty()) {
            delete m_sessions.take(m_sessions.keys().first());
        }
        disconnectFromJanus();
        return;
    }
    QVariantMap map;
    map.insert("janus", "ping");
    map.insert("transaction", "pingety");
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(map);
    qCDebug(dcJanus()) << "Sending PING to Janus";
    m_lastUnconfirmedPing = QDateTime::currentDateTime();
    writeToJanus(jsonDoc.toJson());
}

void JanusConnector::createChannel(WebRtcSession *session)
{
    QVariantMap attachPluginMessage;
    attachPluginMessage.insert("janus", "attach");
    attachPluginMessage.insert("session_id", session->janusSessionId);
    QString transactionId = QUuid::createUuid().toString();
    m_pendingRequests.insert(transactionId, session);
    attachPluginMessage.insert("transaction", transactionId);
    attachPluginMessage.insert("plugin", "janus.plugin.guhio");
    attachPluginMessage.insert("opaque_id", "guhio-" + QUuid::createUuid().toString());
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(attachPluginMessage);
    qCDebug(dcJanus()) << "Establishing channel for session" << session->sessionId;
    writeToJanus(jsonDoc.toJson());
}

void JanusConnector::writeToJanus(const QByteArray &data)
{
    if (!m_socket->isOpen() && !connectToJanus()) {
        qCWarning(dcJanus()) << "Error connecting to Janus. Cannot write data to it.";
        return;
    }
    qCDebug(dcJanusTraffic()) << "Writing to janus" << data;
    qint64 count = m_socket->write(data);
    if (count != data.length()) {
        qCWarning(dcJanus()) << "Error writing to Janus.";
        disconnectFromJanus();
        return;
    }
    m_socket->flush();
}

QDebug operator<<(QDebug debug, const JanusConnector::WebRtcSession &session)
{
    debug.nospace() << session.sessionId << " (Janus session: " << session.janusSessionId << " channel: " << session.janusChannelId << " connected to Janus: " << session.connectedToJanus << " WebRTC connected: " << session.webRtcConnected << ") ";
    return debug;
}

QDebug operator<<(QDebug debug, JanusConnector::WebRtcSession *session)
{
    debug.nospace() << *session;
    return debug;
}
