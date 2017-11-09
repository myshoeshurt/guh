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
    explicit PushButtonDBusService(UserManager *parent);

    Q_SCRIPTABLE void PushButtonPressed();
    Q_SCRIPTABLE QByteArray GenerateAuthToken(const QString &deviceName);

signals:
    void buttonPressed();

private:
    UserManager *m_userManager;
};

}

#endif // PUSHBUTTONDBUSSERVICE_H
