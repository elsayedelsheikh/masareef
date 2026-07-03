#include "db/databasemanager.h"
#include "mainwindow.h"
#include "utils/backupmanager.h"
#include "utils/theme.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Masareef"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    Theme::apply();

    // Snapshot the DB before any connection touches it
    BackupManager::createStartupBackup();

    if (const Result<void> opened = DatabaseManager::instance().initialize(); !opened) {
        QMessageBox::critical(nullptr, QStringLiteral("Masareef"),
                              QObject::tr("The database could not be opened:\n%1")
                                  .arg(opened.error().message));
        return 1;
    }

    MainWindow window;
    window.show();
    return app.exec();
}
