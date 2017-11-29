/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef HARDWAREMANAGER_H
#define HARDWAREMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>

#include "hardwareresource.h"

class PluginTimer;
class BluetoothScanner;
class Radio433;
class NetworkAccessManager;
class UpnpDiscovery;
class UpnpDeviceDescriptor;
class QtAvahiServiceBrowser;

class HardwareManager : public QObject
{
    Q_OBJECT

    friend class DeviceManager;

public:
    explicit HardwareManager(QObject *parent = nullptr);

    Radio433 *radio433();
    PluginTimer *pluginTimer();
    NetworkAccessManager *networkManager();
    UpnpDiscovery *upnpDiscovery();
    QtAvahiServiceBrowser *avahiBrowser();
    BluetoothScanner *bluetoothScanner();

    bool isAvailable(const HardwareResource::Type &hardwareResourceType) const;
    bool isEnabled(const HardwareResource::Type &hardwareResourceType) const;

private:
    QList<HardwareResource *> m_hardwareResources;

    QNetworkAccessManager *m_networkAccessManager;

    // Hardware Resources
    Radio433 *m_radio433 = nullptr;
    PluginTimer *m_pluginTimer = nullptr;
    NetworkAccessManager *m_networkManager = nullptr;
    UpnpDiscovery *m_upnpDiscovery = nullptr;
    QtAvahiServiceBrowser *m_avahiBrowser = nullptr;
    BluetoothScanner *m_bluetoothScanner = nullptr;

    bool enableHardwareReource(const HardwareResource::Type &hardwareResourceType);
    bool disableHardwareReource(const HardwareResource::Type &hardwareResourceType);

signals:
    void hardwareResourceAvailableChanged(const HardwareResource::Type &hardwareResourceType, const bool &available);
    void hardwareResourceEnabledChanged(const HardwareResource::Type &hardwareResourceType, const bool &enabled);


};


#endif // HARDWAREMANAGER_H
