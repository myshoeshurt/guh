/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OAUTH2_H
#define OAUTH2_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "libguh.h"

// OAuth 2.0 - Resource Owner Password Credentials Grant: http://tools.ietf.org/html/rfc6749#section-4.3

class LIBGUH_EXPORT OAuth2 : public QObject
{
    Q_OBJECT
public:
    explicit OAuth2(QString clientId, QString clientSecret, QObject *parent = nullptr);

    QUrl url() const;
    void setUrl(const QUrl &url);

    QUrlQuery query() const;
    void setQuery(const QUrlQuery &query);

    QString username() const;
    void setUsername(const QString &username);

    QString password() const;
    void setPassword(const QString &password);

    QString clientId() const;
    void setClientId(const QString &clientId);

    QString clientSecret() const;
    void setClientSecret(const QString clientSecret);

    QString scope() const;
    void setScope(const QString &scope);

    QString token() const;

    bool authenticated() const;

    void startAuthentication();

private:
    QNetworkAccessManager *m_networkManager;
    QTimer *m_timer;
    QList<QNetworkReply *> m_tokenRequests;
    QList<QNetworkReply *> m_refreshTokenRequests;

    QUrl m_url;
    QUrlQuery m_query;
    QString m_username;
    QString m_password;
    QString m_clientId;
    QString m_clientSecret;
    QString m_scope;

    QString m_token;
    QString m_refreshToken;

    bool m_authenticated;

    void setAuthenticated(const bool &authenticated);
    void setToken(const QString &token);

private slots:
    void replyFinished(QNetworkReply *reply);
    void refreshTimeout();

signals:
    void authenticationChanged();
    void tokenChanged();

};

#endif // OAUTH2_H
