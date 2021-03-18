#include "font-dialog.h"
#include "ui_font-dialog.h"

FontDialog::FontDialog(std::optional<TextWithFont> content, QWidget *parent)
		: QDialog(parent)
		, m_ui(std::make_unique<Ui::FontDialog>()) {
	m_ui->setupUi(this);
	selectedFontFromCombo();

	if (content)
		updateFromContent(content.value());

	connect(m_ui->font, &QFontComboBox::currentFontChanged, this, &FontDialog::selectedFontFromCombo);
	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, [this]() {
		TextWithFont ret;
		ret.font = m_ui->textEdit->font();
		ret.text = m_ui->textEdit->toPlainText();
		m_result = std::move(ret);
	});

	connect(m_ui->size, qOverload<int>(&QSpinBox::valueChanged), [this](auto &&) { selectedFontFromCombo(); });
	connect(m_ui->bold, &QToolButton::toggled, [this](auto &&) { selectedFontFromCombo(); });
	connect(m_ui->italic, &QToolButton::toggled, [this](auto &&) { selectedFontFromCombo(); });
}

void FontDialog::updateFromContent(const TextWithFont &content) {
	auto &&font = content.font;
	auto &&text = content.text;

	m_ui->bold->setChecked(font.bold());
	m_ui->font->setCurrentFont(font);
	m_ui->italic->setChecked(font.italic());
	m_ui->size->setValue(font.pixelSize());
	m_ui->textEdit->setFont(font);
	m_ui->textEdit->setPlainText(text);
}

void FontDialog::selectedFontFromCombo() {
	auto font = m_ui->font->currentFont();
	font.setPixelSize(m_ui->size->value());
	font.setBold(m_ui->bold->isChecked());
	font.setItalic(m_ui->italic->isChecked());
	m_ui->textEdit->setFont(font);
}

FontDialog::~FontDialog() = default;
