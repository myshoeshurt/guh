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

#ifndef PARAMTYPE_H
#define PARAMTYPE_H

#include <QVariant>
#include <QDebug>

#include "libguh.h"
#include "typeutils.h"

class LIBGUH_EXPORT ParamType
{
public:
    ParamType() = default;
    ParamType(const ParamTypeId &id, const QString &name, const QVariant::Type type, const QVariant &defaultValue = QVariant());

    ParamTypeId id() const;

    QString name() const;
    void setName(const QString &name);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    int index() const;
    void setIndex(const int &index);

    QVariant::Type type() const;
    void setType(QVariant::Type type);

    QVariant defaultValue() const;
    void setDefaultValue(const QVariant &defaultValue);

    QVariant minValue() const;
    void setMinValue(const QVariant &minValue);

    QVariant maxValue() const;
    void setMaxValue(const QVariant &maxValue);

    Types::InputType inputType() const;
    void setInputType(const Types::InputType &inputType);

    Types::Unit unit() const;
    void setUnit(const Types::Unit &unit);

    QPair<QVariant, QVariant> limits() const;
    void setLimits(const QVariant &min, const QVariant &max);

    QList<QVariant> allowedValues() const;
    void setAllowedValues(const QList<QVariant> &allowedValues);

    bool readOnly() const;
    void setReadOnly(const bool &readOnly);

    bool isValid() const;

private:
    ParamTypeId m_id;
    QString m_name;
    QString m_displayName;
    int m_index;
    QVariant::Type m_type;
    QVariant m_defaultValue;
    QVariant m_minValue;
    QVariant m_maxValue;
    Types::InputType m_inputType;
    Types::Unit m_unit;
    QVariantList m_allowedValues;
    bool m_readOnly;
};

class ParamTypes: public QList<ParamType>
{
public:
    ParamTypes() = default;
    ParamTypes(const QList<ParamType> &other);
    ParamType findByName(const QString &name);
    ParamType findById(const ParamTypeId &id);
};

QDebug operator<<(QDebug dbg, const ParamType &paramType);
QDebug operator<<(QDebug dbg, const QList<ParamType> &paramTypes);

#endif // PARAMTYPE_H
