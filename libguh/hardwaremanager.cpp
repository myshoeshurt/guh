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

#include "hardwaremanager.h"

#include "plugintimer.h"
#include "loggingcategories.h"
#include "bluetooth/bluetoothscanner.h"
#include "hardware/radio433/radio433.h"
#include "network/networkaccessmanager.h"
#include "network/upnp/upnpdiscovery.h"
#include "network/upnp/upnpdevicedescriptor.h"
#include "network/avahi/qtavahiservicebrowser.h"

HardwareManager::HardwareManager(QObject *parent) : QObject(parent)
{
    // Init hardware resources
    m_pluginTimer = new PluginTimer(10000, this);
    m_hardwareResources.append(m_pluginTimer);

    m_radio433 = new Radio433(this);
    m_hardwareResources.append(m_radio433);
    m_radio433->enable();

    // Create network access manager for all resources, centralized
    // Note: configuration and proxy settings could be implemented here
    m_networkAccessManager = new QNetworkAccessManager(this);

    // Network manager
    m_networkManager = new NetworkAccessManager(m_networkAccessManager, this);
    m_hardwareResources.append(m_networkManager);
    if (m_networkManager->available())
        m_networkManager->enable();

    // UPnP discovery
    m_upnpDiscovery = new UpnpDiscovery(m_networkAccessManager, this);
    m_hardwareResources.append(m_upnpDiscovery);
    m_upnpDiscovery->enable();

    // Avahi Browser
    m_avahiBrowser = new QtAvahiServiceBrowser(this);
    m_hardwareResources.append(m_avahiBrowser);
    m_avahiBrowser->enable();

//    // Bluetooth LE
//    m_bluetoothScanner = new BluetoothScanner(this);
//    if (!m_bluetoothScanner->isAvailable()) {
//        delete m_bluetoothScanner;
//        m_bluetoothScanner = nullptr;
//    }

    qCDebug(dcHardware()) << "Hardware manager initialized successfully";
}

Radio433 *HardwareManager::radio433()
{
    return m_radio433;
}

PluginTimer *HardwareManager::pluginTimer()
{
    return m_pluginTimer;
}

NetworkAccessManager *HardwareManager::networkManager()
{
    return m_networkManager;
}

UpnpDiscovery *HardwareManager::upnpDiscovery()
{
    return m_upnpDiscovery;
}

QtAvahiServiceBrowser *HardwareManager::avahiBrowser()
{
    return m_avahiBrowser;
}

BluetoothScanner *HardwareManager::bluetoothScanner()
{
    return m_bluetoothScanner;
}

bool HardwareManager::isAvailable(const HardwareResource::Type &hardwareResourceType) const
{
    foreach (HardwareResource *resource, m_hardwareResources) {
        if (resource->hardwareReourceType() == hardwareResourceType && resource->available()) {
            return true;
        }
    }
    return false;
}

bool HardwareManager::isEnabled(const HardwareResource::Type &hardwareResourceType) const
{
    foreach (HardwareResource *resource, m_hardwareResources) {
        if (resource->hardwareReourceType() == hardwareResourceType && resource->enabled()) {
            return true;
        }
    }
    return false;
}

bool HardwareManager::enableHardwareReource(const HardwareResource::Type &hardwareResourceType)
{
    foreach (HardwareResource *resource, m_hardwareResources) {
        if (resource->hardwareReourceType() == hardwareResourceType) {
            return resource->enabled();
        }
    }
    return false;
}

bool HardwareManager::disableHardwareReource(const HardwareResource::Type &hardwareResourceType)
{
    foreach (HardwareResource *resource, m_hardwareResources) {
        if (resource->hardwareReourceType() == hardwareResourceType) {
             return resource->disable();
        }
    }
    return false;
}

