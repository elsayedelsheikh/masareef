#include "core/recurrence.h"

#include <QtTest>

class TestRecurrence : public QObject {
    Q_OBJECT
private slots:
    void advance_data();
    void advance();
    void dbString_roundTrip_data();
    void dbString_roundTrip();
    void fromDbString_rejectsUnknown();
};

void TestRecurrence::advance_data()
{
    QTest::addColumn<QDate>("from");
    QTest::addColumn<Recurrence>("recurrence");
    QTest::addColumn<QDate>("expected");
    QTest::newRow("monthly plain")
        << QDate(2026, 1, 15) << Recurrence::Monthly << QDate(2026, 2, 15);
    QTest::newRow("monthly clamps Jan 31 -> Feb 28")
        << QDate(2026, 1, 31) << Recurrence::Monthly << QDate(2026, 2, 28);
    QTest::newRow("quarterly clamps Jan 31 -> Apr 30")
        << QDate(2026, 1, 31) << Recurrence::Quarterly << QDate(2026, 4, 30);
    QTest::newRow("yearly leap day -> Feb 28")
        << QDate(2024, 2, 29) << Recurrence::Yearly << QDate(2025, 2, 28);
}

void TestRecurrence::advance()
{
    QFETCH(QDate, from);
    QFETCH(Recurrence, recurrence);
    QFETCH(QDate, expected);
    QCOMPARE(::advance(from, recurrence), expected);
}

void TestRecurrence::dbString_roundTrip_data()
{
    QTest::addColumn<Recurrence>("recurrence");
    QTest::addColumn<QString>("dbString");
    QTest::newRow("monthly") << Recurrence::Monthly << QStringLiteral("monthly");
    QTest::newRow("quarterly") << Recurrence::Quarterly << QStringLiteral("quarterly");
    QTest::newRow("yearly") << Recurrence::Yearly << QStringLiteral("yearly");
}

void TestRecurrence::dbString_roundTrip()
{
    QFETCH(Recurrence, recurrence);
    QFETCH(QString, dbString);
    QCOMPARE(toDbString(recurrence), dbString);
    QCOMPARE(recurrenceFromDbString(dbString), std::optional(recurrence));
}

void TestRecurrence::fromDbString_rejectsUnknown()
{
    QCOMPARE(recurrenceFromDbString(QStringLiteral("weekly")), std::nullopt);
    QCOMPARE(recurrenceFromDbString(QStringLiteral("MONTHLY")), std::nullopt);
    QCOMPARE(recurrenceFromDbString(QString()), std::nullopt);
}

QTEST_GUILESS_MAIN(TestRecurrence)
#include "tst_recurrence.moc"
