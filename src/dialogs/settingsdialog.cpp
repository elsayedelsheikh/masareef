#include "dialogs/settingsdialog.h"

#include "utils/appconfig.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>

namespace {
const struct { const char* code; const char* symbol; } kCurrencies[] = {
    { "EGP", "ج.م" },
    { "USD", "$" },
    { "EUR", "€" },
    { "GBP", "£" },
    { "SAR", "ر.س" },
    { "AED", "د.إ" },
    { "KWD", "د.ك" },
};
} // namespace

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    setMinimumWidth(320);

    m_code = new QComboBox(this);
    m_code->setEditable(true);
    for (const auto& currency : kCurrencies)
        m_code->addItem(QString::fromUtf8(currency.code));

    m_symbol = new QLineEdit(this);

    m_code->setCurrentText(AppConfig::currencyCode());
    m_symbol->setText(AppConfig::currencySymbol());

    // Picking or typing a known code suggests its symbol; the field stays
    // editable. editTextChanged also fires per keystroke while typing,
    // which currentTextChanged does not for an editable combo.
    connect(m_code, &QComboBox::editTextChanged, this, [this](const QString& code) {
        for (const auto& currency : kCurrencies) {
            if (code == QLatin1String(currency.code)) {
                m_symbol->setText(QString::fromUtf8(currency.symbol));
                return;
            }
        }
    });

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QFormLayout(this);
    layout->addRow(tr("Currency code:"), m_code);
    layout->addRow(tr("Currency symbol:"), m_symbol);
    layout->addRow(new QLabel(tr("The symbol is shown next to every amount."), this));
    layout->addRow(buttons);
}

void SettingsDialog::accept()
{
    const QString code = m_code->currentText().trimmed().toUpper();
    const QString symbol = m_symbol->text().trimmed();
    AppConfig::setCurrency(code.isEmpty() ? QStringLiteral("EGP") : code,
                           symbol.isEmpty() ? code : symbol);
    QDialog::accept();
}
