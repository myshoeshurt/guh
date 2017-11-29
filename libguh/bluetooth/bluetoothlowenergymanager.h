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

#ifndef BLUETOOTHLOWENERGYMANAGER_H
#define BLUETOOTHLOWENERGYMANAGER_H

#include <QTimer>
#include <QObject>
#include <QPointer>
#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>

#include "hardwareresource.h"
#include "bluetoothdiscoveryreply.h"

class BluetoothLowEnergyManager : public HardwareResource
{
    Q_OBJECT

    friend class HardwareManager;

public:
     BluetoothDiscoveryReply *discoverDevices(const int &interval = 5000);

private:
    explicit BluetoothLowEnergyManager(QObject *parent = nullptr);
    QTimer *m_timer = nullptr;

    QList<QBluetoothDeviceDiscoveryAgent *> m_bluetoothDiscoveryAgents;
    QList<QBluetoothDeviceInfo> m_discoveredDevices;

    QPointer<BluetoothDiscoveryReply> m_currentReply;

signals:

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &deviceInfo);
    void onDiscoveryError(const QBluetoothDeviceDiscoveryAgent::Error &error);

    void discoveryTimeout();

public slots:
    bool enable();
    bool disable();

};

#endif // BLUETOOTHLOWENERGYMANAGER_H
