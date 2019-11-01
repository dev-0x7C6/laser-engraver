#include "gui-settings.h"
#include "ui_mainwindow.h"

#include <externals/common/qt/raii/raii-settings-group.hpp>

#include <QCheckBox>

QCheckBox *get_checkbox(Ui::MainWindow &ui, const sheet::iso216_category_a category) noexcept {
	switch (category) {
		case sheet::iso216_category_a::A0: return ui.drawA0;
		case sheet::iso216_category_a::A1: return ui.drawA1;
		case sheet::iso216_category_a::A2: return ui.drawA2;
		case sheet::iso216_category_a::A3: return ui.drawA3;
		case sheet::iso216_category_a::A4: return ui.drawA4;
		case sheet::iso216_category_a::A5: return ui.drawA5;
		case sheet::iso216_category_a::A6: return ui.drawA6;
		case sheet::iso216_category_a::A7: return ui.drawA7;
		case sheet::iso216_category_a::A8: return ui.drawA8;
		case sheet::iso216_category_a::A9: return ui.drawA9;
		case sheet::iso216_category_a::A10: return ui.drawA10;
	}

	return ui.drawA0;
}

GuiSettings::GuiSettings(Ui::MainWindow &ui, QSettings &settings)
		: ui(ui)
		, settings(settings) {
	raii_settings_group _(settings, "gui");
	{
		raii_settings_group _(settings, "reference");
		for (auto &&category : sheet::all_iso216_category())
			get_checkbox(ui, category)->setChecked(settings.value(QString("%1_checked").arg(name(category)), false).toBool());

		ui.drawInverted->setChecked(settings.value("inverted", true).toBool());
		ui.drawCustom->setChecked(settings.value("custom_checked", false).toBool());
		ui.drawCustomW->setValue(settings.value("custom_w", 40.0).toDouble());
		ui.drawCustomH->setValue(settings.value("custom_h", 40.0).toDouble());
	}

	{
		raii_settings_group _(settings, "viewport");
		ui.grid->setValue(settings.value("grid_mm", 10.00).toDouble());
		ui.workspaceScale->setValue(settings.value("scale", 1.00).toDouble());
	}

	{
		raii_settings_group _(settings, "engraver");
		ui.engraveFromCenter->setChecked(settings.value("engrave_from_center", false).toBool());
		ui.saveHomeAfterMove->setChecked(settings.value("save_home_after_move", true).toBool());
	}
}

GuiSettings::~GuiSettings() {
	raii_settings_group _(settings, "gui");
	{
		raii_settings_group _(settings, "reference");
		for (auto &&category : sheet::all_iso216_category())
			settings.setValue(QString("%1_checked").arg(name(category)), get_checkbox(ui, category)->isChecked());

		settings.setValue("inverted", ui.drawInverted->isChecked());
		settings.setValue("custom_checked", ui.drawCustom->isChecked());
		settings.setValue("custom_w", ui.drawCustomW->value());
		settings.setValue("custom_h", ui.drawCustomH->value());
	}

	{
		raii_settings_group _(settings, "viewport");
		settings.setValue("grid_mm", ui.grid->value());
		settings.setValue("scale", ui.workspaceScale->value());
	}

	{
		raii_settings_group _(settings, "engraver");
		settings.setValue("engrave_from_center", ui.engraveFromCenter->isChecked());
		settings.setValue("save_home_after_move", ui.saveHomeAfterMove->isChecked());
	}
}
