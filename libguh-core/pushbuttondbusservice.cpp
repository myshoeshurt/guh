#include "pushbuttondbusservice.h"
#include "loggingcategories.h"

#include <QDBusConnection>
#include <QDebug>

namespace guhserver {

PushButtonDBusService::PushButtonDBusService(UserManager *parent) : QObject(parent),
    m_userManager(parent)
{
    bool status = QDBusConnection::systemBus().registerService("io.guh.nymead");
    if (!status) {
        qCWarning(dcUserManager) << "Failed to register PushButton D-Bus service. PushButton Auth will not work.";
        return;
    }
    status = QDBusConnection::systemBus().registerObject("/io/guh/nymead", this, QDBusConnection::ExportScriptableContents);
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
