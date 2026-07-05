#pragma once

#include <QColor>
#include <QDate>
#include <QObject>
#include <QtQml/qqmlregistration.h>

// Monthly and category breakdowns of expenses, exposed to QML as lists of
// objects so charts can bind to them. No filtering: Reports show all data.

// Q_GADGET so QML can read the members off modelData in the Repeater bindings;
// public fields stay directly accessible from C++ (tests read .month etc).
struct MonthTotal {
    Q_GADGET
    Q_PROPERTY(QString monthName MEMBER monthName)
    Q_PROPERTY(QString monthShort MEMBER monthShort)
    Q_PROPERTY(QString totalFormatted MEMBER totalFormatted)
    Q_PROPERTY(qint64 totalMinor MEMBER totalMinor)
public:
    QDate month;
    QString monthName;  // "MMM yy" — tooltip
    QString monthShort; // "MMM"    — axis label
    QString totalFormatted;
    qint64 totalMinor = 0;
};

struct CategoryTotal {
    Q_GADGET
    Q_PROPERTY(QString categoryName MEMBER categoryName)
    Q_PROPERTY(QColor categoryColor MEMBER categoryColor)
    Q_PROPERTY(QString totalFormatted MEMBER totalFormatted)
    Q_PROPERTY(qint64 totalMinor MEMBER totalMinor)
public:
    int categoryId = 0;
    QString categoryName;
    QColor categoryColor; // resolved via Palette::series at build time
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
