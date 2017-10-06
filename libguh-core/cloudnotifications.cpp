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

#include "cloudnotifications.h"
#include "loggingcategories.h"

#include <QDebug>

DeviceClassId cloudNotificationsDeviceClassId = DeviceClassId("81c1bbcc-543a-48fd-bd18-ab6a76f9c38d");
ParamTypeId notifyActionParamTitleId = ParamTypeId("096503fc-b343-4d7f-8387-96162faf0f8e");

CloudNotifications::CloudNotifications(QObject *parent): DevicePlugin(parent)
{

}

QJsonObject CloudNotifications::metaData() const
{
    QVariantMap pluginMetaData;
    pluginMetaData.insert("name", "CloudNotifications");
    pluginMetaData.insert("id", "ccc6dbc8-e352-48a1-8e87-3c89a4669fc2");
    pluginMetaData.insert("idName", "cloudNotifications");

    QVariantList interfaces;
    interfaces.append("notifications");

    QVariantMap notifyActionParamTitle;
    notifyActionParamTitle.insert("id", notifyActionParamTitleId);
    notifyActionParamTitle.insert("idName", "title");
    notifyActionParamTitle.insert("name", "title");
    notifyActionParamTitle.insert("type", "QString");

    QVariantList notifyActionParamTypes;
    notifyActionParamTypes.append(notifyActionParamTitle);

    QVariantMap notifyAction;
    notifyAction.insert("id", "211d1f25-28e7-4eba-8938-b29de0e41571");
    notifyAction.insert("idName", "notify");
    notifyAction.insert("name", "notify");
    notifyAction.insert("paramTypes", notifyActionParamTypes);

    QVariantList actionTypes;
    actionTypes.append(notifyAction);

    QVariantList createMethods;
    createMethods.append("auto");

    QVariantMap cloudNotificationsDeviceClass;
    cloudNotificationsDeviceClass.insert("id", cloudNotificationsDeviceClassId);
    cloudNotificationsDeviceClass.insert("idName", "cloudNotifications");
    cloudNotificationsDeviceClass.insert("name", "CloudNotifications");
    cloudNotificationsDeviceClass.insert("createMethods", createMethods);
    cloudNotificationsDeviceClass.insert("paramTypes", QVariantMap());
    cloudNotificationsDeviceClass.insert("interfaces", interfaces);
    cloudNotificationsDeviceClass.insert("actionTypes", actionTypes);

    QVariantList deviceClasses;
    deviceClasses.append(cloudNotificationsDeviceClass);

    QVariantMap guhVendor;
    guhVendor.insert("id", "2062d64d-3232-433c-88bc-0d33c0ba2ba6"); // guh's id
    guhVendor.insert("idName", "guh");
    guhVendor.insert("name", "guh");
    guhVendor.insert("deviceClasses", deviceClasses);

    QVariantList vendors;
    vendors.append(guhVendor);
    pluginMetaData.insert("vendors", vendors);
    return QJsonObject::fromVariantMap(pluginMetaData);
}

DeviceManager::DeviceSetupStatus CloudNotifications::setupDevice(Device *device)
{
    Q_UNUSED(device)
    return DeviceManager::DeviceSetupStatusSuccess;
}

void CloudNotifications::startMonitoringAutoDevices()
{
    if (!myDevices().isEmpty()) {
        // Already have a device...
        return;
    }
    DeviceDescriptor descriptor(cloudNotificationsDeviceClassId, "Phone notification", "Send notifications to your phone");
    emit autoDevicesAppeared(cloudNotificationsDeviceClassId, {descriptor});
}

DeviceManager::DeviceError CloudNotifications::executeAction(Device *device, const Action &action)
{
    qCDebug(dcCloud()) << "executeAction" << device << action.id() << action.params();
    emit notify(action.param(notifyActionParamTitleId).value().toString(), "");
    return DeviceManager::DeviceErrorNoError;
}

DeviceManager::HardwareResources CloudNotifications::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}
