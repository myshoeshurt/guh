/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 -2016 Simon Stürz <simon.stuerz@guh.io>             *
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
  \class Radio433
  \brief The Radio433 class helps to interact with the 433 MHz receiver and transmitter.

  \ingroup hardware
  \inmodule libguh

  This class handles all supported radio 433 MHz transmitter. Receiving data on the 433.92 MHz frequency
  is only supported, if there are \l{Gpio}{GPIO's} available and a suitable receiver is connected to GPIO 27. Examples for receiver
  can be found \l{https://www.futurlec.com/Radio-433MHZ.shtml}{here}. The antenna has a very large impact on the quality
  of the signal and how well it is recognized. In many forums and blogs it is described that a 17, 3 mm piece of wire is enough.
  Experiments have shown, it's not. A 50 Ohm coaxial cabel (thickness = 1mm), mounted on the antenna pin of the receiver
  with a minimum distance of 5 cm away from the circuit and unisolated 17.3 mm at the end has shown the best results.

  In order to send data to a 433 MHz device, there currently are two possibilitis. If there are \l{Gpio}{GPIO's}
  available, the data will be sent over the transmitter connected to GPIO 22. Also in this case the antenna is a verry
  important part.

  The second possibility to sent data to a 433 MHz device is the \l{http://www.brennenstuhl.de/de-DE/steckdosenleisten-schaltgeraete-und-adapter/brematic-hausautomation/brematic-home-automation-gateway-gwy-433-1.html}
  {Brennenstuhl 433 MHz LAN Gateway}. If there is a Gateway in the local network, this class will automaticaly detect
  it and will be used. If both transmitter are available (Gateway + GPIO), each signal will be transmitted over both sender.

  \note: Radio 433 on GPIO's is by default disabled. If you want to enable it, you need to compile the source with the qmake config \tt{CONFIG+=radio433gpio}

*/


#include "radio433.h"
#include "loggingcategories.h"
#include "guhsettings.h"
#include "hardware/gpio.h"

#include <QFileInfo>

/*! Construct the hardware resource Radio433 with the given \a parent. Each possible 433 MHz hardware will be initialized here. */
Radio433::Radio433(QObject *parent) :
    HardwareResource(HardwareResource::TypeRadio433, "Radio 433 MHz", parent)
{
    m_brennenstuhlTransmitter = new Radio433BrennenstuhlGateway(this);
    connect(m_brennenstuhlTransmitter, &Radio433BrennenstuhlGateway::availableChanged, this, &Radio433::brennenstuhlAvailableChanged);

    setAvailable(false);

    qCDebug(dcHardware()) << "-->" << name() << "created successfully.";
}

/*! Enables GPIO transmitter and receiver and the Brennenstuhl Lan Gateway.
 *  Returns true, if the GPIO's are available and set up correctly. The status of the gateway will be emited asynchronous. */
bool Radio433::enable()
{
    m_brennenstuhlTransmitter->enable();
    setEnabled(true);
    return true;
}

/*! Returns true, if the Radio433 hardware resources disabled correctly. */
bool Radio433::disable()
{
    m_brennenstuhlTransmitter->disable();
    setEnabled(false);
    return true;
}

void Radio433::brennenstuhlAvailableChanged(const bool &available)
{
    if (available) {
        qCDebug(dcHardware()) << name() << "Brennenstuhl LAN Gateway available.";
        setAvailable(true);
    } else {
        qCWarning(dcHardware()) << name() << "Brennenstuhl LAN Gateway not available.";
        setAvailable(false);
    }
}

/*! Returns true, if the \a rawData with a certain \a delay (pulse length) could be sent \a repetitions times. */
bool Radio433::sendData(int delay, QList<int> rawData, int repetitions)
{
    if (!available()) {
        qCWarning(dcHardware()) << name() << "Brennenstuhl gateway not available";
        return false;
    }

    if (!enabled()) {
        qCWarning(dcHardware()) << name() << "Hardware reouce disabled.";
        return false;
    }

    return m_brennenstuhlTransmitter->sendData(delay, rawData, repetitions);
}
