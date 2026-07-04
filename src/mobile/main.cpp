#include "backend/appbackend.h"
#include "backend/themecontroller.h"
#include "core/recurrence.h"
#include "db/databasemanager.h"
#include "utils/backupmanager.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName(QStringLiteral("Masareef"));
    QGuiApplication::setApplicationName(QStringLiteral("Masareef"));
    QGuiApplication::setApplicationVersion(QStringLiteral(MASAREEF_VERSION));

    if (const auto opened = DatabaseManager::instance().initialize(); !opened) {
        qCritical("Could not open the database: %s",
                  qPrintable(opened.error().message));
        return 1;
    }

    // Auto startup backup (keep-newest-10 policy in BackupManager)
    BackupManager::createStartupBackup();

    // Register Recurrence enum for QML
    qmlRegisterUncreatableMetaObject(RecurrenceNS::staticMetaObject,
                                     "Masareef", 1, 0, "Recurrence",
                                     "Recurrence is an enum namespace");

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    // QML singletons are created lazily; force them now so the stored
    // language (translator + layout direction) and theme are applied before
    // the first frame, not when Settings first happens to reference them.
    engine.singletonInstance<AppBackend*>("Masareef", "AppBackend");
    engine.singletonInstance<ThemeController*>("Masareef", "ThemeController");
    engine.loadFromModule("Masareef", "Main");
    return app.exec();
}
