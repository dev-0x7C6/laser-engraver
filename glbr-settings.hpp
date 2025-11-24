#pragma once

#include <QMainWindow>
#include <memory>

namespace Ui {
class GlbrSettings;
}

class GlbrSettings : public QMainWindow {
	Q_OBJECT

public:
	explicit GlbrSettings(QWidget *parent = nullptr);
	~GlbrSettings();

private:
	std::unique_ptr<Ui::GlbrSettings> m_ui;
};
