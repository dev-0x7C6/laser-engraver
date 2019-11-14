#include "font-dialog.h"
#include "ui_font-dialog.h"

FontDialog::FontDialog(QWidget *parent)
		: QDialog(parent)
		, m_ui(std::make_unique<Ui::FontDialog>()) {
	m_ui->setupUi(this);
	updateFont();

	connect(m_ui->font, &QFontComboBox::currentFontChanged, this, &FontDialog::updateFont);
	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, [this]() {
		TextWithFont ret;
		ret.font = m_ui->textEdit->font();
		ret.text = m_ui->textEdit->toPlainText();
		m_result = std::move(ret);
	});

	connect(m_ui->size, qOverload<int>(&QSpinBox::valueChanged), [this](auto &&) { updateFont(); });
	connect(m_ui->bold, &QToolButton::toggled, [this](auto &&) { updateFont(); });
	connect(m_ui->italic, &QToolButton::toggled, [this](auto &&) { updateFont(); });
}

FontDialog::FontDialog(const QFont &font, const QString &text, QWidget *parent)
		: FontDialog(parent) {
	m_ui->textEdit->setFont(font);
	m_ui->textEdit->setPlainText(text);
	updateFont();
}

void FontDialog::updateFont() {
	auto font = m_ui->textEdit->font();
	font.setPixelSize(m_ui->size->value());
	font.setBold(m_ui->bold->isChecked());
	font.setItalic(m_ui->italic->isChecked());
	m_ui->textEdit->setFont(font);
}

FontDialog::~FontDialog() = default;
