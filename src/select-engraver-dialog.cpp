#include "select-engraver-dialog.h"
#include "ui_select-engraver-dialog.h"

Q_DECLARE_METATYPE(engraver::settings::configuration);

SelectEngraverDialog::SelectEngraverDialog(const engraver::settings::configurations &configurations, QWidget *parent)
		: QDialog(parent)
		, m_ui(std::make_unique<Ui::SelectEngraverDialog>()) {
	m_ui->setupUi(this);

	for (auto &&engraver : configurations)
		m_ui->engraver->addItem(engraver.name, QVariant::fromValue(engraver));

	connect(this, &SelectEngraverDialog::accepted, [this]() {
		m_selectedEngraver = m_ui->engraver->currentData().value<engraver::settings::configuration>();
	});
}

SelectEngraverDialog::~SelectEngraverDialog() = default;
