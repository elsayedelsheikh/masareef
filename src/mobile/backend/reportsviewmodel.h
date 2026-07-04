#pragma once

#include <QDate>
#include <QObject>
#include <QtQml/qqmlregistration.h>

// Monthly and category breakdowns of expenses, exposed to QML as lists of
// objects so charts can bind to them. No filtering: Reports show all data.

// ponytail: these are plain data structs returned by the model; no Q_OBJECT needed,
// but QML accesses their properties as JS objects via the list.
struct MonthTotal {
    QDate month;
    QString monthName;
    QString totalFormatted;
    qint64 totalMinor = 0;
};

struct CategoryTotal {
    int categoryId = 0;
    QString categoryName;
    QString categoryColor;
    QString totalFormatted;
    qint64 totalMinor = 0;
};

Q_DECLARE_METATYPE(MonthTotal)
Q_DECLARE_METATYPE(CategoryTotal)

class ReportsViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ReportsViewModel(QObject* parent = nullptr);

    // QML-callable accessors returning lists suitable for Repeater binding
    Q_INVOKABLE QList<MonthTotal> monthlyTotals() const;
    Q_INVOKABLE QList<CategoryTotal> categoryTotals(QDate from, QDate to) const;

private:
};
