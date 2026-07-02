#pragma once

#include <QList>
#include <QSqlTableModel>

struct Category {
    int id = 0;
    QString name;
    QString type;
    QString color;

    bool isSystem() const { return type == QLatin1String("system"); }
};

// Table model over `categories` (used by ManageCategoriesDialog) plus the
// static helpers enforcing the business rules: system categories and
// categories still referenced by expenses or recurring bills are undeletable.
class CategoryModel : public QSqlTableModel {
    Q_OBJECT
public:
    explicit CategoryModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Category categoryAt(int row) const;
    void reload();

    static QList<Category> allCategories();
    static bool addCategory(const QString& name, const QString& color,
                            QString* error = nullptr);
    static bool renameCategory(int id, const QString& newName, QString* error = nullptr);
    static bool setColor(int id, const QString& color, QString* error = nullptr);
    static bool removeCategory(int id, QString* error = nullptr);
    static bool isInUse(int id);
    // First palette slot not already taken by an existing category
    static QString suggestedColor();
};
