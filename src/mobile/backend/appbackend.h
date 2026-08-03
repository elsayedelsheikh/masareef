#pragma once

#include <QDate>
#include <QObject>
#include <QTranslator>
#include <QtQml/qqmlregistration.h>

// App-wide QML singleton: money formatting/parsing (Money is not a QML
// type, so amounts cross the boundary as qint64 minor units + text) and the
// language preference. Setting the language installs/removes the Arabic
// translator at runtime; the resulting LanguageChange event retranslates
// the QML UI and flips the application layout direction for RTL.
class AppBackend : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString currencyCode READ currencyCode WRITE setCurrencyCode
                   NOTIFY currencyCodeChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString effectiveLanguage READ effectiveLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString localeName READ localeName NOTIFY languageChanged)
    Q_PROPERTY(QString version READ version CONSTANT)

public:
    explicit AppBackend(QObject* parent = nullptr);

    [[nodiscard]] QString currencyCode() const;
    void setCurrencyCode(const QString& code);

    // "system", "en" or "ar" (persisted preference)
    [[nodiscard]] QString language() const { return m_language; }
    void setLanguage(const QString& language);
    // The language actually in effect: "en" or "ar"
    [[nodiscard]] QString effectiveLanguage() const { return m_effectiveLanguage; }
    // Locale for dates/numbers in QML (Qt.locale(AppBackend.localeName)) —
    // qsTr covers strings, but Qt.formatDate would otherwise stay in the
    // process locale regardless of the chosen language.
    [[nodiscard]] QString localeName() const;

    [[nodiscard]] QString version() const;

    Q_INVOKABLE QString formatMoney(qint64 minorUnits) const;
    Q_INVOKABLE QString formatMoneyPlain(qint64 minorUnits) const;
    // Ungrouped "1234.50", safe to put back into an amount field
    Q_INVOKABLE QString amountForEditing(qint64 minorUnits) const;
    // -1 when the text is not a valid non-negative amount
    Q_INVOKABLE qint64 parseMoney(const QString& text) const;

    // Dates, in the UI language and always with Western digits. QML's
    // Date.toLocaleDateString() would use the process locale and Arabic-Indic
    // numerals, which disagree with the amounts shown beside them.
    //
    // These are plain calls, not properties, so a binding that uses one does
    // not re-evaluate on a language switch by itself — read `localeName`
    // alongside it (or refresh the model that owns the row).
    Q_INVOKABLE QString formatDate(QDate date) const;
    Q_INVOKABLE QString formatDateShort(QDate date) const;
    Q_INVOKABLE QString formatMonthYear(QDate date) const;
    Q_INVOKABLE QString formatRelativeDate(QDate date) const;
    Q_INVOKABLE QString formatDueLabel(QDate nextDue) const;
    // ListView section keys are ISO date strings; this turns one back into
    // the "Today" / "15 January 2026" header label.
    Q_INVOKABLE QString formatDateSection(const QString& isoDate) const;

    // Backup and restore. backupNow() closes the DB, copies it, reopens.
    // restore() validates, backs up current DB, replaces with the source,
    // reopens, and signals models to refresh.
    Q_INVOKABLE bool backupNow();
    Q_INVOKABLE QStringList backups() const;
    Q_INVOKABLE bool restore(const QString& path);

signals:
    void currencyCodeChanged();
    void languageChanged();
    void modelsRefreshNeeded();

private:
    void applyLanguage();

    QString m_language;
    QString m_effectiveLanguage;
    QTranslator m_arTranslator;
    bool m_arTranslatorInstalled = false;
};
