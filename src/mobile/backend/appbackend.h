#pragma once

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
    // -1 when the text is not a valid non-negative amount
    Q_INVOKABLE qint64 parseMoney(const QString& text) const;

signals:
    void currencyCodeChanged();
    void languageChanged();

private:
    void applyLanguage();

    QString m_language;
    QString m_effectiveLanguage;
    QTranslator m_arTranslator;
    bool m_arTranslatorInstalled = false;
};
