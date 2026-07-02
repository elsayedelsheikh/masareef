#include "utils/appconfig.h"

#include <QSettings>

namespace {
QSettings settings()
{
    return QSettings(QStringLiteral("Masareef"), QStringLiteral("Masareef"));
}
} // namespace

QString AppConfig::currencyCode()
{
    return settings().value(QStringLiteral("currency/code"), QStringLiteral("EGP")).toString();
}

QString AppConfig::currencySymbol()
{
    return settings().value(QStringLiteral("currency/symbol"), QString::fromUtf8("ج.م")).toString();
}

void AppConfig::setCurrency(const QString& code, const QString& symbol)
{
    QSettings s = settings();
    s.setValue(QStringLiteral("currency/code"), code);
    s.setValue(QStringLiteral("currency/symbol"), symbol);
}
