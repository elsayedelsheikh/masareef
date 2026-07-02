#pragma once

#include <QDate>
#include <QList>
#include <QSqlQueryModel>

// Read model for the Expenses table (JOINed with categories) plus the
// static CRUD/aggregation helpers used across the app.
class ExpenseModel : public QSqlQueryModel {
    Q_OBJECT
public:
    enum Column {
        ColId = 0,
        ColDate,
        ColCategory,
        ColDescription,
        ColAmount,
        ColNotes,
        ColumnCount
    };

    struct Expense {
        int id = 0;
        int categoryId = 0;
        qint64 amount = 0; // minor units
        QString description;
        QDate date;
        QString notes;
    };

    struct CategoryTotal {
        int categoryId = 0;
        QString name;
        QString color;
        qint64 total = 0;
    };

    struct MonthTotal {
        QDate month; // first day of the month
        qint64 total = 0;
    };

    explicit ExpenseModel(QObject* parent = nullptr);

    void setDateRange(const QDate& from, const QDate& to);
    void setCategoryFilter(int categoryId); // -1 = all categories
    void refresh();
    qint64 filteredTotal() const { return m_total; }
    int expenseIdAt(int row) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    static bool addExpense(int categoryId, qint64 amount, const QString& description,
                           const QDate& date, const QString& notes,
                           const QVariant& recurringBillId = QVariant(),
                           QString* error = nullptr);
    static bool updateExpense(int id, int categoryId, qint64 amount,
                              const QString& description, const QDate& date,
                              const QString& notes, QString* error = nullptr);
    static bool deleteExpense(int id, QString* error = nullptr);
    static bool fetchExpense(int id, Expense* out, QString* error = nullptr);

    static qint64 totalBetween(const QDate& from, const QDate& to, int categoryId = -1);
    // Per-category sums in the range, sorted by total descending
    static QList<CategoryTotal> totalsByCategory(const QDate& from, const QDate& to);
    // Last `months` calendar months including the current one, zero-filled
    static QList<MonthTotal> monthlyTotals(int months);

private:
    QDate m_from;
    QDate m_to;
    int m_categoryId = -1;
    qint64 m_total = 0;
};
