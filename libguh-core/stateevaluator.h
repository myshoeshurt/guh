/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef STATEEVALUATOR_H
#define STATEEVALUATOR_H

#include "types/state.h"
#include "types/statedescriptor.h"

#include <QDebug>

class GuhSettings;

namespace guhserver {

class StateEvaluator
{
public:
    StateEvaluator(const StateDescriptor &stateDescriptor);
    StateEvaluator(QList<StateEvaluator> childEvaluators = QList<StateEvaluator>(), Types::StateOperator stateOperator = Types::StateOperatorAnd);

    StateDescriptor stateDescriptor() const;

    QList<StateEvaluator> childEvaluators() const;
    void setChildEvaluators(const QList<StateEvaluator> &childEvaluators);
    void appendEvaluator(const StateEvaluator &stateEvaluator);

    Types::StateOperator operatorType() const;
    void setOperatorType(Types::StateOperator operatorType);

    bool evaluate() const;
    bool containsDevice(const DeviceId &deviceId) const;

    void removeDevice(const DeviceId &deviceId);
    QList<DeviceId> containedDevices() const;

    void dumpToSettings(GuhSettings &settings, const QString &groupName) const;
    static StateEvaluator loadFromSettings(GuhSettings &settings, const QString &groupPrefix);

    bool isValid() const;

private:
    StateDescriptor m_stateDescriptor;

    QList<StateEvaluator> m_childEvaluators;
    Types::StateOperator m_operatorType;
};

}

#endif // STATEEVALUATOR_H
