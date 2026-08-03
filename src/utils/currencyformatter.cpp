#include "utils/currencyformatter.h"

#include "utils/appconfig.h"

#include <QStringList>

#include <limits>

namespace {

// Arabic keyboards type Arabic-Indic (U+0660..) or Extended Arabic-Indic
// (U+06F0..) digits and Arabic separators — none of which
// QString::toLongLong understands, so an amount typed on an Arabic keypad
// would otherwise be rejected as garbage. IMEs also wrap numbers in bidi
// control marks; those are formatting, not part of the value.
QString toAsciiNumerals(const QString& text)
{
    constexpr char16_t kArabicIndicZero = 0x0660;
    constexpr char16_t kExtendedArabicIndicZero = 0x06F0;
    constexpr char16_t kArabicDecimalSeparator = 0x066B;
    constexpr char16_t kArabicThousandsSeparator = 0x066C;

    QString out;
    out.reserve(text.size());
    for (const QChar ch : text) {
        const char16_t code = ch.unicode();
        if (code >= kArabicIndicZero && code <= kArabicIndicZero + 9) {
            out.append(QChar(char16_t(u'0' + (code - kArabicIndicZero))));
        } else if (code >= kExtendedArabicIndicZero
                   && code <= kExtendedArabicIndicZero + 9) {
            out.append(QChar(char16_t(u'0' + (code - kExtendedArabicIndicZero))));
        } else if (code == kArabicDecimalSeparator) {
            out.append(QLatin1Char('.'));
        } else if (code == kArabicThousandsSeparator
                   || code == 0x200E    // LEFT-TO-RIGHT MARK
                   || code == 0x200F    // RIGHT-TO-LEFT MARK
                   || code == 0x061C    // ARABIC LETTER MARK
                   || (code >= 0x2066 && code <= 0x2069) // bidi isolates
                   || code == 0x00A0    // NO-BREAK SPACE
                   || code == 0x202F    // NARROW NO-BREAK SPACE
                   || code == 0x2009) { // THIN SPACE
            // Group separators and bidi controls carry no value, so they
            // are dropped rather than translated. In particular ٬ must not
            // become ',': this parser reads ',' as a decimal point (the
            // European convention), which would turn one thousand into
            // one. A plain ASCII space is left alone — trimmed() handles
            // the ends, and one in the middle is a typo, not grouping.
            continue;
        } else {
            out.append(ch);
        }
    }
    return out;
}

} // namespace

namespace CurrencyFormatter {

QString formatPlain(Money amount)
{
    const bool negative = amount.isNegative();
    const std::int64_t absValue =
        negative ? -amount.minorUnits() : amount.minorUnits();
    const std::int64_t whole = absValue / 100;
    const std::int64_t frac = absValue % 100;

    QString wholeStr = QString::number(whole);
    for (int i = wholeStr.size() - 3; i > 0; i -= 3)
        wholeStr.insert(i, QLatin1Char(','));

    return QStringLiteral("%1%2.%3")
        .arg(negative ? QStringLiteral("-") : QString(), wholeStr,
             QStringLiteral("%1").arg(frac, 2, 10, QLatin1Char('0')));
}

QString format(Money amount)
{
    return formatPlain(amount) + QLatin1Char(' ') + AppConfig::currencyCode();
}

std::optional<Money> parse(const QString& text)
{
    QString t = toAsciiNumerals(text).trimmed();
    t.replace(QLatin1Char(','), QLatin1Char('.'));
    if (t.isEmpty())
        return std::nullopt;

    const QStringList parts = t.split(QLatin1Char('.'));
    if (parts.size() > 2)
        return std::nullopt;

    bool numOk = true;
    const qint64 whole = parts.at(0).isEmpty() ? 0 : parts.at(0).toLongLong(&numOk);
    if (!numOk || whole < 0)
        return std::nullopt;

    qint64 frac = 0;
    if (parts.size() == 2 && !parts.at(1).isEmpty()) {
        QString fracStr = parts.at(1).left(2);
        while (fracStr.size() < 2)
            fracStr.append(QLatin1Char('0'));
        frac = fracStr.toLongLong(&numOk);
        if (!numOk)
            return std::nullopt;
    }

    if (whole > (std::numeric_limits<qint64>::max() - frac) / 100)
        return std::nullopt;

    return Money::fromMinorUnits(whole * 100 + frac);
}

} // namespace CurrencyFormatter
