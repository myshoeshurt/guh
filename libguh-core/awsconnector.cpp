#include "awsconnector.h"
#include "loggingcategories.h"

#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QtConcurrent/QtConcurrentRun>

using namespace awsiotsdk;
using namespace awsiotsdk::network;
using namespace awsiotsdk::mqtt;

QHash<quint16, AWSConnector*> AWSConnector::s_requestMap;

AWSConnector::AWSConnector(QObject *parent) : QObject(parent)
{
    connect(this, &AWSConnector::connected, this, &AWSConnector::onConnected);
}

AWSConnector::~AWSConnector()
{
}

void AWSConnector::connect2AWS(const QString &endpoint, const QString &clientId, const QString &caFile, const QString &clientCertFile, const QString &clientPrivKeyFile)
{
    m_networkConnection = std::shared_ptr<MbedTLSConnection>(new MbedTLSConnection(
                                                                 endpoint.toStdString(),
                                                                 8883,
                                                                 caFile.toStdString(),
                                                                 clientCertFile.toStdString(),
                                                                 clientPrivKeyFile.toStdString(),
                                                                 std::chrono::milliseconds(30000),
                                                                 std::chrono::milliseconds(30000),
                                                                 std::chrono::milliseconds(30000),
                                                                 true
                                                                 ));
    m_client = MqttClient::Create(m_networkConnection, std::chrono::milliseconds(30000));
    m_client->SetDisconnectCallbackPtr(&onDisconnected, std::shared_ptr<DisconnectCallbackContextData>(this));
    m_client->SetAutoReconnectEnabled(true);
    m_clientId = clientId;

    // subscribe to pairing api topics
    subscribe({QString("create/device/%1").arg(m_clientId)});

    qCDebug(dcAWS()) << "Connecting to AWS with ID:" << m_clientId;
    m_connectingFuture = QtConcurrent::run([&]() {
        ResponseCode rc = m_client->Connect(std::chrono::milliseconds(30000), true, mqtt::Version::MQTT_3_1_1, std::chrono::seconds(60), Utf8String::Create(m_clientId.toStdString()), nullptr, nullptr, nullptr);
        if (rc == ResponseCode::MQTT_CONNACK_CONNECTION_ACCEPTED) {
            emit connected();
        } else {
            qCWarning(dcAWS) << "Error connecting to AWS. Response code:" << QString::fromStdString(ResponseHelper::ToString(rc));
            m_client.reset();
            m_networkConnection.reset();
        }
    });
}

DisconnectCallbackContextData::~DisconnectCallbackContextData() {}

void AWSConnector::disconnectAWS()
{
    if (isConnected()) {
        m_client->Disconnect(std::chrono::seconds(2));
    }
}

bool AWSConnector::isConnected() const
{
    return m_connectingFuture.isFinished() && m_networkConnection && m_client && m_client->IsConnected();
}

void AWSConnector::pairDevice(const QString &idToken, const QString &authToken, const QString &cognitoUserId)
{
    QVariantMap map;
    map.insert("idToken", idToken);
    map.insert("authToken", authToken);
    map.insert("cognitoUserId", cognitoUserId);
    map.insert("id", ++m_transactionId);
    map.insert("timestamp", QDateTime::currentMSecsSinceEpoch());
    publish(QString("%1/pair").arg(m_clientId), map);
    m_pairingRequests.insert(m_transactionId, cognitoUserId);
}

void AWSConnector::sendWebRtcHandshakeMessage(const QString &sessionId, const QVariantMap &map)
{
    publish(sessionId + "/reply", map);
}


quint16 AWSConnector::publish(const QString &topic, const QVariantMap &message)
{
    if (!isConnected()) {
        qCWarning(dcAWS()) << "Can't publish to AWS: Not connected.";
        return -1;
    }
    QString fullTopic = topic;
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(message);

    uint16_t packetId = 0;
    ResponseCode res = m_client->PublishAsync(Utf8String::Create(fullTopic.toStdString()), false, false, mqtt::QoS::QOS1, jsonDoc.toJson().toStdString(), &publishCallback, packetId);
    qCDebug(dcAWS()) << "publish call queued with status:" << QString::fromStdString(ResponseHelper::ToString(res)) << packetId << "for topic" << topic << jsonDoc.toJson();
    s_requestMap.insert(packetId, this);
    return packetId;
}

void AWSConnector::subscribe(const QStringList &topics)
{
    m_subscribedTopics.append(topics);

    if (!isConnected()) {
        qCDebug(dcAWS()) << "Can't subscribe to AWS: Not connected. Subscription will happen upon next connection.";
        return;
    }
    doSubscribe(topics);
}

void AWSConnector::onConnected()
{
    qCDebug(dcAWS()) << "AWS connected";
    registerDevice();
    retrievePairedDeviceInfo();
    if (!m_subscribedTopics.isEmpty()) {
        doSubscribe(m_subscribedTopics);
    }
}

void AWSConnector::retrievePairedDeviceInfo()
{
    QVariantMap params;
    params.insert("timestamp", QDateTime::currentMSecsSinceEpoch());
    params.insert("id", ++m_transactionId);
    publish(QString("%1/pair/list").arg(m_clientId), params);
}

void AWSConnector::registerDevice()
{
    QVariantMap params;
    params.insert("serverUUID", m_clientId);
    publish("create/device", params);
}

void AWSConnector::doSubscribe(const QStringList &topics)
{
    util::Vector<std::shared_ptr<mqtt::Subscription>> subscription_list;
    foreach (const QString &topic, topics) {
        qCDebug(dcAWS()) << "topic to subscribe is" << topic << "is valid topic:" << Subscription::IsValidTopicName(topic.toStdString());
        auto subscription = mqtt::Subscription::Create(Utf8String::Create(topic.toStdString()), mqtt::QoS::QOS1, &onSubscriptionReceivedCallback, std::shared_ptr<SubscriptionHandlerContextData>(this));
        subscription_list.push_back(subscription);
    }

    uint16_t packetId;
    ResponseCode res = m_client->SubscribeAsync(subscription_list, subscribeCallback, packetId);
    qCDebug(dcAWS()) << "subscribe call queued with status:" << QString::fromStdString(ResponseHelper::ToString(res)) << packetId;
    s_requestMap.insert(packetId, this);
}

void AWSConnector::publishCallback(uint16_t actionId, ResponseCode rc)
{
    AWSConnector* obj = s_requestMap.take(actionId);
    if (!obj) {
        qCWarning(dcAWS())<< "Received a response callback but don't have an object waiting for it.";
        return;
    }

    switch (rc) {
    case ResponseCode::SUCCESS:
        qCDebug(dcAWS()) << "Successfully published" << actionId;
        break;
    default:
        qCDebug(dcAWS())<< "Error publishing data to AWS:" << QString::fromStdString(ResponseHelper::ToString(rc));
    }
}

void AWSConnector::subscribeCallback(uint16_t actionId, ResponseCode rc)
{
    qCDebug(dcAWS()) << "subscribed to topic" << actionId << QString::fromStdString(ResponseHelper::ToString(rc));
}

ResponseCode AWSConnector::onSubscriptionReceivedCallback(util::String topic_name, util::String payload, std::shared_ptr<SubscriptionHandlerContextData> p_app_handler_data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(payload), &error);
    if (error.error != QJsonParseError::NoError) {
        qCDebug(dcAWS()) << "Failed to parse JSON from AWS subscription on topic" << QString::fromStdString(topic_name) << ":" << error.errorString() << "\n" << QString::fromStdString(payload);
        return ResponseCode::JSON_PARSING_ERROR;
    }

    AWSConnector *connector = dynamic_cast<AWSConnector*>(p_app_handler_data.get());
    QString topic = QString::fromStdString(topic_name);
    if (topic == QString("create/device/%1").arg(connector->m_clientId)) {
        int statusCode = jsonDoc.toVariant().toMap().value("result").toMap().value("code").toInt();
        if (statusCode != 200) {
            qCWarning(dcAWS()) << "Error registering device in the cloud. AWS connetion will not work.";
            return ResponseCode::SUCCESS;
        }
        qCDebug(dcAWS()) << "Device registered in cloud";
        connector->subscribe({QString("%1/pair/response").arg(connector->m_clientId),
                QString("%1/pair/list/response").arg(connector->m_clientId)});
    } else if (topic == QString("%1/pair/response").arg(connector->m_clientId)) {
        int statusCode = jsonDoc.toVariant().toMap().value("status").toInt();
        int id = jsonDoc.toVariant().toMap().value("id").toInt();
        QString cognitoUserId = connector->m_pairingRequests.take(id);
        if (!cognitoUserId.isEmpty()) {
            qCDebug(dcAWS()) << "Pairing response for id:" << cognitoUserId << statusCode;
            emit connector->devicePaired(cognitoUserId, statusCode);
            connector->subscribe({QString("eu-west-1:%1/listeningPeer/#").arg(cognitoUserId)});
        } else {
            qCWarning(dcAWS()) << "Received a pairing response for a transaction we didn't start";
        }
    } else if (topic == QString("%1/pair/list/response").arg(connector->m_clientId)) {
        qCDebug(dcAWS) << "have device pairings:" << jsonDoc.toVariant().toMap().value("pairings").toList();
        QStringList topics;
        foreach (const QVariant &pairing, jsonDoc.toVariant().toMap().value("pairings").toList()) {
            topics << QString("eu-west-1:%1/listeningPeer/#").arg(pairing.toMap().value("cognitoIdIdentityId").toString());
        }
        connector->subscribe(topics);
    } else if (topic.contains("listeningPeer") && !topic.contains("reply")) {
        static QStringList dupes;
        QString id = jsonDoc.toVariant().toMap().value("id").toString();
        QString type = jsonDoc.toVariant().toMap().value("type").toString();
        if (dupes.contains(id+type)) {
            qCDebug(dcAWS()) << "Dropping duplicate packet";
            return ResponseCode::SUCCESS;
        }
        dupes.append(id+type);

        qCDebug(dcAWS) << "received webrtc handshake message" << topic << jsonDoc.toJson();
        connector->webRtcHandshakeMessageReceived(topic, jsonDoc.toVariant().toMap());
    } else if (topic.contains("listeningPeer") && topic.contains("reply")) {
        // silently drop our own things (should not be subscribed to that in the first place)
    } else {
        qCWarning(dcAWS) << "Unhandled subscription received!" << topic << QString::fromStdString(payload);
    }
    return ResponseCode::SUCCESS;
}

ResponseCode AWSConnector::onDisconnected(util::String mqtt_client_id, std::shared_ptr<DisconnectCallbackContextData> p_app_handler_data)
{
    Q_UNUSED(p_app_handler_data)
    qCDebug(dcAWS()) << "disconnected" << QString::fromStdString(mqtt_client_id);
    return ResponseCode::SUCCESS;
}
