#include "utils/palette.h"

namespace {

Palette::Mode g_mode = Palette::Mode::Light;

// Dark-mode steps of the categorical slots, validated against the dark
// surface (#1a1a19) as a set — same hues, re-stepped, same order.
constexpr const char* kCategoricalDark[] = {
    "#3987e5", // blue
    "#199e70", // aqua
    "#c98500", // yellow
    "#008300", // green
    "#9085e9", // violet
    "#e66767", // red
    "#d55181", // magenta
    "#d95926", // orange
};

bool sameHexIgnoringCase(const char* a, const QString& b)
{
    return QString::fromLatin1(a).compare(b, Qt::CaseInsensitive) == 0;
}

QColor pick(const char* light, const char* dark)
{
    return QColor(QLatin1String(g_mode == Palette::Mode::Dark ? dark : light));
}

} // namespace

namespace Palette {

void setMode(Mode mode)
{
    g_mode = mode;
}

Mode mode()
{
    return g_mode;
}

QColor series(const QString& storedColor)
{
    if (g_mode == Mode::Dark) {
        for (int i = 0; i < kCategoricalCount; ++i) {
            if (sameHexIgnoringCase(kCategorical[i], storedColor))
                return QColor(QLatin1String(kCategoricalDark[i]));
        }
    }
    return QColor(storedColor);
}

// Status steps are fixed across modes (all clear 3:1 on the dark surface
// and stay distinct from the categorical slots).
QColor critical() { return QColor(QLatin1String("#d03b3b")); }
QColor serious()  { return QColor(QLatin1String("#ec835a")); }
QColor good()     { return QColor(QLatin1String("#0ca30c")); }

QColor surface()      { return pick("#fcfcfb", "#1a1a19"); }
QColor gridline()     { return pick("#e1e0d9", "#2c2c2a"); }
QColor axisLine()     { return pick("#c3c2b7", "#383835"); }
QColor mutedInk()     { return QColor(QLatin1String("#898781")); }
QColor primaryInk()   { return pick("#0b0b0b", "#ffffff"); }
QColor secondaryInk() { return pick("#52514e", "#c3c2b7"); }

} // namespace Palette
