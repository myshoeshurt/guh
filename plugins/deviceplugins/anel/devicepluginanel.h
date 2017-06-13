/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
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

#ifndef DEVICEPLUGINANEL_H
#define DEVICEPLUGINANEL_H

#include "plugin/deviceplugin.h"

#include <QHash>
#include <QTimer>

class DevicePluginAnel : public DevicePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "devicepluginanel.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginAnel();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void postSetupDevice(Device *device) override;

    DeviceManager::DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params) override;
    void deviceRemoved(Device *device) override;

public slots:
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

private:
    void setupUdp();

private slots:
    void dataAvailable();
    void finishDiscovery();

private:
    QUdpSocket* m_socket = nullptr;
    quint16 m_port = 7777;
//    QList<Host*> m_list;
    QHash<QByteArray, DeviceDescriptor> m_discoveredDevices;
    QTimer m_discoveryTimer;
};

#endif // DEVICEPLUGINANEL_H
