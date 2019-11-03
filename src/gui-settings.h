#pragma once

#include <memory>
#include <src/sheets.hpp>

class QSettings;
class QCheckBox;
class QLabel;

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

QCheckBox *get_checkbox(Ui::MainWindow &, sheet::iso216_category_a) noexcept;
QLabel *get_label(Ui::MainWindow &, sheet::iso216_category_a) noexcept;
