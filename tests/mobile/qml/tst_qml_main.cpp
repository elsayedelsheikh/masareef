#include "../testutils.h"

#include "storage/expenserepository.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QtQuickTest/quicktest.h>

// QML-invokable fixture: database reset and direct repository access so the
// Quick tests can arrange state without going through the UI under test.
class TestFixture : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;

    Q_INVOKABLE bool resetDatabase() { return TestUtils::resetDatabase(); }

    Q_INVOKABLE int categoryId(const QString& name)
    {
        return TestUtils::categoryId(name);
    }

    Q_INVOKABLE int addExpense(const QString& categoryName, qint64 amountMinor,
                               const QString& description, const QString& isoDate)
    {
        const Result<int> added = ExpenseRepository::add(
            { .categoryId = TestUtils::categoryId(categoryName),
              .amount = Money::fromMinorUnits(amountMinor),
              .description = description,
              .date = QDate::fromString(isoDate, Qt::ISODate) });
        return added ? *added : -1;
    }

    Q_INVOKABLE int expenseCount()
    {
        return TestUtils::countRows(QStringLiteral("expenses"));
    }
};

class Setup : public QObject {
    Q_OBJECT
public slots:
    void applicationAvailable()
    {
        TestUtils::enableTestMode();
        TestUtils::isolateSettings();
        QVERIFY(TestUtils::resetDatabase());
    }

    void qmlEngineAvailable(QQmlEngine* engine)
    {
        engine->rootContext()->setContextProperty(QStringLiteral("TestFixture"),
                                                  new TestFixture(engine));
    }
};

QUICK_TEST_MAIN_WITH_SETUP(masareef_qml, Setup)
#include "tst_qml_main.moc"
