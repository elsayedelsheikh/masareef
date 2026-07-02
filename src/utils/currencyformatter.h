#pragma once

#include <QString>

// Money is stored as integer minor units (piasters/cents, 2 decimals).
class CurrencyFormatter {
public:
    // 10050 -> "100.50 ج.م"
    static QString format(qint64 minorUnits);
    // Amount without the currency symbol: 10050 -> "100.50"
    static QString formatPlain(qint64 minorUnits);
    // "100.5" / "100,5" -> 10050. Sets *ok=false on invalid input.
    static qint64 parse(const QString& text, bool* ok = nullptr);
};
