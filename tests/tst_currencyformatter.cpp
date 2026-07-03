#include "core/money.h"
#include "utils/appconfig.h"
#include "utils/currencyformatter.h"

#include <QtTest>

class TestCurrencyFormatter : public QObject {
    Q_OBJECT
private slots:
    void formatPlain_data();
    void formatPlain();
    void format_appendsCurrencyCode();
    void parse_data();
    void parse();
};

void TestCurrencyFormatter::formatPlain_data()
{
    QTest::addColumn<qint64>("minor");
    QTest::addColumn<QString>("expected");
    QTest::newRow("zero") << qint64(0) << QStringLiteral("0.00");
    QTest::newRow("simple") << qint64(10050) << QStringLiteral("100.50");
    QTest::newRow("sub-unit") << qint64(5) << QStringLiteral("0.05");
    QTest::newRow("thousands") << qint64(123456789) << QStringLiteral("1,234,567.89");
    QTest::newRow("negative") << qint64(-10050) << QStringLiteral("-100.50");
}

void TestCurrencyFormatter::formatPlain()
{
    QFETCH(qint64, minor);
    QFETCH(QString, expected);
    QCOMPARE(CurrencyFormatter::formatPlain(Money::fromMinorUnits(minor)), expected);
}

void TestCurrencyFormatter::format_appendsCurrencyCode()
{
    QCOMPARE(CurrencyFormatter::format(Money::fromMinorUnits(10050)),
             QStringLiteral("100.50 ") + AppConfig::currencyCode());
}

void TestCurrencyFormatter::parse_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<bool>("expectOk");
    QTest::addColumn<qint64>("expected");
    QTest::newRow("integer") << QStringLiteral("100") << true << qint64(10000);
    QTest::newRow("one decimal, dot") << QStringLiteral("100.5") << true << qint64(10050);
    QTest::newRow("two decimals") << QStringLiteral("100.55") << true << qint64(10055);
    QTest::newRow("comma decimal") << QStringLiteral("100,5") << true << qint64(10050);
    QTest::newRow("surrounding spaces") << QStringLiteral(" 42 ") << true << qint64(4200);
    QTest::newRow("bare fraction") << QStringLiteral(".5") << true << qint64(50);
    QTest::newRow("trailing dot") << QStringLiteral("100.") << true << qint64(10000);
    QTest::newRow("empty") << QString() << false << qint64(0);
    QTest::newRow("letters") << QStringLiteral("abc") << false << qint64(0);
    QTest::newRow("two dots") << QStringLiteral("1.2.3") << false << qint64(0);
    QTest::newRow("negative") << QStringLiteral("-5") << false << qint64(0);
}

void TestCurrencyFormatter::parse()
{
    QFETCH(QString, text);
    QFETCH(bool, expectOk);
    QFETCH(qint64, expected);
    const std::optional<Money> value = CurrencyFormatter::parse(text);
    QCOMPARE(value.has_value(), expectOk);
    if (expectOk)
        QCOMPARE(value->minorUnits(), expected);
}

QTEST_GUILESS_MAIN(TestCurrencyFormatter)
#include "tst_currencyformatter.moc"
