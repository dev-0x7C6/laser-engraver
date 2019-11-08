#pragma once

#include <QDialog>

#include <memory>

namespace Ui {
class AddFontDialog;
}

struct TextWithFont {
	QString text;
	QFont font;
};

class AddFontDialog : public QDialog {
	Q_OBJECT
public:
	explicit AddFontDialog(QWidget *parent = nullptr);
	~AddFontDialog();

	auto result() const noexcept { return m_result; }

private:
	void updateFont(QFont font);

private:
	std::unique_ptr<Ui::AddFontDialog> m_ui;
	std::optional<TextWithFont> m_result;
};
