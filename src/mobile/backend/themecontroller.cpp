#include "backend/themecontroller.h"

#include "utils/appconfig.h"
#include "utils/palette.h"

#include <QGuiApplication>
#include <QStyleHints>

ThemeController::ThemeController(QObject* parent)
    : QObject(parent)
{
    resolve(/*force=*/true);
    if (qGuiApp) {
        // Follow live OS light/dark switches while the preference is "system"
        connect(qGuiApp->styleHints(), &QStyleHints::colorSchemeChanged,
                this, [this] { resolve(); });
    }
}

QString ThemeController::preference() const
{
    switch (AppConfig::theme()) {
    case ThemePreference::Light:
        return QStringLiteral("light");
    case ThemePreference::Dark:
        return QStringLiteral("dark");
    case ThemePreference::System:
        break;
    }
    return QStringLiteral("system");
}

void ThemeController::setPreference(const QString& preference)
{
    if (this->preference() == preference)
        return;

    AppConfig::setTheme(preference == QLatin1String("light") ? ThemePreference::Light
                            : preference == QLatin1String("dark")
                            ? ThemePreference::Dark
                            : ThemePreference::System);
    emit preferenceChanged();
    resolve();
}

QColor ThemeController::surface() const { return Palette::surface(); }
QColor ThemeController::primaryInk() const { return Palette::primaryInk(); }
QColor ThemeController::secondaryInk() const { return Palette::secondaryInk(); }
QColor ThemeController::mutedInk() const { return Palette::mutedInk(); }
QColor ThemeController::gridline() const { return Palette::gridline(); }
QColor ThemeController::axisLine() const { return Palette::axisLine(); }
QColor ThemeController::good() const { return Palette::good(); }
QColor ThemeController::serious() const { return Palette::serious(); }
QColor ThemeController::critical() const { return Palette::critical(); }

void ThemeController::resolve(bool force)
{
    const ThemePreference preference = AppConfig::theme();
    const bool dark = preference == ThemePreference::Dark
        || (preference == ThemePreference::System && systemPrefersDark());
    if (dark == m_dark && !force)
        return;

    m_dark = dark;
    Palette::setMode(dark ? Palette::Mode::Dark : Palette::Mode::Light);
    emit darkChanged();
}

bool ThemeController::systemPrefersDark() const
{
    return qGuiApp
        && qGuiApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}
