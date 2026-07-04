#pragma once

#include "core/recurrence.h"

#include <QDate>
#include <QObject>
#include <QtQml/qqmlregistration.h>

// Add/edit/delete recurring bills from QML. Amounts arrive as user-typed text
// and are validated here; failures return 0/false with the reason in lastError.
class BillController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(int editCategoryId READ editCategoryId NOTIFY editLoaded)
    Q_PROPERTY(QString editAmountText READ editAmountText NOTIFY editLoaded)
    Q_PROPERTY(QString editName READ editName NOTIFY editLoaded)
    Q_PROPERTY(QDate editNextDue READ editNextDue NOTIFY editLoaded)
    Q_PROPERTY(Recurrence editRecurrence READ editRecurrence NOTIFY editLoaded)
    Q_PROPERTY(QString editNotes READ editNotes NOTIFY editLoaded)

public:
    explicit BillController(QObject* parent = nullptr);

    [[nodiscard]] QString lastError() const { return m_lastError; }
    [[nodiscard]] int editCategoryId() const { return m_editCategoryId; }
    [[nodiscard]] QString editAmountText() const { return m_editAmountText; }
    [[nodiscard]] QString editName() const { return m_editName; }
    [[nodiscard]] QDate editNextDue() const { return m_editNextDue; }
    [[nodiscard]] Recurrence editRecurrence() const { return m_editRecurrence; }
    [[nodiscard]] QString editNotes() const { return m_editNotes; }

    // Add a bill; returns the new bill id, or 0 with lastError set.
    Q_INVOKABLE int add(int categoryId, const QString& amountText, const QString& name,
                        QDate nextDue, Recurrence recurrence, const QString& notes);
    // Update an existing bill.
    Q_INVOKABLE bool update(int id, int categoryId, const QString& amountText,
                            const QString& name, QDate nextDue, Recurrence recurrence,
                            const QString& notes);
    Q_INVOKABLE bool remove(int id);
    // Mark paid: create expense and advance next_due_date per recurrence.
    Q_INVOKABLE bool markPaid(int id, const QString& amountText, QDate paidDate,
                              const QString& description);
    // Enable/disable showing a bill in the list.
    Q_INVOKABLE bool setActive(int id, bool active);
    // Populate edit properties for the edit sheet.
    Q_INVOKABLE bool load(int id);

signals:
    void billAdded(int id);
    void billUpdated(int id);
    void billRemoved(int id);
    void billPaid(int id);
    void lastErrorChanged();
    void editLoaded();

private:
    void setLastError(const QString& message);

    QString m_lastError;
    int m_editCategoryId = -1;
    QString m_editAmountText;
    QString m_editName;
    QDate m_editNextDue;
    Recurrence m_editRecurrence = Recurrence::Monthly;
    QString m_editNotes;
};
