#include "utils/currencyformatter.h"

#include "utils/appconfig.h"

#include <QStringList>

#include <limits>

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
    QString t = text.trimmed();
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
