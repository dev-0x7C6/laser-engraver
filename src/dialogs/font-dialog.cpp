#include "font-dialog.h"
#include "ui_font-dialog.h"

FontDialog::FontDialog(QWidget *parent)
		: QDialog(parent)
		, m_ui(std::make_unique<Ui::FontDialog>()) {
	m_ui->setupUi(this);
	updateFont(m_ui->textEdit->font());

	connect(m_ui->fontComboBox, &QFontComboBox::currentFontChanged, this, &FontDialog::updateFont);
	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, [this]() {
		TextWithFont ret;
		ret.font = m_ui->textEdit->font();
		ret.text = m_ui->textEdit->toPlainText();
		m_result = std::move(ret);
	});
	connect(m_ui->spinBox, qOverload<int>(&QSpinBox::valueChanged), [this]() {
		updateFont(m_ui->textEdit->font());
	});
}

void FontDialog::updateFont(QFont font) {
	font.setPixelSize(m_ui->spinBox->value());
	m_ui->textEdit->setFont(font);
}

FontDialog::~FontDialog() = default;
