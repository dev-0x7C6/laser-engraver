#pragma once

#include <QDialog>

#include <memory>
#include <optional>

namespace Ui {
class FontDialog;
}

struct TextWithFont {
	TextWithFont() = default;
	TextWithFont(const QString &text, const QFont &font)
			: text(text)
			, font(font) {}

	QString text;
	QFont font;
};

class FontDialog : public QDialog {
	Q_OBJECT
public:
	explicit FontDialog(std::optional<TextWithFont> content = {}, QWidget *parent = nullptr);
	~FontDialog();

	auto result() const noexcept { return m_result; }

private:
	void updateFromContent(const TextWithFont &content);
	void selectedFontFromCombo();

private:
	std::unique_ptr<Ui::FontDialog> m_ui;
	std::optional<TextWithFont> m_result;
};
