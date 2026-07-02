#pragma once

#include <QString>

// QSettings-backed application configuration.
class AppConfig {
public:
    static QString currencyCode();   // default "EGP"
    static QString currencySymbol(); // default "ج.م"
    static void setCurrency(const QString& code, const QString& symbol);
};
