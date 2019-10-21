#pragma once

#include <memory>
#include <optional>
#include <QDialog>

#include <src/engraver-settings.h>

namespace Ui {
class AddPrinterDialog;
}

class AddEngraverDialog : public QDialog {
	Q_OBJECT

public:
	explicit AddEngraverDialog(QWidget *parent = nullptr);
	~AddEngraverDialog();

	auto result() { return m_result; }

private:
	void saveResult();

private:
	std::optional<EngraverSettings> m_result;
	std::unique_ptr<Ui::AddPrinterDialog> m_ui;
};
