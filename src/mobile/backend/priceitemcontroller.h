#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

// Add/edit/delete price book entries from QML. Prices arrive as user-typed
// text and are validated here (same rules as an expense amount, except that
// a price of zero is allowed — "I have not priced this yet" is a useful
// state for a shopping list). Failures return 0/false with the reason in
// lastError.
class PriceItemController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QString editName READ editName NOTIFY editLoaded)
    Q_PROPERTY(QString editUnit READ editUnit NOTIFY editLoaded)
    Q_PROPERTY(QString editPriceText READ editPriceText NOTIFY editLoaded)
    Q_PROPERTY(int editCategoryId READ editCategoryId NOTIFY editLoaded)

public:
    explicit PriceItemController(QObject* parent = nullptr);

    [[nodiscard]] QString lastError() const { return m_lastError; }
    [[nodiscard]] QString editName() const { return m_editName; }
    [[nodiscard]] QString editUnit() const { return m_editUnit; }
    [[nodiscard]] QString editPriceText() const { return m_editPriceText; }
    [[nodiscard]] int editCategoryId() const { return m_editCategoryId; }

    // categoryId <= 0 means "uncategorized".
    Q_INVOKABLE int add(const QString& name, int categoryId, const QString& unit,
                        const QString& priceText);
    Q_INVOKABLE bool update(int id, const QString& name, int categoryId,
                            const QString& unit, const QString& priceText);
    Q_INVOKABLE bool remove(int id);
    Q_INVOKABLE bool load(int id);

signals:
    void priceItemAdded(int id);
    void priceItemUpdated(int id);
    void priceItemRemoved(int id);
    void lastErrorChanged();
    void editLoaded();

private:
    void setLastError(const QString& message);

    QString m_lastError;
    QString m_editName;
    QString m_editUnit;
    QString m_editPriceText;
    int m_editCategoryId = -1;
};
