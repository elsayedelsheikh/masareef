#include "utils/appconfig.h"
#include "utils/currencyformatter.h"
#include "utils/localeformat.h"

#include <QSettings>
#include <QStandardPaths>
#include <QtTest>

// LocaleFormat is what keeps dates in the language the UI is actually
// running in — the process locale is deliberately not consulted — and what
// keeps every number on screen in Western digits, so a row never mixes
// "١٥ يناير" with "150.00 EGP".
class TestLocaleFormat : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanup();

    void uiLocaleName_followsTheLanguagePreference();
    void date_localizesTheMonthName();
    void date_keepsWesternDigitsInArabic();
    void shortDate_keepsWesternDigitsInArabic();
    void monthYear_keepsWesternDigitsInArabic();
    void monthShort_keepsWesternDigitsInArabic();
    void invalidDate_formatsAsEmpty();

    void relativeDate_namesTheNearbyDays();
    void relativeDate_fallsBackToTheFullDate();

    void dueLabel_countsForwardsAndBackwards();

    void amountForEditing_roundTripsThroughTheParser_data();
    void amountForEditing_roundTripsThroughTheParser();
    void amountForEditing_hasNoThousandsSeparator();

private:
    // True when `text` holds no Arabic-Indic digit in either block.
    static bool isWesternDigitsOnly(const QString& text);
    // True when `text` holds at least one Arabic letter (not a digit).
    static bool hasArabicLetter(const QString& text);
};

void TestLocaleFormat::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QSettings::setPath(
        QSettings::NativeFormat, QSettings::UserScope,
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));
}

void TestLocaleFormat::cleanup()
{
    AppConfig::setLanguage(QStringLiteral("system"));
}

bool TestLocaleFormat::isWesternDigitsOnly(const QString& text)
{
    for (const QChar ch : text) {
        const char16_t code = ch.unicode();
        if ((code >= 0x0660 && code <= 0x0669) || (code >= 0x06F0 && code <= 0x06F9))
            return false;
    }
    return true;
}

bool TestLocaleFormat::hasArabicLetter(const QString& text)
{
    for (const QChar ch : text) {
        const char16_t code = ch.unicode();
        const bool arabicBlock = (code >= 0x0620 && code <= 0x064A)
            || (code >= 0x0671 && code <= 0x06D3);
        if (arabicBlock)
            return true;
    }
    return false;
}

void TestLocaleFormat::uiLocaleName_followsTheLanguagePreference()
{
    AppConfig::setLanguage(QStringLiteral("ar"));
    QCOMPARE(LocaleFormat::uiLocaleName(), QStringLiteral("ar_EG"));

    // Explicit English pins the locale rather than following the device, so
    // an English UI reads the same on every phone.
    AppConfig::setLanguage(QStringLiteral("en"));
    QCOMPARE(LocaleFormat::uiLocaleName(), QStringLiteral("en_US"));
}

void TestLocaleFormat::date_localizesTheMonthName()
{
    const QDate date(2026, 1, 15);

    AppConfig::setLanguage(QStringLiteral("en"));
    const QString english = LocaleFormat::date(date);
    QVERIFY(english.contains(QStringLiteral("January")));

    AppConfig::setLanguage(QStringLiteral("ar"));
    const QString arabic = LocaleFormat::date(date);
    QVERIFY(arabic != english);
    // Arabic script, not a transliteration of the English name
    QVERIFY(hasArabicLetter(arabic));
}

void TestLocaleFormat::date_keepsWesternDigitsInArabic()
{
    AppConfig::setLanguage(QStringLiteral("ar"));
    const QString formatted = LocaleFormat::date(QDate(2026, 1, 15));
    QVERIFY(!formatted.isEmpty());
    QVERIFY(isWesternDigitsOnly(formatted));
    QVERIFY(formatted.contains(QStringLiteral("15")));
    QVERIFY(formatted.contains(QStringLiteral("2026")));
}

void TestLocaleFormat::shortDate_keepsWesternDigitsInArabic()
{
    AppConfig::setLanguage(QStringLiteral("ar"));
    const QString formatted = LocaleFormat::shortDate(QDate(2026, 1, 15));
    QVERIFY(!formatted.isEmpty());
    QVERIFY(isWesternDigitsOnly(formatted));
}

void TestLocaleFormat::monthYear_keepsWesternDigitsInArabic()
{
    AppConfig::setLanguage(QStringLiteral("ar"));
    const QString formatted = LocaleFormat::monthYear(QDate(2026, 1, 15));
    QVERIFY(!formatted.isEmpty());
    QVERIFY(isWesternDigitsOnly(formatted));
    QVERIFY(formatted.contains(QStringLiteral("2026")));
}

void TestLocaleFormat::monthShort_keepsWesternDigitsInArabic()
{
    AppConfig::setLanguage(QStringLiteral("ar"));
    QVERIFY(isWesternDigitsOnly(LocaleFormat::monthShort(QDate(2026, 1, 15))));
    const QString withYear = LocaleFormat::monthShortYear(QDate(2026, 1, 15));
    QVERIFY(isWesternDigitsOnly(withYear));
    QVERIFY(withYear.contains(QStringLiteral("26")));
}

void TestLocaleFormat::invalidDate_formatsAsEmpty()
{
    const QDate invalid;
    QVERIFY(LocaleFormat::date(invalid).isEmpty());
    QVERIFY(LocaleFormat::shortDate(invalid).isEmpty());
    QVERIFY(LocaleFormat::monthYear(invalid).isEmpty());
    QVERIFY(LocaleFormat::monthShort(invalid).isEmpty());
    QVERIFY(LocaleFormat::monthShortYear(invalid).isEmpty());
    QVERIFY(LocaleFormat::relativeDate(invalid).isEmpty());
    QVERIFY(LocaleFormat::dueLabel(invalid).isEmpty());
}

void TestLocaleFormat::relativeDate_namesTheNearbyDays()
{
    AppConfig::setLanguage(QStringLiteral("en"));
    const QDate today = QDate::currentDate();
    QCOMPARE(LocaleFormat::relativeDate(today), QStringLiteral("Today"));
    QCOMPARE(LocaleFormat::relativeDate(today.addDays(-1)), QStringLiteral("Yesterday"));
    QCOMPARE(LocaleFormat::relativeDate(today.addDays(1)), QStringLiteral("Tomorrow"));
}

void TestLocaleFormat::relativeDate_fallsBackToTheFullDate()
{
    AppConfig::setLanguage(QStringLiteral("en"));
    const QDate far = QDate::currentDate().addDays(9);
    QCOMPARE(LocaleFormat::relativeDate(far), LocaleFormat::date(far));
}

void TestLocaleFormat::dueLabel_countsForwardsAndBackwards()
{
    AppConfig::setLanguage(QStringLiteral("en"));
    const QDate today = QDate::currentDate();

    QCOMPARE(LocaleFormat::dueLabel(today), QStringLiteral("Due today"));
    QCOMPARE(LocaleFormat::dueLabel(today.addDays(1)), QStringLiteral("Due tomorrow"));

    const QString soon = LocaleFormat::dueLabel(today.addDays(5));
    QVERIFY(soon.contains(QStringLiteral("5")));
    QVERIFY(!soon.contains(QStringLiteral("-")));

    const QString late = LocaleFormat::dueLabel(today.addDays(-3));
    QVERIFY(late.contains(QStringLiteral("3")));
    // The count is stated as "overdue by 3", never as minus three days.
    QVERIFY(!late.contains(QStringLiteral("-3")));
    QVERIFY(late != soon);
}

void TestLocaleFormat::amountForEditing_roundTripsThroughTheParser_data()
{
    QTest::addColumn<qint64>("minor");
    QTest::addColumn<QString>("expected");
    QTest::newRow("zero") << qint64(0) << QStringLiteral("0.00");
    QTest::newRow("sub-unit") << qint64(5) << QStringLiteral("0.05");
    QTest::newRow("simple") << qint64(10050) << QStringLiteral("100.50");
    QTest::newRow("thousands") << qint64(123456789) << QStringLiteral("1234567.89");
    QTest::newRow("negative") << qint64(-10050) << QStringLiteral("-100.50");
    // The case a naive "%1.%2" of minorUnits/100 and minorUnits%100 gets
    // wrong: -50/100 is 0 and -50%100 is -50, giving "0.-50".
    QTest::newRow("negative sub-unit") << qint64(-50) << QStringLiteral("-0.50");
}

void TestLocaleFormat::amountForEditing_roundTripsThroughTheParser()
{
    QFETCH(qint64, minor);
    QFETCH(QString, expected);
    QCOMPARE(LocaleFormat::amountForEditing(Money::fromMinorUnits(minor)), expected);
}

void TestLocaleFormat::amountForEditing_hasNoThousandsSeparator()
{
    // This is the whole point of the function: formatPlain's separators are
    // rejected by the parser, so an edit field filled from formatPlain
    // could not be saved again.
    const QString text = LocaleFormat::amountForEditing(Money::fromMinorUnits(123456789));
    QVERIFY(!text.contains(QLatin1Char(',')));
    QCOMPARE(CurrencyFormatter::parse(text).value_or(Money()).minorUnits(), 123456789);
}

QTEST_GUILESS_MAIN(TestLocaleFormat)
#include "tst_localeformat.moc"
