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

#ifndef PUSHBUTTONDBUSSERVICE_H
#define PUSHBUTTONDBUSSERVICE_H

#include <QObject>
#include "usermanager.h"

namespace guhserver {

class PushButtonDBusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.guh.nymead")

public:
    explicit PushButtonDBusService(const QString &serviceName, const QString &objectPath, UserManager *parent);

    Q_SCRIPTABLE void PushButtonPressed();
    Q_SCRIPTABLE QByteArray GenerateAuthToken(const QString &deviceName);

signals:
    void buttonPressed();

private:
    UserManager *m_userManager;

};

}

#endif // PUSHBUTTONDBUSSERVICE_H
