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

void setTheme(ThemePreference theme)
{
    const QString value = theme == ThemePreference::Light ? QStringLiteral("light")
        : theme == ThemePreference::Dark                  ? QStringLiteral("dark")
                                                          : QStringLiteral("system");
    settings().setValue(QStringLiteral("appearance/theme"), value);
}

} // namespace AppConfig
