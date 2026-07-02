#include "utils/currencyformatter.h"

#include "utils/appconfig.h"

#include <QStringList>

QString CurrencyFormatter::formatPlain(qint64 minorUnits)
{
    const bool negative = minorUnits < 0;
    const qint64 absValue = negative ? -minorUnits : minorUnits;
    const qint64 whole = absValue / 100;
    const qint64 frac = absValue % 100;

    QString wholeStr = QString::number(whole);
    for (int i = wholeStr.size() - 3; i > 0; i -= 3)
        wholeStr.insert(i, QLatin1Char(','));

    return QStringLiteral("%1%2.%3")
        .arg(negative ? QStringLiteral("-") : QString(), wholeStr,
             QStringLiteral("%1").arg(frac, 2, 10, QLatin1Char('0')));
}

QString CurrencyFormatter::format(qint64 minorUnits)
{
    return formatPlain(minorUnits) + QLatin1Char(' ') + AppConfig::currencySymbol();
}

qint64 CurrencyFormatter::parse(const QString& text, bool* ok)
{
    if (ok)
        *ok = false;

    QString t = text.trimmed();
    t.replace(QLatin1Char(','), QLatin1Char('.'));
    if (t.isEmpty())
        return 0;

    const QStringList parts = t.split(QLatin1Char('.'));
    if (parts.size() > 2)
        return 0;

    bool numOk = true;
    const qint64 whole = parts.at(0).isEmpty() ? 0 : parts.at(0).toLongLong(&numOk);
    if (!numOk || whole < 0)
        return 0;

    qint64 frac = 0;
    if (parts.size() == 2 && !parts.at(1).isEmpty()) {
        QString fracStr = parts.at(1).left(2);
        while (fracStr.size() < 2)
            fracStr.append(QLatin1Char('0'));
        frac = fracStr.toLongLong(&numOk);
        if (!numOk)
            return 0;
    }

    if (whole > (std::numeric_limits<qint64>::max() - frac) / 100)
        return 0;

    if (ok)
        *ok = true;
    return whole * 100 + frac;
}
