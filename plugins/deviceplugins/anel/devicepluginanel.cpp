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

/*!
    \page anel.html
    \title Anel
    \brief Plugin for Anel network controlled power plugs.

    \ingroup plugins
    \ingroup guh-plugins

    This plugin allows to control the Anel network controlled power plugs.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    For more details how to read this JSON file please check out the documentation for \l{The plugin JSON File}.

    \quotefile plugins/deviceplugins/netatmo/devicepluginanel.json
*/

#include "devicepluginanel.h"
#include "plugin/device.h"
#include "plugininfo.h"

#include <QDebug>
#include <QUrlQuery>
#include <QJsonDocument>

DevicePluginAnel::DevicePluginAnel()
{
    m_discoveryTimer.setSingleShot(true);
    m_discoveryTimer.setInterval(1000);
    connect(&m_discoveryTimer, &QTimer::timeout, this, &DevicePluginAnel::finishDiscovery);
    setupUdp();
}

DeviceManager::HardwareResources DevicePluginAnel::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceSetupStatus DevicePluginAnel::setupDevice(Device *device)
{
    Q_UNUSED(device)
    return DeviceManager::DeviceSetupStatusSuccess;
}

void DevicePluginAnel::postSetupDevice(Device *device)
{
    if (device->deviceClassId() == netControlDeviceClassId) {
        foreach (const QByteArray &discoveryData, m_discoveredDevices.keys()) {
            if (m_discoveredDevices.value(discoveryData).params().paramValue(bridgeIpParamTypeId) == device->paramValue(bridgeIpParamTypeId)) {
                QList<QByteArray> parts = discoveryData.split(':');
                qCDebug(dcAnel) << "have data for device:" << discoveryData;
                QList<DeviceDescriptor> descriptors;
                for (int i = 0; i < 8; ++i) {
                    Param ipParam(socketIpParamTypeId, parts.at(2));
                    Param portParam(socketPortParamTypeId, device->paramValue(bridgePortParamTypeId));
                    Param numberParam(socketNumberParamTypeId, i+1);
                    ParamList params;
                    params.append(ipParam);
                    params.append(portParam);
                    params.append(numberParam);
                    qCDebug(dcAnel) << "adding socket with name:" << parts.at(6+i).split(',').first();
                    DeviceDescriptor descriptor(socketDeviceClassId, parts.at(6+i).split(',').first());
                    descriptor.setParams(params);
                    descriptors.append(descriptor);
                }
                emit autoDevicesAppeared(socketDeviceClassId, descriptors);
            }
        }
    }
}

DeviceManager::DeviceError DevicePluginAnel::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    qCDebug(dcAnel) << "Discovering devices" << deviceClassId << params;
    Q_UNUSED(deviceClassId)
    Q_UNUSED(params)

    m_discoveredDevices.clear();

    bool ok = m_socket->writeDatagram("wer da?", QHostAddress::Broadcast, 7575);
    if (!ok) {
        qCDebug(dcAnel) << "Error writing discovery message";
        return DeviceManager::DeviceErrorHardwareNotAvailable;
    } else {
        qCDebug(dcAnel) << "Discovering on port" << 7575 << "...";
    }
    m_discoveryTimer.start();
    return DeviceManager::DeviceErrorAsync;
}

void DevicePluginAnel::deviceRemoved(Device *device)
{
    Q_UNUSED(device)
}

DeviceManager::DeviceError DevicePluginAnel::executeAction(Device *device, const Action &action)
{
    QHostAddress ip(QHostAddress(device->paramValue(socketIpParamTypeId).toString()));
    int port = device->paramValue(socketPortParamTypeId).toInt();
    int socket = device->paramValue(socketNumberParamTypeId).toInt();
    QString data = QString("Sw_%1%2%3%4").arg(action.param(setPowerActionPowerParamTypeId).value().toBool() ? "on" : "off").arg(socket).arg("admin").arg("anel");
    qCDebug(dcAnel) << "Switching:" << data;
    m_socket->writeDatagram(data.toLocal8Bit(), ip, port);

    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginAnel::setupUdp()
{
    if (m_socket != nullptr) {
        m_socket->close();
        delete m_socket;
    }
    m_socket = new QUdpSocket(this);
    bool ok = m_socket->bind(QHostAddress(QHostAddress::AnyIPv4), m_port, QUdpSocket::ShareAddress);
    if (!ok) {
        qCDebug(dcAnel) << "Error binding UDP port" << m_port;
    } else {
        qCDebug(dcAnel) << "Bound to UDP port" << m_port;
    }
    ok = m_socket->open(QIODevice::ReadWrite);
    if (!ok) {
        qCDebug(dcAnel) << "Error opening socket";
    } else {
        qCDebug(dcAnel) << "Opened socket.";
    }
    connect(m_socket, &QUdpSocket::readyRead, this, &DevicePluginAnel::dataAvailable);
}

void DevicePluginAnel::dataAvailable()
{
    char buffer[1024];
    QHostAddress sender;
    quint16 port;
    m_socket->readDatagram(buffer, 1024, &sender, &port);
    qCDebug(dcAnel) << "have data:" << buffer << sender << port;
    QByteArray data(buffer);

    QList<QByteArray> parts = data.split(':');
    qDebug() << parts.length();
    if (parts.length() != 16) {
        qDebug() << "Invalid packet";
        return;
    }

    foreach (const DeviceDescriptor &descriptor, m_discoveredDevices) {
        if (descriptor.params().paramValue(bridgeIpParamTypeId).toByteArray() == parts.at(2)) {
            // skipping dupe
            return;
        }
    }
    qCDebug(dcAnel) << "have new device:" << parts.at(1);
    Param ipParam(bridgeIpParamTypeId, parts.at(2));
    Param portParam(bridgePortParamTypeId, port);
    ParamList params;
    params.append(ipParam);
    params.append(portParam);
    DeviceDescriptor descriptor(netControlDeviceClassId, parts.at(1).trimmed(), parts.at(2));
    descriptor.setParams(params);
    m_discoveredDevices.insert(data, descriptor);
}

void DevicePluginAnel::finishDiscovery()
{
    emit devicesDiscovered(netControlDeviceClassId, m_discoveredDevices.values());
}
