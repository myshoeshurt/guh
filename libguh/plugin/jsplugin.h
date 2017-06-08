#ifndef JSPLUGIN_H
#define JSPLUGIN_H

#include <QObject>
#include <QJSEngine>

#include "deviceplugin.h"

class JsPlugin : public DevicePlugin
{
public:
    JsPlugin(QObject *parent = nullptr);

    virtual DeviceManager::HardwareResources requiredHardware() const override;
private:
    QJSEngine *m_engine = nullptr;


};

#endif // JSPLUGIN_H
