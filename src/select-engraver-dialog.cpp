#include "select-engraver-dialog.h"
#include "ui_select-engraver-dialog.h"

Q_DECLARE_METATYPE(EngraverSettings);

SelectEngraverDialog::SelectEngraverDialog(const std::vector<EngraverSettings> &engravers, QWidget *parent)
		: QDialog(parent)
		, m_ui(std::make_unique<Ui::SelectEngraverDialog>()) {
	m_ui->setupUi(this);

	for (auto &&engraver : engravers)
		m_ui->engraver->addItem(engraver.name, QVariant::fromValue(engraver));

	connect(this, &SelectEngraverDialog::accepted, [this]() {
		m_selectedEngraver = m_ui->engraver->currentData().value<EngraverSettings>();
	});
}

SelectEngraverDialog::~SelectEngraverDialog() = default;
