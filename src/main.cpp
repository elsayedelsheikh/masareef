#include "db/databasemanager.h"
#include "mainwindow.h"
#include "utils/backupmanager.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Masareef"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    // Snapshot the DB before any connection touches it
    BackupManager::createStartupBackup();

    QString error;
    if (!DatabaseManager::instance().initialize(&error)) {
        QMessageBox::critical(nullptr, QStringLiteral("Masareef"),
                              QObject::tr("The database could not be opened:\n%1").arg(error));
        return 1;
    }

    MainWindow window;
    window.show();
    return app.exec();
}
