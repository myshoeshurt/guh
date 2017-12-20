/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

/*!
    \class EventDescriptor
    \brief Describes a certain \l{Event}.

    \ingroup guh-types
    \ingroup rules
    \inmodule libguh

    An EventDescriptor describes an \l{Event} in order to match it with a \l{guhserver::Rule}.

    An EventDescriptor can either be bound to a certain device/eventtype, or to an interface.
    If an event is bound to a device, it will only match when the given device fires the given event.
    If an event is bound to an interface, it will match the given event for all the devices implementing
    the given interface.

    \sa Event, EventType, guhserver::Rule
*/

#include "eventdescriptor.h"

/*! Constructs an EventDescriptor describing an \l{Event} with the given \a eventTypeId, \a deviceId and the given \a paramDescriptors. */
EventDescriptor::EventDescriptor(const EventTypeId &eventTypeId, const DeviceId &deviceId, const QList<ParamDescriptor> &paramDescriptors):
    m_eventTypeId(eventTypeId),
    m_deviceId(deviceId),
    m_paramDescriptors(paramDescriptors)
{
}

EventDescriptor::EventDescriptor(const QString &interface, const QString &interfaceEvent, const QList<ParamDescriptor> &paramDescriptors):
    m_interface(interface),
    m_interfaceEvent(interfaceEvent),
    m_paramDescriptors(paramDescriptors)
{

}

EventDescriptor::Type EventDescriptor::type() const
{
    return (!m_deviceId.isNull() && !m_eventTypeId.isNull()) ? TypeDevice : TypeInterface;
}

/*! Returns the id of the \l{EventType} which describes this Event. */
EventTypeId EventDescriptor::eventTypeId() const
{
    return m_eventTypeId;
}

/*! Returns the id of the \l{Device} associated with this Event. */
DeviceId EventDescriptor::deviceId() const
{
    return m_deviceId;
}

/*! Returns the interface associated with this EventDescriptor. */
QString EventDescriptor::interface() const
{
    return m_interface;
}

/*! Returns the interface's event name associated with this EventDescriptor.*/
QString EventDescriptor::interfaceEvent() const
{
    return m_interfaceEvent;
}

/*! Returns the parameters of this Event. */
QList<ParamDescriptor> EventDescriptor::paramDescriptors() const
{
    return m_paramDescriptors;
}

/*! Set the parameters of this Event to \a paramDescriptors. */
void EventDescriptor::setParamDescriptors(const QList<ParamDescriptor> &paramDescriptors)
{
    m_paramDescriptors = paramDescriptors;
}

/*! Returns the ParamDescriptor with the given \a paramTypeId, otherwise an invalid ParamDescriptor. */
ParamDescriptor EventDescriptor::paramDescriptor(const ParamTypeId &paramTypeId) const
{
    foreach (const ParamDescriptor &paramDescriptor, m_paramDescriptors) {
        if (paramDescriptor.paramTypeId() == paramTypeId) {
            return paramDescriptor;
        }
    }
    return ParamDescriptor(QString());
}

/*! Compare this Event to the Event given by \a other.
 *  Events are equal (returns true) if eventTypeId, deviceId and params match. */
bool EventDescriptor::operator ==(const EventDescriptor &other) const
{
    bool paramsMatch = true;
    foreach (const ParamDescriptor &otherParamDescriptor, other.paramDescriptors()) {
        ParamDescriptor paramDescriptor = this->paramDescriptor(otherParamDescriptor.paramTypeId());
        if (!paramDescriptor.isValid() || paramDescriptor.value() != otherParamDescriptor.value()) {
            paramsMatch = false;
            break;
        }
    }

    return m_eventTypeId == other.eventTypeId()
            && m_deviceId == other.deviceId()
            && paramsMatch;
}

/*! Writes the eventTypeId and the deviceId of the given \a eventDescriptor to \a dbg. */
QDebug operator<<(QDebug dbg, const EventDescriptor &eventDescriptor)
{
    dbg.nospace() << "EventDescriptor(EventTypeId: " << eventDescriptor.eventTypeId().toString() << ", DeviceId" << eventDescriptor.deviceId() << ")";

    return dbg.space();
}

/*! Writes each \a eventDescriptors to \a dbg. */
QDebug operator<<(QDebug dbg, const QList<EventDescriptor> &eventDescriptors)
{
    dbg.nospace() << "EventDescriptorList (count:" << eventDescriptors.count() << ")";
    for (int i = 0; i < eventDescriptors.count(); i++ ) {
        dbg.nospace() << "     " << i << ": " << eventDescriptors.at(i);
    }

    return dbg.space();
}
