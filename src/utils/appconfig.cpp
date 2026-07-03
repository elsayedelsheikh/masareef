#include "utils/appconfig.h"

#include <QSettings>

namespace {
QSettings settings()
{
    return QSettings(QStringLiteral("Masareef"), QStringLiteral("Masareef"));
}
} // namespace

namespace AppConfig {

QString currencyCode()
{
    return settings()
        .value(QStringLiteral("currency/code"), QStringLiteral("EGP"))
        .toString();
}

void setCurrencyCode(const QString& code)
{
    settings().setValue(QStringLiteral("currency/code"), code);
}

ThemePreference theme()
{
    const QString value =
        settings().value(QStringLiteral("appearance/theme"), QStringLiteral("system"))
            .toString();
    if (value == QLatin1String("light"))
        return ThemePreference::Light;
    if (value == QLatin1String("dark"))
        return ThemePreference::Dark;
    return ThemePreference::System;
}

QString language()
{
    // Not "general/...": QSettings' INI format reserves the General section
    // for top-level keys and mangles a group of that name to "%General",
    // which no longer matches on re-read in a fresh process.
    const QString value =
        settings().value(QStringLiteral("locale/language"), QStringLiteral("system"))
            .toString();
    if (value == QLatin1String("en") || value == QLatin1String("ar"))
        return value;
    return QStringLiteral("system");
}

void setLanguage(const QString& language)
{
    settings().setValue(QStringLiteral("locale/language"), language);
}

void setTheme(ThemePreference theme)
{
    const QString value = theme == ThemePreference::Light ? QStringLiteral("light")
        : theme == ThemePreference::Dark                  ? QStringLiteral("dark")
                                                          : QStringLiteral("system");
    settings().setValue(QStringLiteral("appearance/theme"), value);
}

} // namespace AppConfig
