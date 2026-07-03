#include "testutils.h"

#include "backend/themecontroller.h"
#include "utils/appconfig.h"
#include "utils/palette.h"

#include <QSignalSpy>
#include <QtTest>

class TestThemeController : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void preference_defaultsToSystem();
    void preference_restoredFromConfig();
    void setPreference_persistsAndSignals();
    void dark_syncsPaletteMode();
    void colors_followPalette();
};

void TestThemeController::initTestCase()
{
    TestUtils::enableTestMode();
    TestUtils::isolateSettings();
}

void TestThemeController::init()
{
    QSettings(QStringLiteral("Masareef"), QStringLiteral("Masareef")).clear();
}

void TestThemeController::preference_defaultsToSystem()
{
    ThemeController theme;
    QCOMPARE(theme.preference(), QStringLiteral("system"));
}

void TestThemeController::preference_restoredFromConfig()
{
    AppConfig::setTheme(ThemePreference::Dark);
    ThemeController theme;
    QCOMPARE(theme.preference(), QStringLiteral("dark"));
    QCOMPARE(theme.dark(), true);
    QCOMPARE(Palette::mode(), Palette::Mode::Dark);
}

void TestThemeController::setPreference_persistsAndSignals()
{
    ThemeController theme;
    QSignalSpy preferenceSpy(&theme, &ThemeController::preferenceChanged);

    theme.setPreference(QStringLiteral("dark"));
    QCOMPARE(theme.preference(), QStringLiteral("dark"));
    QCOMPARE(AppConfig::theme(), ThemePreference::Dark);
    QCOMPARE(preferenceSpy.count(), 1);

    theme.setPreference(QStringLiteral("light"));
    QCOMPARE(AppConfig::theme(), ThemePreference::Light);
    QCOMPARE(preferenceSpy.count(), 2);
}

void TestThemeController::dark_syncsPaletteMode()
{
    ThemeController theme;
    QSignalSpy darkSpy(&theme, &ThemeController::darkChanged);

    theme.setPreference(QStringLiteral("dark"));
    QCOMPARE(theme.dark(), true);
    QCOMPARE(Palette::mode(), Palette::Mode::Dark);
    QVERIFY(darkSpy.count() >= 1);

    theme.setPreference(QStringLiteral("light"));
    QCOMPARE(theme.dark(), false);
    QCOMPARE(Palette::mode(), Palette::Mode::Light);
}

void TestThemeController::colors_followPalette()
{
    ThemeController theme;

    theme.setPreference(QStringLiteral("light"));
    QCOMPARE(theme.surface(), Palette::surface());
    QCOMPARE(theme.primaryInk(), QColor(QStringLiteral("#0b0b0b")));

    theme.setPreference(QStringLiteral("dark"));
    QCOMPARE(theme.surface(), QColor(QStringLiteral("#1a1a19")));
    QCOMPARE(theme.primaryInk(), QColor(QStringLiteral("#ffffff")));
    QCOMPARE(theme.good(), Palette::good());
    QCOMPARE(theme.serious(), Palette::serious());
    QCOMPARE(theme.critical(), Palette::critical());
    QCOMPARE(theme.mutedInk(), Palette::mutedInk());
    QCOMPARE(theme.secondaryInk(), Palette::secondaryInk());
    QCOMPARE(theme.gridline(), Palette::gridline());
}

QTEST_MAIN(TestThemeController)
#include "tst_themecontroller.moc"
