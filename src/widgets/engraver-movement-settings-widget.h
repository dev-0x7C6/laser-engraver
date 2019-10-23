#pragma once

#include <QWidget>
#include <memory>

#include <src/engraver-settings.h>

namespace Ui {
class EngraverMovementSettingsWidget;
}

class EngraverMovementSettingsWidget : public QWidget {
	Q_OBJECT
public:
	explicit EngraverMovementSettingsWidget(QWidget *parent = nullptr);
	~EngraverMovementSettingsWidget();

	void setParameters(const engraver::settings::movement_parameters &parameters);
	engraver::settings::movement_parameters parameters();

signals:
	void settingsChanged();

private:
	std::unique_ptr<Ui::EngraverMovementSettingsWidget> m_ui;
};
