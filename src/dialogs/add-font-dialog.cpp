#include "add-font-dialog.h"
#include "ui_add-font-dialog.h"

AddFontDialog::AddFontDialog(QWidget *parent)
		: QDialog(parent)
		, m_ui(std::make_unique<Ui::AddFontDialog>()) {
	m_ui->setupUi(this);
	setResultFont(m_ui->textEdit->font());

	connect(m_ui->fontComboBox, &QFontComboBox::currentFontChanged, this, &AddFontDialog::setResultFont);
	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
		TextWithFont ret;
		ret.font = m_ui->textEdit->font();
		ret.text = m_ui->textEdit->toPlainText();
		m_result = std::move(ret);
	});
}

void AddFontDialog::setResultFont(QFont font) {
	font.setPixelSize(m_ui->spinBox->value());
	m_ui->textEdit->setFont(font);
}

AddFontDialog::~AddFontDialog() = default;
