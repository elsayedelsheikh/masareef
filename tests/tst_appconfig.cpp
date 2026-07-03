#include "testutils.h"

#include "utils/appconfig.h"

#include <QSettings>
#include <QtTest>

class TestAppConfig : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void currencyCode_defaultsToEgp();
    void currencyCode_roundTrip();
    void theme_defaultsToSystem();
    void theme_roundTrip();
    void language_defaultsToSystem();
    void language_roundTrip();
    void language_unknownStoredValueReadsAsSystem();
    void language_storedKeySurvivesIniRoundTrip();
};

void TestAppConfig::initTestCase()
{
    TestUtils::enableTestMode();
    TestUtils::isolateSettings();
}

void TestAppConfig::init()
{
    QSettings(QStringLiteral("Masareef"), QStringLiteral("Masareef")).clear();
}

void TestAppConfig::currencyCode_defaultsToEgp()
{
    QCOMPARE(AppConfig::currencyCode(), QStringLiteral("EGP"));
}

void TestAppConfig::currencyCode_roundTrip()
{
    AppConfig::setCurrencyCode(QStringLiteral("USD"));
    QCOMPARE(AppConfig::currencyCode(), QStringLiteral("USD"));
}

void TestAppConfig::theme_defaultsToSystem()
{
    QCOMPARE(AppConfig::theme(), ThemePreference::System);
}

void TestAppConfig::theme_roundTrip()
{
    AppConfig::setTheme(ThemePreference::Dark);
    QCOMPARE(AppConfig::theme(), ThemePreference::Dark);
    AppConfig::setTheme(ThemePreference::Light);
    QCOMPARE(AppConfig::theme(), ThemePreference::Light);
    AppConfig::setTheme(ThemePreference::System);
    QCOMPARE(AppConfig::theme(), ThemePreference::System);
}

void TestAppConfig::language_defaultsToSystem()
{
    QCOMPARE(AppConfig::language(), QStringLiteral("system"));
}

void TestAppConfig::language_roundTrip()
{
    AppConfig::setLanguage(QStringLiteral("ar"));
    QCOMPARE(AppConfig::language(), QStringLiteral("ar"));
    AppConfig::setLanguage(QStringLiteral("en"));
    QCOMPARE(AppConfig::language(), QStringLiteral("en"));
    AppConfig::setLanguage(QStringLiteral("system"));
    QCOMPARE(AppConfig::language(), QStringLiteral("system"));
}

void TestAppConfig::language_unknownStoredValueReadsAsSystem()
{
    QSettings(QStringLiteral("Masareef"), QStringLiteral("Masareef"))
        .setValue(QStringLiteral("locale/language"), QStringLiteral("fr"));
    QCOMPARE(AppConfig::language(), QStringLiteral("system"));
}

void TestAppConfig::language_storedKeySurvivesIniRoundTrip()
{
    // The INI backend rewrites a group named "general" (any case) to
    // "%General", so such a key silently stops matching after the file is
    // re-parsed — the value is only readable again in a fresh process. Assert
    // that the key we store is spelled identically after a disk round trip.
    AppConfig::setLanguage(QStringLiteral("ar"));

    QSettings settings(QStringLiteral("Masareef"), QStringLiteral("Masareef"));
    settings.sync(); // flush and re-parse the INI file
    QVERIFY2(settings.allKeys().contains(QStringLiteral("locale/language")),
             qPrintable(QStringLiteral("stored keys: ")
                        + settings.allKeys().join(QLatin1String(", "))));
}

QTEST_GUILESS_MAIN(TestAppConfig)
#include "tst_appconfig.moc"
