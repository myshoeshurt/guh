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

#include "pushbuttondbusservice.h"
#include "loggingcategories.h"

#include <QDBusConnection>
#include <QDebug>

namespace guhserver {

PushButtonDBusService::PushButtonDBusService(const QString &serviceName, const QString &objectPath, UserManager *parent) : QObject(parent),
    m_userManager(parent)
{
    bool status = QDBusConnection::systemBus().registerService(serviceName);
    if (!status) {
        qCWarning(dcUserManager) << "Failed to register PushButton D-Bus service. PushButton Auth will not work.";
        return;
    }
    status = QDBusConnection::systemBus().registerObject(objectPath, this, QDBusConnection::ExportScriptableContents);
    if (!status) {
        qCWarning(dcUserManager) << "Failed to register PushButton D-Bus object. PushButton Auth will not work.";
    }
    qCDebug(dcUserManager()) << "PushButton D-Bus service set up.";
}

void PushButtonDBusService::PushButtonPressed()
{
    m_userManager->pushButtonPressed();
}

QByteArray PushButtonDBusService::GenerateAuthToken(const QString &deviceName)
{
    int transactionId = m_userManager->requestPushButtonAuth(deviceName);
    bool success = false;
    QByteArray token;
    QMetaObject::Connection c = connect(m_userManager, &UserManager::pushButtonAuthFinished, this, [&] (int i, bool s, const QByteArray &t) {
        if (transactionId == i) {
            success = s;
            token = t;
        }
    });
    m_userManager->pushButtonPressed();
    disconnect(c);
    return token;


}

}
