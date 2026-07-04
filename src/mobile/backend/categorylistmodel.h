#pragma once

#include "storage/categoryrepository.h"

#include <QAbstractListModel>
#include <QtQml/qqmlregistration.h>

// QML-facing category list with the CRUD operations the category manager
// screen needs. Mutations return bool and leave the reason in lastError;
// the business rules themselves (unique names, undeletable in-use/last
// remaining categories) live in CategoryRepository.
class CategoryListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        CategoryIdRole = Qt::UserRole + 1,
        NameRole,
        ColorRole,
        InUseRole,
    };
    Q_ENUM(Role)

    explicit CategoryListModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] QString lastError() const { return m_lastError; }
    [[nodiscard]] int count() const { return int(m_rows.size()); }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool add(const QString& name, const QString& color);
    Q_INVOKABLE bool rename(int id, const QString& newName);
    Q_INVOKABLE bool setColor(int id, const QString& color);
    Q_INVOKABLE bool remove(int id);
    Q_INVOKABLE bool removeAndReassign(int id, int targetId);
    Q_INVOKABLE QString suggestedColor() const;
    // The categorical palette slots (stored hex values) for the color picker
    Q_INVOKABLE QStringList paletteColors() const;

signals:
    void lastErrorChanged();
    void countChanged();

private:
    struct Row {
        Category category;
        bool inUse = false;
    };

    void reload();
    bool applyResult(const Result<void>& result);

    QList<Row> m_rows;
    QString m_lastError;
};
