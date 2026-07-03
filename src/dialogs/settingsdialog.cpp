#include "dialogs/settingsdialog.h"

#include "utils/appconfig.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>

namespace {
// Codes only — amounts always display as "100.50 EGP", never with a
// localized symbol.
constexpr const char* kCurrencyCodes[] = {
    "EGP", "USD", "EUR", "GBP", "SAR", "AED", "KWD",
};
} // namespace

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    setMinimumWidth(320);

    m_code = new QComboBox(this);
    m_code->setEditable(true);
    for (const char* code : kCurrencyCodes)
        m_code->addItem(QString::fromLatin1(code));
    m_code->setCurrentText(AppConfig::currencyCode());

    m_theme = new QComboBox(this);
    m_theme->addItem(tr("System"), int(ThemePreference::System));
    m_theme->addItem(tr("Light"), int(ThemePreference::Light));
    m_theme->addItem(tr("Dark"), int(ThemePreference::Dark));
    m_theme->setCurrentIndex(m_theme->findData(int(AppConfig::theme())));

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QFormLayout(this);
    layout->addRow(tr("Currency:"), m_code);
    layout->addRow(new QLabel(tr("The code is shown next to every amount."), this));
    layout->addRow(tr("Theme:"), m_theme);
    layout->addRow(buttons);
}

void SettingsDialog::accept()
{
    const QString code = m_code->currentText().trimmed().toUpper();
    AppConfig::setCurrencyCode(code.isEmpty() ? QStringLiteral("EGP") : code);
    AppConfig::setTheme(ThemePreference(m_theme->currentData().toInt()));
    QDialog::accept();
}
