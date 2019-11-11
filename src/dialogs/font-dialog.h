#pragma once

#include <QDialog>

#include <memory>

namespace Ui {
class FontDialog;
}

struct TextWithFont {
	QString text;
	QFont font;
};

class FontDialog : public QDialog {
	Q_OBJECT
public:
	explicit FontDialog(QWidget *parent = nullptr);
	~FontDialog();

	auto result() const noexcept { return m_result; }

private:
	void updateFont();

private:
	std::unique_ptr<Ui::FontDialog> m_ui;
	std::optional<TextWithFont> m_result;
};
