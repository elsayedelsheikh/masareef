#pragma once

#include "storage/priceitemrepository.h"

#include <QAbstractListModel>
#include <QHash>
#include <QtQml/qqmlregistration.h>

// QML-facing price book: what things normally cost, name-ordered, with an
// optional text filter. Rows carry both the current price and how it moved
// since the previous recorded one, so the table can show the trend without
// a second round trip.
class PriceListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    // How many of the listed items got dearer / cheaper at their last
    // recorded change, for the "3 up, 1 down" line above the list.
    Q_PROPERTY(int risenCount READ risenCount NOTIFY countChanged)
    Q_PROPERTY(int fallenCount READ fallenCount NOTIFY countChanged)

public:
    enum Role {
        PriceItemIdRole = Qt::UserRole + 1,
        NameRole,
        UnitRole,
        PriceMinorRole,
        PriceFormattedRole,
        CategoryIdRole, // -1 when uncategorized
        CategoryNameRole,
        CategoryColorRole,
        HasCategoryRole,
        UpdatedAtFormattedRole,
        HasPreviousPriceRole,
        PreviousPriceFormattedRole,
        PriceRoseRole,
        PriceFellRole,
        // Signed percentage against the previous price; 0 when there is none
        ChangePercentRole,
    };
    Q_ENUM(Role)

    explicit PriceListModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] QString searchText() const { return m_searchText; }
    void setSearchText(const QString& text);
    [[nodiscard]] int count() const { return int(m_items.size()); }
    [[nodiscard]] int risenCount() const;
    [[nodiscard]] int fallenCount() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE int priceItemIdAt(int row) const;
    Q_INVOKABLE QVariantMap get(int row) const;

signals:
    void searchTextChanged();
    void countChanged();

private:
    void reload();

    QList<PriceItem> m_items;
    QString m_searchText;
};
