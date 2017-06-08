#include "jsplugin.h"

#include <QJSEngine>
#include <QJSValueIterator>
#include <QLoggingCategory>
#include <QDebug>

Q_DECLARE_LOGGING_CATEGORY(dcJs)
Q_LOGGING_CATEGORY(dcJs, "JS")


JsPlugin::JsPlugin(QObject *parent): DevicePlugin(parent)
{
    m_engine = new QJSEngine(this);

    QString fileName = "/home/micha/jsdemoplugin.js";
    QFile scriptFile(fileName);
    if (!scriptFile.open(QIODevice::ReadOnly)) {
        // handle error
    }
    QTextStream stream(&scriptFile);

    QString contents = stream.readAll();
    scriptFile.close();
    QJSValue ret = m_engine->evaluate(contents, fileName);

    QJSValueIterator it(ret);
    while (it.hasNext()) {
        it.next();
        qDebug() << "value:" << it.name();
    }

    qWarning() << "loaded:" << ret.isError();
    qWarning() << "has setup:" << ret.hasProperty("setup");

    qWarning() << "**************************************" << ret.toVariant() << ret.isError() << ret.property("lineNumber").toInt()
               << ":" << ret.toString();
}

DeviceManager::HardwareResources JsPlugin::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}
