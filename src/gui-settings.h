#pragma once

#include <memory>
#include <src/sheets.hpp>

class QSettings;
class QCheckBox;

namespace Ui {
class MainWindow;
}

class GuiSettings {
public:
	GuiSettings(Ui::MainWindow &ui, QSettings &settings);
	~GuiSettings();

private:
	Ui::MainWindow &ui;
	QSettings &settings;
};

QCheckBox *get_checkbox(Ui::MainWindow &ui, const sheet::iso216_category_a category) noexcept;
