#include "utils/localeformat.h"

#include "utils/appconfig.h"

#include <QCoreApplication>

namespace {

// Arabic locales render numbers with Arabic-Indic digits; fold them back to
// ASCII so dates line up with the amounts next to them.
QString toWesternDigits(QString text)
{
    constexpr char16_t kArabicIndicZero = 0x0660;
    constexpr char16_t kExtendedArabicIndicZero = 0x06F0;

    for (QChar& ch : text) {
        const char16_t code = ch.unicode();
        if (code >= kArabicIndicZero && code <= kArabicIndicZero + 9)
            ch = QChar(char16_t(u'0' + (code - kArabicIndicZero)));
        else if (code >= kExtendedArabicIndicZero
                 && code <= kExtendedArabicIndicZero + 9)
            ch = QChar(char16_t(u'0' + (code - kExtendedArabicIndicZero)));
    }
    return text;
}

QString translate(const char* source)
{
    return QCoreApplication::translate("LocaleFormat", source);
}

QString translate(const char* source, int n)
{
    return QCoreApplication::translate("LocaleFormat", source, nullptr, n);
}

} // namespace

namespace LocaleFormat {

QString uiLocaleName()
{
    const QString preference = AppConfig::language();
    const bool arabic = preference == QLatin1String("ar")
        || (preference == QLatin1String("system")
            && QLocale::system().language() == QLocale::Arabic);
    if (arabic)
        return QStringLiteral("ar_EG");

    // Explicit English gets a stable locale; "system" keeps the OS one so
    // dates match the rest of the device.
    return preference == QLatin1String("system") ? QLocale::system().name()
                                                 : QStringLiteral("en_US");
}

QLocale uiLocale()
{
    return QLocale(uiLocaleName());
}

QString date(QDate date)
{
    if (!date.isValid())
        return {};
    return toWesternDigits(uiLocale().toString(date, QLocale::LongFormat));
}

QString shortDate(QDate date)
{
    if (!date.isValid())
        return {};
    return toWesternDigits(uiLocale().toString(date, QLocale::ShortFormat));
}

QString monthYear(QDate date)
{
    if (!date.isValid())
        return {};
    return toWesternDigits(uiLocale().toString(date, QStringLiteral("MMMM yyyy")));
}

QString monthShort(QDate date)
{
    if (!date.isValid())
        return {};
    return toWesternDigits(uiLocale().toString(date, QStringLiteral("MMM")));
}

QString monthShortYear(QDate date)
{
    if (!date.isValid())
        return {};
    return toWesternDigits(uiLocale().toString(date, QStringLiteral("MMM yy")));
}

QString relativeDate(QDate date)
{
    if (!date.isValid())
        return {};

    switch (QDate::currentDate().daysTo(date)) {
    case 0:
        return translate("Today");
    case -1:
        return translate("Yesterday");
    case 1:
        return translate("Tomorrow");
    default:
        break;
    }
    return LocaleFormat::date(date);
}

QString dueLabel(QDate nextDue)
{
    if (!nextDue.isValid())
        return {};

    const int days = int(QDate::currentDate().daysTo(nextDue));
    if (days == 0)
        return translate("Due today");
    if (days == 1)
        return translate("Due tomorrow");
    if (days > 1)
        return translate("Due in %n day(s)", days);
    return translate("Overdue by %n day(s)", -days);
}

// Expects a non-negative amount: this is what an amount field is prefilled
// with, and CurrencyFormatter::parse — the other half of that round trip —
// rejects a leading "-". The sign is emitted anyway so a negative value
// shows up as an unsavable field rather than as a wrong positive number.
QString amountForEditing(Money amount)
{
    const std::int64_t minorUnits = amount.minorUnits();
    const std::int64_t absUnits = minorUnits < 0 ? -minorUnits : minorUnits;
    return QStringLiteral("%1%2.%3")
        .arg(minorUnits < 0 ? QStringLiteral("-") : QString())
        .arg(absUnits / 100)
        .arg(absUnits % 100, 2, 10, QLatin1Char('0'));
}

} // namespace LocaleFormat
