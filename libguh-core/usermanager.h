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

#ifndef USERMANAGER_H
#define USERMANAGER_H

#include "tokeninfo.h"

#include <QObject>
#include <QSqlDatabase>

namespace guhserver {

class PushButtonDBusService;

class UserManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(UserError)
public:
    enum UserError {
        UserErrorNoError,
        UserErrorBackendError,
        UserErrorInvalidUserId,
        UserErrorDuplicateUserId,
        UserErrorBadPassword,
        UserErrorTokenNotFound,
        UserErrorPermissionDenied
    };

    explicit UserManager(QObject *parent = 0);

    QStringList users() const;

    UserError createUser(const QString &username, const QString &password);
    UserError removeUser(const QString &username);

    QByteArray authenticate(const QString &username, const QString &password, const QString &deviceName);
    int requestPushButtonAuth(const QString &deviceName);
    void cancelPushButtonAuth(int transactionId);
    QString userForToken(const QByteArray &token) const;
    QList<TokenInfo> tokens(const QString &username) const;
    guhserver::UserManager::UserError removeToken(const QUuid &tokenId);

    bool verifyToken(const QByteArray &token);

    void pushButtonPressed();

signals:
    void pushButtonAuthFinished(int transactionId, bool success, const QByteArray &token);

private:
    void initDB();
    bool validateUsername(const QString &username) const;
    bool validateToken(const QByteArray &token) const;

private:
    QSqlDatabase m_db;
    PushButtonDBusService *m_pushButtonDBusService = nullptr;
    int m_pushButtonTransactionIdCounter = 0;
    QPair<int, QString> m_pushButtonTransaction = QPair<int, QString>(-1, QString());

};

}

#endif // USERMANAGER_H
