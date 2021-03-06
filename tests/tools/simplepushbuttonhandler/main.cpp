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

#include <QCoreApplication>
#include <QDebug>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "../../utils/pushbuttonagent.h"
#include "inputwatcher.h"

int main(int argc, char *argv[])
{
    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption dbusOption(QStringList() << "session", QCoreApplication::translate("main", "If specified, all D-Bus interfaces will be bound to the session bus instead of the system bus."));
    parser.addOption(dbusOption);

    QCoreApplication a(argc, argv);

    PushButtonAgent agent;
    if (!agent.init(parser.isSet(dbusOption) ? QDBusConnection::SessionBus : QDBusConnection::SystemBus)) {
        return -1;
    }
    InputWatcher inputWatcher;
    QObject::connect(&inputWatcher, &InputWatcher::enterPressed, &agent, &PushButtonAgent::sendButtonPressed);

    qDebug() << "Use the Enter key as push button.";
    return a.exec();
}
