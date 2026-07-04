#include "backend/appbackend.h"

#include "utils/appconfig.h"
#include "utils/backupmanager.h"
#include "utils/currencyformatter.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QGuiApplication>
#include <QLocale>
#include <QQmlEngine>
#include <QStandardPaths>

// Keeps the Qt-internal layout-direction key in our catalog: lupdate would
// otherwise mark it obsolete and lrelease would drop it from the .qm.
[[maybe_unused]] static const char* const kLayoutDirectionKey[] = QT_TRANSLATE_NOOP3(
    "QGuiApplication", "QT_LAYOUT_DIRECTION",
    "Translate this string to the string 'LTR' in left-to-right languages or "
    "to 'RTL' in right-to-left languages (such as Hebrew and Arabic) to get "
    "proper widget layout.");

AppBackend::AppBackend(QObject* parent)
    : QObject(parent)
    , m_language(AppConfig::language())
{
    applyLanguage();
}

QString AppBackend::currencyCode() const
{
    return AppConfig::currencyCode();
}

void AppBackend::setCurrencyCode(const QString& code)
{
    if (AppConfig::currencyCode() == code)
        return;
    AppConfig::setCurrencyCode(code);
    emit currencyCodeChanged();
}

void AppBackend::setLanguage(const QString& language)
{
    if (m_language == language)
        return;
    m_language = language;
    AppConfig::setLanguage(language);
    applyLanguage();
    emit languageChanged();
}

QString AppBackend::localeName() const
{
    if (m_effectiveLanguage == QLatin1String("ar"))
        return QStringLiteral("ar_EG");
    // Explicit English gets a stable locale; "system" keeps the OS one so
    // dates match the rest of the device.
    return m_language == QLatin1String("system") ? QLocale::system().name()
                                                 : QStringLiteral("en_US");
}

QString AppBackend::version() const
{
    return QCoreApplication::applicationVersion();
}

QString AppBackend::formatMoney(qint64 minorUnits) const
{
    return CurrencyFormatter::format(Money::fromMinorUnits(minorUnits));
}

QString AppBackend::formatMoneyPlain(qint64 minorUnits) const
{
    return CurrencyFormatter::formatPlain(Money::fromMinorUnits(minorUnits));
}

qint64 AppBackend::parseMoney(const QString& text) const
{
    const std::optional<Money> amount = CurrencyFormatter::parse(text);
    return amount ? amount->minorUnits() : -1;
}

void AppBackend::applyLanguage()
{
    m_effectiveLanguage = m_language;
    if (m_language == QLatin1String("system")) {
        m_effectiveLanguage = QLocale::system().language() == QLocale::Arabic
            ? QStringLiteral("ar")
            : QStringLiteral("en");
    }

    // English is the source language: no translator. Installing/removing a
    // translator sends LanguageChange, which retranslates the QML engine
    // and re-evaluates the layout direction (the ar catalog carries
    // QT_LAYOUT_DIRECTION=RTL).
    const bool wantArabic = m_effectiveLanguage == QLatin1String("ar");
    if (wantArabic && !m_arTranslatorInstalled) {
        if (m_arTranslator.load(QStringLiteral(":/i18n/masareef_ar.qm")))
            m_arTranslatorInstalled =
                QCoreApplication::installTranslator(&m_arTranslator);
    } else if (!wantArabic && m_arTranslatorInstalled) {
        QCoreApplication::removeTranslator(&m_arTranslator);
        m_arTranslatorInstalled = false;
    }

    // installTranslator() only notifies the application object, which the
    // QML engine does not observe — re-evaluate the qsTr bindings ourselves.
    // Null during construction, when nothing is loaded yet anyway.
    if (QQmlEngine* engine = qmlEngine(this))
        engine->retranslate();

    // Drive the direction from the effective language rather than trusting
    // the catalog's QT_LAYOUT_DIRECTION entry alone; QML LayoutMirroring
    // binds to Qt.application.layoutDirection.
    if (qGuiApp) {
        QGuiApplication::setLayoutDirection(wantArabic ? Qt::RightToLeft
                                                       : Qt::LeftToRight);
    }
}

bool AppBackend::backupNow()
{
    // ponytail: backing up to standard documents folder; upgrade to file picker if needed
    const QString docsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString destFile = QDir(docsPath).filePath(
        QStringLiteral("masareef-backup-%1.db")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"))));
    return static_cast<bool>(BackupManager::backupTo(destFile));
}

QStringList AppBackend::backups() const
{
    // ponytail: list backups from documents folder; upgrade if a proper backup folder is added
    const QString docsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QDir backupDir(docsPath);
    QStringList filenames = backupDir.entryList(QStringList() << QStringLiteral("masareef-backup-*.db"),
                                                QDir::Files, QDir::Time);
    QStringList absolutePaths;
    absolutePaths.reserve(filenames.size());
    for (const QString& filename : filenames)
        absolutePaths.append(backupDir.filePath(filename));
    return absolutePaths;
}

bool AppBackend::restore(const QString& path)
{
    const bool ok = static_cast<bool>(BackupManager::restoreFrom(path));
    if (ok)
        emit modelsRefreshNeeded();
    return ok;
}
