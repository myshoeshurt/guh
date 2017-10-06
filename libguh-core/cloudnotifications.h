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

#ifndef CLOUDNOTIFICATIONS_H
#define CLOUDNOTIFICATIONS_H

#include "plugin/deviceplugin.h"

class CloudNotifications : public DevicePlugin
{
    Q_OBJECT

//    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "deviceplugincloudnotifications.json")
    Q_INTERFACES(DevicePlugin)

public:
    CloudNotifications(QObject* parent = nullptr);

    QJsonObject metaData() const;

    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void startMonitoringAutoDevices() override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

    DeviceManager::HardwareResources requiredHardware() const override;

signals:
    void notify(const QString &title, const QString &text);
};

#endif // CLOUDNOTIFICATIONS_H
