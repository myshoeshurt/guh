/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2017 Simon Stürz <simon.stuerz@guh.io>              *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "libguh.h"

#include "plugin/deviceclass.h"
#include "plugin/device.h"
#include "plugin/devicedescriptor.h"

#include "types/event.h"
#include "types/action.h"
#include "types/vendor.h"

#include <QObject>
#include <QTimer>
#include <QLocale>
#include <QPluginLoader>

#include "hardwaremanager.h"

class Device;
class DevicePlugin;
class DevicePairingInfo;
class HardwareManager;

class LIBGUH_EXPORT DeviceManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(DeviceError)

    friend class DevicePlugin;

public:
    enum DeviceError {
        DeviceErrorNoError,
        DeviceErrorPluginNotFound,
        DeviceErrorVendorNotFound,
        DeviceErrorDeviceNotFound,
        DeviceErrorDeviceClassNotFound,
        DeviceErrorActionTypeNotFound,
        DeviceErrorStateTypeNotFound,
        DeviceErrorEventTypeNotFound,
        DeviceErrorDeviceDescriptorNotFound,
        DeviceErrorMissingParameter,
        DeviceErrorInvalidParameter,
        DeviceErrorSetupFailed,
        DeviceErrorDuplicateUuid,
        DeviceErrorCreationMethodNotSupported,
        DeviceErrorSetupMethodNotSupported,
        DeviceErrorHardwareNotAvailable,
        DeviceErrorHardwareFailure,
        DeviceErrorAuthentificationFailure,
        DeviceErrorAsync,
        DeviceErrorDeviceInUse,
        DeviceErrorDeviceInRule,
        DeviceErrorDeviceIsChild,
        DeviceErrorPairingTransactionIdNotFound,
        DeviceErrorParameterNotWritable
    };
    Q_ENUM(DeviceError)

    enum DeviceSetupStatus {
        DeviceSetupStatusSuccess,
        DeviceSetupStatusFailure,
        DeviceSetupStatusAsync
    };
    Q_ENUM(DeviceSetupStatus)

    explicit DeviceManager(const QLocale &locale, QObject *parent = nullptr);
    ~DeviceManager();

    static QStringList pluginSearchDirs();
    static QList<QJsonObject> pluginsMetadata();

    void setLocale(const QLocale &locale);

    HardwareManager *hardwareManager();

    QList<DevicePlugin*> plugins() const;
    DevicePlugin* plugin(const PluginId &id) const;
    DeviceError setPluginConfig(const PluginId &pluginId, const ParamList &pluginConfig);

    QList<Vendor> supportedVendors() const;
    QList<DeviceClass> supportedDevices(const VendorId &vendorId = VendorId()) const;
    DeviceError discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params);

    QList<Device*> configuredDevices() const;
    DeviceError addConfiguredDevice(const DeviceClassId &deviceClassId, const QString &name, const ParamList &params, const DeviceId id = DeviceId::createDeviceId());
    DeviceError addConfiguredDevice(const DeviceClassId &deviceClassId, const QString &name, const DeviceDescriptorId &deviceDescriptorId, const DeviceId &id = DeviceId::createDeviceId());

    DeviceError reconfigureDevice(const DeviceId &deviceId, const ParamList &params, bool fromDiscovery = false);
    DeviceError reconfigureDevice(const DeviceId &deviceId, const DeviceDescriptorId &deviceDescriptorId);

    DeviceError editDevice(const DeviceId &deviceId, const QString &name);

    DeviceError pairDevice(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const QString &name, const ParamList &params);
    DeviceError pairDevice(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const QString &name, const DeviceDescriptorId &deviceDescriptorId);
    DeviceError confirmPairing(const PairingTransactionId &pairingTransactionId, const QString &secret = QString());

    DeviceError removeConfiguredDevice(const DeviceId &deviceId);

    Device* findConfiguredDevice(const DeviceId &id) const;
    QList<Device *> findConfiguredDevices(const DeviceClassId &deviceClassId) const;
    QList<Device *> findChildDevices(const DeviceId &id) const;
    DeviceClass findDeviceClass(const DeviceClassId &deviceClassId) const;

    DeviceError verifyParams(const QList<ParamType> paramTypes, ParamList &params, bool requireAll = true);
    DeviceError verifyParam(const QList<ParamType> paramTypes, const Param &param);
    DeviceError verifyParam(const ParamType &paramType, const Param &param);

signals:
    void loaded();
    void languageUpdated();
    void pluginConfigChanged(const PluginId &id, const ParamList &config);
    void eventTriggered(const Event &event);
    void deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);
    void deviceRemoved(const DeviceId &deviceId);
    void deviceDisappeared(const DeviceId &deviceId);
    void deviceAdded(Device *device);
    void deviceChanged(Device *device);
    void devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &devices);
    void deviceSetupFinished(Device *device, DeviceError status);
    void deviceReconfigurationFinished(Device *device, DeviceError status);
    void pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceError status, const DeviceId &deviceId = DeviceId());
    void actionExecutionFinished(const ActionId &actionId, DeviceManager::DeviceError status);

public slots:
    DeviceError executeAction(const Action &action);
    void timeTick();

private slots:
    void loadPlugins();
    void loadConfiguredDevices();
    void storeConfiguredDevices();
    void startMonitoringAutoDevices();
    void slotDevicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> deviceDescriptors);
    void slotDeviceSetupFinished(Device *device, DeviceManager::DeviceSetupStatus status);
    void slotPairingFinished(const PairingTransactionId &pairingTransactionId, DeviceManager::DeviceSetupStatus status);
    void onAutoDevicesAppeared(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors);
    void onAutoDeviceDisappeared(const DeviceId &deviceId);
    void onLoaded();
    void cleanupDeviceStateCache();

    // Only connect this to Devices. It will query the sender()
    void slotDeviceStateValueChanged(const QUuid &stateTypeId, const QVariant &value);

private:
    bool verifyPluginMetadata(const QJsonObject &data);
    DeviceError addConfiguredDeviceInternal(const DeviceClassId &deviceClassId, const QString &name, const ParamList &params, const DeviceId id = DeviceId::createDeviceId());
    DeviceSetupStatus setupDevice(Device *device);
    void postSetupDevice(Device *device);
    void storeDeviceStates(Device *device);
    void loadDeviceStates(Device *device);


private:
    QLocale m_locale;
    QHash<VendorId, Vendor> m_supportedVendors;
    QHash<VendorId, QList<DeviceClassId> > m_vendorDeviceMap;
    QHash<DeviceClassId, DeviceClass> m_supportedDevices;
    QHash<DeviceId, Device*> m_configuredDevices;
    QHash<DeviceDescriptorId, DeviceDescriptor> m_discoveredDevices;

    QHash<PluginId, DevicePlugin*> m_devicePlugins;
    QList<DevicePlugin *> m_pluginTimerUsers;

    HardwareManager *m_hardwareManager;

    QHash<QUuid, DevicePairingInfo> m_pairingsJustAdd;
    QHash<QUuid, DevicePairingInfo> m_pairingsDiscovery;

    QList<Device *> m_asyncDeviceReconfiguration;
    QList<DevicePlugin *> m_discoveringPlugins;
};

Q_DECLARE_METATYPE(DeviceManager::DeviceError)

#endif // DEVICEMANAGER_H
