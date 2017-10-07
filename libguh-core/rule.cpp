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

/*!
    \class guhserver::Rule
    \brief This class represents a rule.

    \ingroup rules
    \inmodule core

    A Rule is always triggered by an \l{EventDescriptor}, has \l{State}{States}
    to be compared and \l{RuleAction}{RuleActions} to be executed.

    \sa EventDescriptor, State, RuleAction
*/


#include "rule.h"
#include "loggingcategories.h"

#include <QDebug>

namespace guhserver {

/*! Constructs an empty, invalid \l{Rule}. */
Rule::Rule():
    m_id(RuleId()),
    m_name(QString()),
    m_timeDescriptor(TimeDescriptor()),
    m_stateEvaluator(StateEvaluator()),
    m_eventDescriptors(QList<EventDescriptor>()),
    m_actions(QList<RuleAction>()),
    m_exitActions(QList<RuleAction>()),
    m_enabled(false),
    m_active(false),
    m_statesActive(false),
    m_timeActive(false),
    m_executable(false)
{

}

/*! Returns the id of this \l{Rule}. */
RuleId Rule::id() const
{
    return m_id;
}

/*! Sets the \a ruleId of this \l{Rule}. */
void Rule::setId(const RuleId &ruleId)
{
    m_id = ruleId;
}

/*! Returns the name of this rule. */
QString Rule::name() const
{
    return m_name;
}

/*! Sets the \a name of this \l{Rule}. */
void Rule::setName(const QString &name)
{
    m_name = name;
}

/*! Returns true if the rule is active. */
bool Rule::active() const
{
    return m_active;
}

bool Rule::statesActive() const
{
    return m_statesActive;
}

bool Rule::timeActive() const
{
    if (m_timeDescriptor.calendarItems().isEmpty())
        return true;

    return m_timeActive;
}

/*! Returns the \l{TimeDescriptor} or this Rule. */
TimeDescriptor Rule::timeDescriptor() const
{
    return m_timeDescriptor;
}

/*! Sets the \a timeDescriptor of this \l{Rule}. */
void Rule::setTimeDescriptor(const TimeDescriptor &timeDescriptor)
{
    m_timeDescriptor = timeDescriptor;
}

/*! Returns the StateEvaluator that needs to evaluate successfully in order for this to Rule apply. */
StateEvaluator Rule::stateEvaluator() const
{
    return m_stateEvaluator;
}

/*! Sets the \a stateEvaluator of this \l{Rule}. */
void Rule::setStateEvaluator(const StateEvaluator &stateEvaluator)
{
    m_stateEvaluator = stateEvaluator;
}

/*! Returns the \l{EventDescriptor} for this Rule.*/
QList<EventDescriptor> Rule::eventDescriptors() const
{
    return m_eventDescriptors;
}

/*! Sets the \a eventDescriptors of this \l{Rule}. */
void Rule::setEventDescriptors(const QList<EventDescriptor> &eventDescriptors)
{
    m_eventDescriptors = eventDescriptors;
}

/*! Returns the \l{RuleAction}{RuleActions} to be executed when this Rule is matched and states match. */
QList<RuleAction> Rule::actions() const
{
    return m_actions;
}

/*! Sets the \a actions of this \l{Rule}. */
void Rule::setActions(const QList<RuleAction> actions)
{
    m_actions = actions;
}

/*! Returns the \l{RuleAction}{RuleActions} to be executed when this Rule leaves the active state. */
QList<RuleAction> Rule::exitActions() const
{
    return m_exitActions;
}

/*! Sets the \a exitActions of this \l{Rule}. */
void Rule::setExitActions(const QList<RuleAction> exitActions)
{
    m_exitActions = exitActions;
}

/*! Returns true if the rule is enabled. */
bool Rule::enabled() const {
    return m_enabled;
}

/*! Set the \a enabled flag of this rule. In order to actually enable/disable the rule you still need to
 * update the \l{RuleEngine} */
void Rule::setEnabled(const bool &enabled)
{
    m_enabled = enabled;
}

/*! Returns true if the rule is executable. */
bool Rule::executable() const
{
    return m_executable;
}

/*! Set the rule \a executable. */
void Rule::setExecutable(const bool &executable)
{
    m_executable = executable;
}

/*! Returns true if this \l{Rule} is valid. A \l{Rule} with a valid \l{id()} is valid. */
bool Rule::isValid() const
{
    return !m_id.isNull();
}

/*! Returns true if this \l{Rule} is consistent. */
bool Rule::isConsistent() const
{
    // check if this rules is based on any event and contains exit actions
    if (!eventDescriptors().isEmpty() && !exitActions().isEmpty()) {
        qCWarning(dcRuleEngine) << "Rule not consistent. The exitActions will never be executed if the rule contains an eventDescriptor.";
        return false;
    }

    // check if this rules is based on any time events and contains exit actions
    if (!timeDescriptor().timeEventItems().isEmpty() && !exitActions().isEmpty()) {
        qCWarning(dcRuleEngine) << "Rule not consistent. The exitActions will never be executed if the rule contains an timeEvents.";
        return false;
    }

    // check if there are any actions
    if (actions().isEmpty()) {
        qCWarning(dcRuleEngine) << "Rule not consistent. A rule without actions has no effect.";
        return false;
    }

    return true;
}

void Rule::setStatesActive(const bool &statesActive)
{
    m_statesActive = statesActive;
}

void Rule::setTimeActive(const bool &timeActive)
{
    m_timeActive = timeActive;
}

void Rule::setActive(const bool &active)
{
    m_active = active;
}

}
