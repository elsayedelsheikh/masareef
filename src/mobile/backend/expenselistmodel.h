#pragma once

#include "storage/expenserepository.h"

#include <QAbstractListModel>
#include <QDate>
#include <QHash>
#include <QtQml/qqmlregistration.h>

// QML-facing list of expenses. Wraps ExpenseRepository with role-based
// access (the desktop ExpenseModel is column-based and has no roleNames).
// The filter is exposed as individual properties because ExpenseFilter is
// not a QML type; every filter change re-queries so rows, count and total
// always agree with the repository.
class ExpenseListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QDate fromDate READ fromDate WRITE setFromDate NOTIFY filterChanged)
    Q_PROPERTY(QDate toDate READ toDate WRITE setToDate NOTIFY filterChanged)
    Q_PROPERTY(int categoryId READ categoryId WRITE setCategoryId NOTIFY filterChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY filterChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString totalFormatted READ totalFormatted NOTIFY totalChanged)

public:
    enum Role {
        ExpenseIdRole = Qt::UserRole + 1,
        DateRole,
        CategoryIdRole,
        CategoryNameRole,
        CategoryColorRole,
        DescriptionRole,
        AmountMinorRole,
        AmountFormattedRole,
        NotesRole,
        DateSectionRole, // ISO date string, for ListView section grouping
    };
    Q_ENUM(Role)

    explicit ExpenseListModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] QDate fromDate() const { return m_filter.from; }
    void setFromDate(QDate date);
    [[nodiscard]] QDate toDate() const { return m_filter.to; }
    void setToDate(QDate date);
    [[nodiscard]] int categoryId() const { return m_filter.categoryId; }
    void setCategoryId(int categoryId);
    [[nodiscard]] QString searchText() const { return m_filter.searchText; }
    void setSearchText(const QString& text);

    [[nodiscard]] int count() const { return int(m_rows.size()); }
    [[nodiscard]] QString totalFormatted() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool removeAt(int row);
    Q_INVOKABLE int expenseIdAt(int row) const;
    // All roles of one row as a map (QML has no model.data by role name);
    // used to stash a row for undo before deleting it.
    Q_INVOKABLE QVariantMap get(int row) const;

signals:
    void filterChanged();
    void countChanged();
    void totalChanged();

private:
    struct Row {
        Expense expense;
        QString categoryName;
        QString categoryColor; // stored (light) hex; resolved via Palette at read time
    };

    void reload();
    void updateTotal();

    ExpenseFilter m_filter;
    QList<Row> m_rows;
    Money m_total;
};
