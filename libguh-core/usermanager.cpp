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

#include "usermanager.h"
#include "guhsettings.h"
#include "loggingcategories.h"
#include "guhcore.h"
#include "pushbuttondbusservice.h"

#include <QUuid>
#include <QCryptographicHash>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QRegExpValidator>
#include <QDateTime>
#include <QDebug>

namespace guhserver {

UserManager::UserManager(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), "users");
    m_db.setDatabaseName(GuhSettings::settingsPath() + "/user-db.sqlite");

    if (!m_db.open()) {
        qCWarning(dcUserManager) << "Error opening users database:" << m_db.lastError().driverText();
        return;
    }
    initDB();

    m_pushButtonDBusService = new PushButtonDBusService(this);
}

QStringList UserManager::users() const
{
    QString userQuery("SELECT username FROM users;");
    QSqlQuery result = m_db.exec(userQuery);
    QStringList ret;
    while (result.next()) {
        ret << result.value("username").toString();
    }
    return ret;
}

UserManager::UserError UserManager::createUser(const QString &username, const QString &password)
{
    if (!validateUsername(username)) {
        qCWarning(dcUserManager) << "Error creating user. Invalid username";
        return UserErrorInvalidUserId;
    }

    QRegExp passwordValidator = QRegExp("^(?=.*[A-Za-z])(?=.*\[0-9])(?=.*[$@$!%*#?&])[A-Za-z0-9$@$!%*#?&]{8,}$");
    if (!passwordValidator.exactMatch(password)) {
        qCWarning(dcUserManager) << "Password failed character validation. Must contain a letter, a number and a special charactar. Minimum length: 8";
        return UserErrorBadPassword;
    }

    QString checkForDuplicateUserQuery = QString("SELECT * FROM users WHERE lower(username) = \"%1\";").arg(username.toLower());
    QSqlQuery result = m_db.exec(checkForDuplicateUserQuery);
    if (result.first()) {
        qCWarning(dcUserManager) << "Username already in use";
        return UserErrorDuplicateUserId;
    }

    QByteArray salt = QUuid::createUuid().toString().remove(QRegExp("[{}]")).toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(QString(password + salt).toUtf8(), QCryptographicHash::Sha512).toBase64();
    QString queryString = QString("INSERT INTO users(username, password, salt) values(\"%1\", \"%2\", \"%3\");")
            .arg(username)
            .arg(QString::fromUtf8(hashedPassword))
            .arg(QString::fromUtf8(salt));
    m_db.exec(queryString);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Error creating user:" << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return UserErrorBackendError;
    }
    return UserErrorNoError;
}

UserManager::UserError UserManager::removeUser(const QString &username)
{
    QString dropUserQuery = QString("DELETE FROM users WHERE lower(username) =\"%1\";").arg(username.toLower());
    QSqlQuery result = m_db.exec(dropUserQuery);
    if (result.numRowsAffected() == 0) {
        return UserErrorInvalidUserId;
    }

    QString dropTokensQuery = QString("DELETE FROM tokens WHERE lower(username) = \"%1\";").arg(username.toLower());
    m_db.exec(dropTokensQuery);

    return UserErrorNoError;
}

QByteArray UserManager::authenticate(const QString &username, const QString &password, const QString &deviceName)
{
    if (!validateUsername(username)) {
        qCWarning(dcUserManager) << "Username did not pass validation:" << username;
        return QByteArray();
    }

    QString passwordQuery = QString("SELECT password, salt FROM users WHERE lower(username) = \"%1\";").arg(username.toLower());
    QSqlQuery result = m_db.exec(passwordQuery);
    if (!result.first()) {
        qCWarning(dcUserManager) << "No such username" << username;
        return QByteArray();
    }
    QByteArray salt = result.value("salt").toByteArray();
    QByteArray hashedPassword = result.value("password").toByteArray();

    if (hashedPassword != QCryptographicHash::hash(QString(password + salt).toUtf8(), QCryptographicHash::Sha512).toBase64()) {
        qCWarning(dcUserManager) << "Authentication error for user:" << username;
        return QByteArray();
    }

    QByteArray token = QCryptographicHash::hash(QUuid::createUuid().toByteArray(), QCryptographicHash::Sha256).toBase64();
    QString storeTokenQuery = QString("INSERT INTO tokens(id, username, token, creationdate, devicename) VALUES(\"%1\", \"%2\", \"%3\", \"%4\", \"%5\");")
            .arg(QUuid::createUuid().toString())
            .arg(username)
            .arg(QString::fromUtf8(token))
            .arg(GuhCore::instance()->timeManager()->currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(deviceName);

    m_db.exec(storeTokenQuery);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Error storing token in DB:" << m_db.lastError().databaseText() << m_db.lastError().driverText();
        return QByteArray();
    }
    return token;
}

int UserManager::requestPushButtonAuth(const QString &deviceName)
{
    if (m_pushButtonTransaction.first != -1) {
        qCWarning(dcUserManager()) << "PushButton authentication already in progress for device" << m_pushButtonTransaction.second << ". Cancelling...";
        cancelPushButtonAuth(m_pushButtonTransaction.first);
    }

    qCDebug(dcUserManager()) << "Starting PushButton authentication for device" << deviceName;
    int transactionId = ++m_pushButtonTransactionIdCounter;
    m_pushButtonTransaction = qMakePair<int, QString>(transactionId, deviceName);
    return transactionId;
}

void UserManager::cancelPushButtonAuth(int transactionId)
{
    if (m_pushButtonTransaction.first == -1) {
        qCWarning(dcUserManager()) << "No PushButton transaction in progress. Nothing to cancel.";
        return;
    }
    if (m_pushButtonTransaction.first != transactionId) {
        qWarning(dcUserManager()) << "PushButton transaction" << transactionId << "not in progress. Cannot cancel.";
        return;
    }
    emit pushButtonAuthFinished(m_pushButtonTransaction.first, false, QByteArray());
    m_pushButtonTransaction.first = -1;

}

QString UserManager::userForToken(const QByteArray &token) const
{
    if (!validateToken(token)) {
        qCWarning(dcUserManager) << "Token failed character validation:" << token;
        return QString();
    }
    QString getUserQuery = QString("SELECT * FROM tokens WHERE token = \"%1\";")
            .arg(QString::fromUtf8(token));
    QSqlQuery result = m_db.exec(getUserQuery);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Error fetching username for token:" << m_db.lastError().databaseText() << m_db.lastError().driverText() << getUserQuery;
        return QString();
    }
    if (!result.first()) {
        qCWarning(dcUserManager) << "No such token in DB:" << token;
        return QString();
    }

    return result.value("username").toString();
}

QList<TokenInfo> UserManager::tokens(const QString &username) const
{
    QList<TokenInfo> ret;

    if (!validateUsername(username)) {
        qCWarning(dcUserManager) << "Username did not pass validation:" << username;
        return ret;
    }
    QString getTokensQuery = QString("SELECT id, username, creationdate, deviceName FROM tokens WHERE lower(username) = \"%1\";")
            .arg(username.toLower());
    QSqlQuery result = m_db.exec(getTokensQuery);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Query for tokens failed:" << m_db.lastError().databaseText() << m_db.lastError().driverText() << getTokensQuery;
        return ret;
    }

    while (result.next()) {
        ret << TokenInfo(result.value("id").toUuid(), result.value("username").toString(), result.value("creationdate").toDateTime(), result.value("devicename").toString());
    }
    return ret;
}

UserManager::UserError UserManager::removeToken(const QUuid &tokenId)
{
    QString removeTokenQuery = QString("DELETE FROM tokens WHERE id = \"%1\";")
            .arg(tokenId.toString());
    QSqlQuery result = m_db.exec(removeTokenQuery);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Removing token failed:" << m_db.lastError().databaseText() << m_db.lastError().driverText() << removeTokenQuery;
        return UserErrorBackendError;
    }
    if (result.numRowsAffected() != 1) {
        qCWarning(dcUserManager) << "Token not found in DB";
        return UserErrorTokenNotFound;
    }

    qCDebug(dcUserManager) << "Token" << tokenId << "removed from DB";
    return UserErrorNoError;
}

bool UserManager::verifyToken(const QByteArray &token)
{
    if (!validateToken(token)) {
        qCWarning(dcUserManager) << "Token failed character validation" << token;
        return false;
    }
    QString getTokenQuery = QString("SELECT * FROM tokens WHERE token = \"%1\";")
            .arg(QString::fromUtf8(token));
    QSqlQuery result = m_db.exec(getTokenQuery);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Query for token failed:" << m_db.lastError().databaseText() << m_db.lastError().driverText() << getTokenQuery;
        return false;
    }
    if (!result.first()) {
        qCDebug(dcUserManager) << "Authorisation failed for token" << token;
        return false;
    }
    //qCDebug(dcUserManager) << "Token authorized for user" << result.value("username").toString();
    return true;
}

void UserManager::initDB()
{
    if (!m_db.tables().contains("users")) {
        m_db.exec("CREATE TABLE users (username VARCHAR(40) UNIQUE, password VARCHAR(100), salt VARCHAR(100));");
    }
    if (!m_db.tables().contains("tokens")) {
        m_db.exec("CREATE TABLE tokens (id VARCHAR(40) UNIQUE, username VARCHAR(40), token VARCHAR(100) UNIQUE, creationdate DATETIME, devicename VARCHAR(40));");
    }
}

bool UserManager::validateUsername(const QString &username) const
{
    QRegExp validator("(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+.[a-zA-Z0-9-.]+$)");
    return validator.exactMatch(username);
}

bool UserManager::validateToken(const QByteArray &token) const
{
    QRegExp validator(QRegExp("(^[a-zA-Z0-9_.+-/=]+$)"));
    return validator.exactMatch(token);
}

void UserManager::pushButtonPressed()
{
    if (m_pushButtonTransaction.first == -1) {
        qCDebug(dcUserManager()) << "PushButton pressed but don't have a transaction waiting for it.";
        return;
    }

    QByteArray token = QCryptographicHash::hash(QUuid::createUuid().toByteArray(), QCryptographicHash::Sha256).toBase64();
    QString storeTokenQuery = QString("INSERT INTO tokens(id, username, token, creationdate, devicename) VALUES(\"%1\", \"%2\", \"%3\", \"%4\", \"%5\");")
            .arg(QUuid::createUuid().toString())
            .arg("")
            .arg(QString::fromUtf8(token))
            .arg(GuhCore::instance()->timeManager()->currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(m_pushButtonTransaction.second);

    m_db.exec(storeTokenQuery);
    if (m_db.lastError().type() != QSqlError::NoError) {
        qCWarning(dcUserManager) << "Error storing token in DB:" << m_db.lastError().databaseText() << m_db.lastError().driverText();
        emit pushButtonAuthFinished(m_pushButtonTransaction.first, false, QByteArray());
    }
    qCDebug(dcUserManager()) << "PushButton Auth succeeded";
    emit pushButtonAuthFinished(m_pushButtonTransaction.first, true, token);

    m_pushButtonTransaction.first = -1;
}

}
