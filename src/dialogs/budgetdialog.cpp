#include "dialogs/budgetdialog.h"

#include "storage/budgetrepository.h"
#include "storage/categoryrepository.h"
#include "utils/appconfig.h"
#include "utils/currencyformatter.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpressionValidator>

namespace {

QLineEdit* makeAmountEdit(QWidget* parent, std::optional<Money> current)
{
    auto* edit = new QLineEdit(parent);
    edit->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral(R"(\d{1,9}([.,]\d{0,2})?)")), edit));
    edit->setPlaceholderText(QObject::tr("no budget"));
    if (current)
        edit->setText(CurrencyFormatter::formatPlain(*current).remove(QLatin1Char(',')));
    return edit;
}

// Empty field -> nullopt (clear the budget); anything else must parse.
std::optional<std::optional<Money>> parseBudgetField(const QLineEdit* edit)
{
    const QString text = edit->text().trimmed();
    if (text.isEmpty())
        return std::optional<Money>{};
    const std::optional<Money> amount = CurrencyFormatter::parse(text);
    if (!amount)
        return std::nullopt;
    return amount;
}

} // namespace

BudgetDialog::BudgetDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Monthly Budget"));
    setMinimumWidth(360);

    auto* layout = new QFormLayout(this);

    m_overall = makeAmountEdit(this, BudgetRepository::overallBudget());
    layout->addRow(tr("Overall budget (%1):").arg(AppConfig::currencyCode()), m_overall);

    auto* hint = new QLabel(
        tr("The dashboard tracks spending against these amounts each calendar month. "
           "Leave a field empty for no budget."),
        this);
    hint->setWordWrap(true);
    layout->addRow(hint);

    auto* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    layout->addRow(separator);

    const QHash<int, Money> budgets = BudgetRepository::categoryBudgets();
    const QList<Category> categories = CategoryRepository::all();
    for (const Category& cat : categories) {
        auto* edit = makeAmountEdit(
            this, budgets.contains(cat.id) ? std::optional(budgets.value(cat.id))
                                           : std::nullopt);
        layout->addRow(cat.name + QLatin1Char(':'), edit);
        m_categoryEdits.append({ cat.id, edit });
    }

    auto* buttons =
        new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addRow(buttons);
}

void BudgetDialog::accept()
{
    const auto overall = parseBudgetField(m_overall);
    if (!overall) {
        QMessageBox::warning(this, windowTitle(), tr("Please enter a valid overall amount."));
        m_overall->setFocus();
        return;
    }

    QList<QPair<int, std::optional<Money>>> categoryAmounts;
    for (const auto& [categoryId, edit] : m_categoryEdits) {
        const auto amount = parseBudgetField(edit);
        if (!amount) {
            QMessageBox::warning(this, windowTitle(), tr("Please enter a valid amount."));
            edit->setFocus();
            return;
        }
        categoryAmounts.append({ categoryId, *amount });
    }

    if (auto res = BudgetRepository::setOverallBudget(*overall); !res) {
        QMessageBox::warning(this, windowTitle(), res.error().message);
        return;
    }
    for (const auto& [categoryId, amount] : categoryAmounts) {
        if (auto res = BudgetRepository::setCategoryBudget(categoryId, amount); !res) {
            QMessageBox::warning(this, windowTitle(), res.error().message);
            return;
        }
    }
    QDialog::accept();
}
