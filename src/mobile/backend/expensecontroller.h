#pragma once

#include <QDate>
#include <QObject>
#include <QtQml/qqmlregistration.h>

// Add/edit/delete expenses from QML. Amounts arrive as user-typed text and
// are validated here (same rules as the desktop dialog: parseable, positive,
// category chosen); failures return 0/false with the reason in lastError.
// The expenseAdded/Updated/Removed signals drive the live refresh of every
// model showing expense-derived data.
class ExpenseController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(int editCategoryId READ editCategoryId NOTIFY editLoaded)
    Q_PROPERTY(QString editAmountText READ editAmountText NOTIFY editLoaded)
    Q_PROPERTY(QString editDescription READ editDescription NOTIFY editLoaded)
    Q_PROPERTY(QDate editDate READ editDate NOTIFY editLoaded)
    Q_PROPERTY(QString editNotes READ editNotes NOTIFY editLoaded)

public:
    explicit ExpenseController(QObject* parent = nullptr);

    [[nodiscard]] QString lastError() const { return m_lastError; }
    [[nodiscard]] int editCategoryId() const { return m_editCategoryId; }
    [[nodiscard]] QString editAmountText() const { return m_editAmountText; }
    [[nodiscard]] QString editDescription() const { return m_editDescription; }
    [[nodiscard]] QDate editDate() const { return m_editDate; }
    [[nodiscard]] QString editNotes() const { return m_editNotes; }

    // Returns the new expense id, or 0 with lastError set.
    Q_INVOKABLE int add(int categoryId, const QString& amountText,
                        const QString& description, QDate date, const QString& notes);
    Q_INVOKABLE bool update(int id, int categoryId, const QString& amountText,
                            const QString& description, QDate date,
                            const QString& notes);
    Q_INVOKABLE bool remove(int id);
    // Multi-select delete: emits expenseRemoved for each id on success.
    Q_INVOKABLE bool removeMany(const QList<int>& ids);
    // Undo of a delete: re-adds from model role data (minor units, no text
    // parsing). Returns the new id, or 0 with lastError set.
    Q_INVOKABLE int restore(int categoryId, qint64 amountMinor,
                            const QString& description, QDate date,
                            const QString& notes);
    // Populates the edit* properties for the edit sheet.
    Q_INVOKABLE bool load(int id);

signals:
    void expenseAdded(int id);
    void expenseUpdated(int id);
    void expenseRemoved(int id);
    void lastErrorChanged();
    void editLoaded();

private:
    void setLastError(const QString& message);

    QString m_lastError;
    int m_editCategoryId = -1;
    QString m_editAmountText;
    QString m_editDescription;
    QDate m_editDate;
    QString m_editNotes;
};
