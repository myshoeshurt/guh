/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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

/*!
    \class guhserver::ConfigurationHandler
    \brief This subclass of \l{JsonHandler} processes the JSON requests for the \tt Configuration namespace.

    \ingroup json
    \inmodule core

    This \l{JsonHandler} will be created in the \l{JsonRPCServer} and used to handle JSON-RPC requests
    for the \tt {Configuration} namespace of the API.

    \sa JsonHandler, JsonRPCServer
*/

/*! \fn void guhserver::ConfigurationHandler::BasicConfigurationChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the configurations of the server have been changed.
    The \a params contains the map for the notification.
*/

/*! \fn void guhserver::ConfigurationHandler::TcpServerConfigurationChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the configurations of the \l{TcpServer} have been changed.
    The \a params contains the map for the notification.
*/

/*! \fn void guhserver::ConfigurationHandler::WebServerConfigurationChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the configurations of the \l{WebServer} have been changed.
    The \a params contains the map for the notification.
*/

/*! \fn void guhserver::ConfigurationHandler::WebSocketServerConfigurationChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the configurations of the \l{WebSocketServer} have been changed.
    The \a params contains the map for the notification.
*/

/*! \fn void guhserver::ConfigurationHandler::LanguageChanged(const QVariantMap &params);
    This signal is emitted to the API notifications when the language of the system has changed.
    The \a params contains the map for the notification.
*/



#include "configurationhandler.h"
#include "guhcore.h"

namespace guhserver {

/*! Constructs a new \l ConfigurationHandler with the given \a parent. */
ConfigurationHandler::ConfigurationHandler(QObject *parent):
    JsonHandler(parent)
{
    // Methods
    QVariantMap params; QVariantMap returns;
    setDescription("GetTimeZones", "Get the list of available timezones.");
    setParams("GetTimeZones", params);
    returns.insert("timeZones", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::String));
    setReturns("GetTimeZones", returns);

    params.clear(); returns.clear();
    setDescription("GetAvailableLanguages", "Returns a list of locale codes available for the server. i.e. en_US, de_AT");
    setParams("GetAvailableLanguages", params);
    returns.insert("languages", QVariantList() << JsonTypes::basicTypeToString(JsonTypes::String));
    setReturns("GetAvailableLanguages", returns);

    params.clear(); returns.clear();
    setDescription("GetConfigurations", "Get all configuration parameters of the server.");
    setParams("GetConfigurations", params);
    QVariantMap basicConfiguration;
    basicConfiguration.insert("serverName", JsonTypes::basicTypeToString(JsonTypes::String));
    basicConfiguration.insert("serverUuid", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    basicConfiguration.insert("serverTime", JsonTypes::basicTypeToString(JsonTypes::Uint));
    basicConfiguration.insert("timeZone", JsonTypes::basicTypeToString(JsonTypes::String));
    basicConfiguration.insert("language", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("basicConfiguration", basicConfiguration);
    QVariantList tcpServerConfigurations;
    tcpServerConfigurations.append(JsonTypes::serverConfigurationRef());
    returns.insert("tcpServerConfigurations", tcpServerConfigurations);
    QVariantList webServerConfigurations;
    webServerConfigurations.append(JsonTypes::webServerConfigurationRef());
    returns.insert("webServerConfigurations", webServerConfigurations);
    QVariantList webSocketServerConfigurations;
    webSocketServerConfigurations.append(JsonTypes::serverConfigurationRef());
    returns.insert("webSocketServerConfigurations", webSocketServerConfigurations);
    setReturns("GetConfigurations", returns);

    params.clear(); returns.clear();
    setDescription("SetServerName", "Set the name of the server. Default is guhIO.");
    params.insert("serverName",  JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("SetServerName", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetServerName", returns);

    params.clear(); returns.clear();
    setDescription("SetTimeZone", "Set the time zone of the server. See also: \"GetTimeZones\"");
    params.insert("timeZone",  JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("SetTimeZone", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetTimeZone", returns);

    params.clear(); returns.clear();
    setDescription("SetLanguage", "Sets the server language to the given language. See also: \"GetAvailableLanguages\"");
    params.insert("language",  JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("SetLanguage", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetLanguage", returns);

    params.clear(); returns.clear();
    setDescription("SetTcpServerConfiguration", "Configure a TCP interface of the server. If the ID is an existing one, the existing config will be modified, otherwise a new one will be added. Note: if you are changing the configuration for the interface you are currently connected to, the connection will be dropped.");
    params.insert("configuration", JsonTypes::serverConfigurationRef());
    setParams("SetTcpServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetTcpServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("DeleteTcpServerConfiguration", "Delete a TCP interface of the server. Note: if you are deleting the configuration for the interface you are currently connected to, the connection will be dropped.");
    params.insert("id", JsonTypes::basicTypeToString(QVariant::String));
    setParams("DeleteTcpServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("DeleteTcpServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("SetWebSocketServerConfiguration", "Configure a WebSocket Server interface of the server. If the ID is an existing one, the existing config will be modified, otherwise a new one will be added. Note: if you are changing the configuration for the interface you are currently connected to, the connection will be dropped.");
    params.insert("configuration", JsonTypes::serverConfigurationRef());
    setParams("SetWebSocketServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetWebSocketServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("DeleteWebSocketServerConfiguration", "Delete a WebSocket Server interface of the server. Note: if you are deleting the configuration for the interface you are currently connected to, the connection will be dropped.");
    params.insert("id", JsonTypes::basicTypeToString(QVariant::String));
    setParams("DeleteWebSocketServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("DeleteWebSocketServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("SetWebServerConfiguration", "Configure a WebServer interface of the server. If the ID is an existing one, the existing config will be modified, otherwise a new one will be added.");
    params.insert("configuration", JsonTypes::webServerConfigurationRef());
    setParams("SetWebServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("SetWebServerConfiguration", returns);

    params.clear(); returns.clear();
    setDescription("DeleteWebServerConfiguration", "Delete a WebServer interface of the server.");
    params.insert("id", JsonTypes::basicTypeToString(QVariant::String));
    setParams("DeleteWebServerConfiguration", params);
    returns.insert("configurationError", JsonTypes::configurationErrorRef());
    setReturns("DeleteWebServerConfiguration", returns);

    // Notifications
    params.clear(); returns.clear();
    setDescription("BasicConfigurationChanged", "Emitted whenever the basic configuration of this server changes.");
    params.insert("serverName", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("serverUuid", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    params.insert("serverTime", JsonTypes::basicTypeToString(JsonTypes::Uint));
    params.insert("timeZone", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("BasicConfigurationChanged", params);

    params.clear(); returns.clear();
    setDescription("TcpServerConfigurationChanged", "Emitted whenever the TCP server configuration changes.");
    params.insert("configuration", JsonTypes::serverConfigurationRef());
    setParams("TcpServerConfigurationChanged", params);

    params.clear(); returns.clear();
    setDescription("TcpServerConfigurationRemoved", "Emitted whenever the TCP server configuration has been removed.");
    params.insert("configuration", JsonTypes::serverConfigurationRef());
    setParams("TcpServerConfigurationRemoved", params);

    params.clear(); returns.clear();
    setDescription("WebServerConfigurationChanged", "Emitted whenever the web server configuration changes.");
    params.insert("configuration", JsonTypes::serverConfigurationRef());
    setParams("WebServerConfigurationChanged", params);

    params.clear(); returns.clear();
    setDescription("WebSocketServerConfigurationChanged", "Emitted whenever the web socket server configuration changes.");
    params.insert("configuration", JsonTypes::serverConfigurationRef());
    setParams("WebSocketServerConfigurationChanged", params);

    params.clear(); returns.clear();
    setDescription("LanguageChanged", "Emitted whenever the language of the server changed. The Plugins, Vendors and DeviceClasses have to be reloaded to get the translated data.");
    params.insert("language", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("LanguageChanged", params);

    connect(GuhCore::instance()->configuration(), &GuhConfiguration::serverNameChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(GuhCore::instance()->configuration(), &GuhConfiguration::timeZoneChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(GuhCore::instance()->configuration(), &GuhConfiguration::localeChanged, this, &ConfigurationHandler::onBasicConfigurationChanged);
    connect(GuhCore::instance()->configuration(), &GuhConfiguration::tcpServerConfigurationChanged, this, &ConfigurationHandler::onTcpServerConfigurationChanged);
    connect(GuhCore::instance()->configuration(), &GuhConfiguration::tcpServerConfigurationRemoved, this, &ConfigurationHandler::onTcpServerConfigurationRemoved);
    connect(GuhCore::instance()->configuration(), &GuhConfiguration::webServerConfigurationChanged, this, &ConfigurationHandler::onWebServerConfigurationChanged);
    connect(GuhCore::instance()->configuration(), &GuhConfiguration::webServerConfigurationRemoved, this, &ConfigurationHandler::onWebServerConfigurationRemoved);
    connect(GuhCore::instance()->configuration(), &GuhConfiguration::webSocketServerConfigurationChanged, this, &ConfigurationHandler::onWebSocketServerConfigurationChanged);
    connect(GuhCore::instance()->configuration(), &GuhConfiguration::webSocketServerConfigurationRemoved, this, &ConfigurationHandler::onWebSocketServerConfigurationRemoved);
    connect(GuhCore::instance()->deviceManager(), &DeviceManager::languageUpdated, this, &ConfigurationHandler::onLanguageChanged);
}

/*! Returns the name of the \l{ConfigurationHandler}. In this case \b Configuration.*/
QString ConfigurationHandler::name() const
{
    return "Configuration";
}

JsonReply *ConfigurationHandler::GetConfigurations(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantMap returns;
    returns.insert("basicConfiguration", JsonTypes::packBasicConfiguration());
    QVariantList tcpServerConfigs;
    foreach (const ServerConfiguration &config, GuhCore::instance()->configuration()->tcpServerConfigurations()) {
        tcpServerConfigs.append(JsonTypes::packServerConfiguration(config));
    }
    returns.insert("tcpServerConfigurations", tcpServerConfigs);

    QVariantList webServerConfigs;
    foreach (const WebServerConfiguration &config, GuhCore::instance()->configuration()->webServerConfigurations()) {
        webServerConfigs.append(JsonTypes::packWebServerConfiguration(config));

    }
    returns.insert("webServerConfigurations", webServerConfigs);

    QVariantList webSocketServerConfigs;
    foreach (const ServerConfiguration &config, GuhCore::instance()->configuration()->webSocketServerConfigurations()) {
        webSocketServerConfigs.append(JsonTypes::packServerConfiguration(config));
    }
    returns.insert("webSocketServerConfigurations", webSocketServerConfigs);

    return createReply(returns);
}

JsonReply *ConfigurationHandler::GetTimeZones(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantList timeZones;
    foreach (const QByteArray &timeZoneId, GuhCore::instance()->timeManager()->availableTimeZones()) {
        timeZones.append(QString::fromUtf8(timeZoneId));
    }

    QVariantMap returns;
    returns.insert("timeZones", timeZones);
    return createReply(returns);
}

JsonReply *ConfigurationHandler::GetAvailableLanguages(const QVariantMap &params) const
{
    Q_UNUSED(params)
    QVariantList languages;
    foreach (const QString &language, GuhCore::getAvailableLanguages()) {
        languages.append(language);
    }
    QVariantMap returns;
    returns.insert("languages", languages);
    return createReply(returns);
}

JsonReply *ConfigurationHandler::SetServerName(const QVariantMap &params) const
{
    QString serverName = params.value("serverName").toString();
    GuhCore::instance()->configuration()->setServerName(serverName);
    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetTimeZone(const QVariantMap &params) const
{
    qCDebug(dcJsonRpc()) << "Setting time zone to" << params.value("timeZone").toString();

    QByteArray timeZone = params.value("timeZone").toString().toUtf8();
    if (!GuhCore::instance()->timeManager()->setTimeZone(timeZone))
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidTimeZone));

    GuhCore::instance()->configuration()->setTimeZone(timeZone);
    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetLanguage(const QVariantMap &params) const
{
    qCDebug(dcJsonRpc()) << "Setting language to" << params.value("language").toString();
    QLocale locale(params.value("language").toString());

    GuhCore::instance()->configuration()->setLocale(locale);

    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetTcpServerConfiguration(const QVariantMap &params) const
{
    ServerConfiguration config = JsonTypes::unpackServerConfiguration(params.value("configuration").toMap());
    if (config.id.isEmpty()) {
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidId));
    }
    if (config.address.isNull())
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidHostAddress));

    if (config.port <= 0 || config.port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidPort));
    }

    qCDebug(dcJsonRpc()) << QString("Configure TCP server %1:%2").arg(config.address.toString()).arg(config.port);

    GuhCore::instance()->configuration()->setTcpServerConfiguration(config);

//    GuhCore::instance()->tcpServer()->reconfigureServer(config);

    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::DeleteTcpServerConfiguration(const QVariantMap &params) const
{
    QString id = params.value("id").toString();
    if (id.isEmpty() || !GuhCore::instance()->configuration()->tcpServerConfigurations().contains(id)) {
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidId));
    }
    GuhCore::instance()->configuration()->removeTcpServerConfiguration(id);
    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetWebServerConfiguration(const QVariantMap &params) const
{
    WebServerConfiguration config = JsonTypes::unpackWebServerConfiguration(params.value("configuration").toMap());

    if (config.id.isEmpty()) {
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidId));
    }
    if (config.address.isNull())
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidHostAddress));

    if (config.port <= 0 || config.port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidPort));
    }

    qCDebug(dcJsonRpc()) << QString("Configure web server %1:%2").arg(config.address.toString()).arg(config.port);

    GuhCore::instance()->configuration()->setWebServerConfiguration(config);
//    GuhCore::instance()->webServer()->reconfigureServer(config);

    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::DeleteWebServerConfiguration(const QVariantMap &params) const
{
    QString id = params.value("id").toString();
    if (id.isEmpty() || !GuhCore::instance()->configuration()->webServerConfigurations().contains(id)) {
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidId));
    }
    GuhCore::instance()->configuration()->removeWebServerConfiguration(id);
    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::SetWebSocketServerConfiguration(const QVariantMap &params) const
{
    ServerConfiguration config = JsonTypes::unpackServerConfiguration(params.value("configuration").toMap());
    if (config.id.isEmpty()) {
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidId));
    }
    if (config.address.isNull())
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidHostAddress));

    if (config.port <= 0 || config.port > 65535) {
        qCWarning(dcJsonRpc()) << "Port out of range";
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidPort));
    }

    qCDebug(dcJsonRpc()) << QString("Configure web socket server %1:%2").arg(config.address.toString()).arg(config.port);

    GuhCore::instance()->configuration()->setWebSocketServerConfiguration(config);

    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

JsonReply *ConfigurationHandler::DeleteWebSocketServerConfiguration(const QVariantMap &params) const
{
    QString id = params.value("id").toString();
    if (id.isEmpty() || !GuhCore::instance()->configuration()->webSocketServerConfigurations().contains(id)) {
        return createReply(statusToReply(GuhConfiguration::ConfigurationErrorInvalidId));
    }
    GuhCore::instance()->configuration()->removeWebSocketServerConfiguration(id);
    return createReply(statusToReply(GuhConfiguration::ConfigurationErrorNoError));
}

void ConfigurationHandler::onBasicConfigurationChanged()
{
    QVariantMap params;
    qCDebug(dcJsonRpc()) << "Notification: Basic configuration changed";
    params.insert("basicConfiguration", JsonTypes::packBasicConfiguration());
    emit BasicConfigurationChanged(params);
}

void ConfigurationHandler::onTcpServerConfigurationChanged(const QString &id)
{
    QVariantMap params;
    qCDebug(dcJsonRpc()) << "Notification: TCP server configuration changed";
    params.insert("configuration", JsonTypes::packServerConfiguration(GuhCore::instance()->configuration()->tcpServerConfigurations().value(id)));
    emit TcpServerConfigurationChanged(params);
}

void ConfigurationHandler::onTcpServerConfigurationRemoved(const QString &id)
{
    QVariantMap params;
    qCDebug(dcJsonRpc()) << "Notification: TCP server configuration removed";
    params.insert("configuration", JsonTypes::packServerConfiguration(GuhCore::instance()->configuration()->tcpServerConfigurations().value(id)));
    emit TcpServerConfigurationChanged(params);
}

void ConfigurationHandler::onWebServerConfigurationChanged(const QString &id)
{
    QVariantMap params;
    qCDebug(dcJsonRpc()) << "Notification: Web server configuration changed";
    params.insert("configuration", JsonTypes::packServerConfiguration(GuhCore::instance()->configuration()->webServerConfigurations().value(id)));
    emit WebServerConfigurationChanged(params);
}

void ConfigurationHandler::onWebServerConfigurationRemoved(const QString &id)
{

}

void ConfigurationHandler::onWebSocketServerConfigurationChanged(const QString &id)
{
    QVariantMap params;
    qCDebug(dcJsonRpc()) << "Notification: Web socket server configuration changed";
    params.insert("configuration", JsonTypes::packServerConfiguration(GuhCore::instance()->configuration()->webSocketServerConfigurations().value(id)));
    emit WebSocketServerConfigurationChanged(params);
}

void ConfigurationHandler::onWebSocketServerConfigurationRemoved(const QString &id)
{

}

void ConfigurationHandler::onLanguageChanged()
{
    QVariantMap params;
    qCDebug(dcJsonRpc()) << "Notification: Language configuration changed";
    params.insert("language", GuhCore::instance()->configuration()->locale().name());
    emit LanguageChanged(params);
}

}
