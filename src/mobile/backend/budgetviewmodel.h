#pragma once

#include "core/money.h"

#include <QAbstractListModel>
#include <QDate>
#include <QtQml/qqmlregistration.h>

#include <optional>

// The Budgets screen: one row per category (with optional budget, current
// month spend and progress) plus the optional overall budget as properties.
// Budget amounts arrive as user-typed text; empty text clears the budget,
// unparseable text fails with the reason in lastError.
class BudgetViewModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool hasOverallBudget READ hasOverallBudget NOTIFY overallChanged)
    Q_PROPERTY(qint64 overallBudgetMinor READ overallBudgetMinor NOTIFY overallChanged)
    Q_PROPERTY(QString overallBudgetText READ overallBudgetText NOTIFY overallChanged)
    Q_PROPERTY(qint64 overallSpentMinor READ overallSpentMinor NOTIFY overallChanged)
    Q_PROPERTY(double overallProgress READ overallProgress NOTIFY overallChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    enum Role {
        CategoryIdRole = Qt::UserRole + 1,
        NameRole,
        ColorRole,
        BudgetMinorRole,
        HasBudgetRole,
        SpentMinorRole,
        ProgressRole,
        BudgetTextRole,
    };
    Q_ENUM(Role)

    explicit BudgetViewModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] bool hasOverallBudget() const { return m_overallBudget.has_value(); }
    [[nodiscard]] qint64 overallBudgetMinor() const;
    [[nodiscard]] QString overallBudgetText() const;
    [[nodiscard]] qint64 overallSpentMinor() const { return m_overallSpent.minorUnits(); }
    [[nodiscard]] double overallProgress() const;
    [[nodiscard]] QString lastError() const { return m_lastError; }

    Q_INVOKABLE bool setOverallBudget(const QString& amountText);
    Q_INVOKABLE bool setCategoryBudget(int categoryId, const QString& amountText);

public slots:
    void refresh();

signals:
    void overallChanged();
    void lastErrorChanged();

private:
    struct Row {
        int categoryId = 0;
        QString name;
        QString color; // stored (light) hex; resolved via Palette at read time
        std::optional<Money> budget;
        Money spent;
    };

    // Empty/whitespace text -> cleared budget (nullopt of nullopt);
    // unparseable text -> nullopt with lastError set.
    [[nodiscard]] std::optional<std::optional<Money>> parseBudgetText(const QString& text);
    void setLastError(const QString& message);
    void reload();

    QList<Row> m_rows;
    std::optional<Money> m_overallBudget;
    Money m_overallSpent;
    QString m_lastError;
};
