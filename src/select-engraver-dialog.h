#pragma once

#include <QDialog>

#include <memory>
#include <src/engraver-settings.h>

namespace Ui {
class SelectEngraverDialog;
}

class SelectEngraverDialog : public QDialog {
	Q_OBJECT

public:
	explicit SelectEngraverDialog(const engraver::settings::configurations &configurations, QWidget *parent = nullptr);
	~SelectEngraverDialog();

	auto result() { return m_selectedEngraver; }

private:
	std::unique_ptr<Ui::SelectEngraverDialog> m_ui;
	engraver::settings::optional_configuration m_selectedEngraver;
};
