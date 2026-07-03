#include "utils/theme.h"

#include "utils/appconfig.h"
#include "utils/palette.h"

#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>

namespace {

bool systemPrefersDark()
{
    return QApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

QPalette darkPalette()
{
    // Built from the same chrome steps the charts use, so widget chrome
    // and chart chrome sit on one surface system.
    QPalette palette;
    const QColor window(0x23, 0x23, 0x22);
    const QColor base(0x1a, 0x1a, 0x19);
    const QColor button(0x2c, 0x2c, 0x2a);
    const QColor text(Qt::white);
    const QColor muted(0x89, 0x87, 0x81);
    const QColor highlight(0x39, 0x87, 0xe5); // dark blue categorical step

    palette.setColor(QPalette::Window, window);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, base);
    palette.setColor(QPalette::AlternateBase, window);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::PlaceholderText, muted);
    palette.setColor(QPalette::Button, button);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::ToolTipBase, button);
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::Highlight, highlight);
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Link, highlight);

    for (const QPalette::ColorRole role :
         { QPalette::WindowText, QPalette::Text, QPalette::ButtonText })
        palette.setColor(QPalette::Disabled, role, muted);
    return palette;
}

} // namespace

namespace Theme {

void apply()
{
    const ThemePreference pref = AppConfig::theme();
    const bool dark =
        pref == ThemePreference::Dark
        || (pref == ThemePreference::System && systemPrefersDark());

    Palette::setMode(dark ? Palette::Mode::Dark : Palette::Mode::Light);

    // Fusion renders custom palettes consistently across platforms; the
    // platform default style often ignores QPalette in dark mode.
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QApplication::setPalette(dark ? darkPalette()
                                  : QApplication::style()->standardPalette());
}

bool isDark()
{
    return Palette::mode() == Palette::Mode::Dark;
}

} // namespace Theme
