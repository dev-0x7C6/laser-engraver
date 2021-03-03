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

QLabel *get_label(Ui::MainWindow &ui, const sheet::iso216_category_a category) noexcept {
	switch (category) {
		case sheet::iso216_category_a::A0: return ui.drawA0Label;
		case sheet::iso216_category_a::A1: return ui.drawA1Label;
		case sheet::iso216_category_a::A2: return ui.drawA2Label;
		case sheet::iso216_category_a::A3: return ui.drawA3Label;
		case sheet::iso216_category_a::A4: return ui.drawA4Label;
		case sheet::iso216_category_a::A5: return ui.drawA5Label;
		case sheet::iso216_category_a::A6: return ui.drawA6Label;
		case sheet::iso216_category_a::A7: return ui.drawA7Label;
		case sheet::iso216_category_a::A8: return ui.drawA8Label;
		case sheet::iso216_category_a::A9: return ui.drawA9Label;
		case sheet::iso216_category_a::A10: return ui.drawA10Label;
	}

	return ui.drawA0Label;
}

namespace {
namespace defaults {
constexpr auto WORKSPACE_CUSTOM_REFERENCE_W = 60.00;
constexpr auto WORKSPACE_CUSTOM_REFERENCE_H = 40.00;
constexpr auto WORKSPACE_CUSTOM_REFERENCE_INVERTED = true;
constexpr auto WORKSPACE_CUSTOM_REFERENCE_CHECKED = false;

constexpr auto WORKSPACE_DPI = 150;
constexpr auto WORKSPACE_GRID_MM = 10.00;
constexpr auto WORKSPACE_SCALE = 1.00;
constexpr auto WORKSPACE_SCALE_OBJECTS_WITH_DPI = false;

constexpr auto LASER_POWER = 100;
} // namespace defaults
} // namespace

using namespace defaults;

GuiSettings::GuiSettings(Ui::MainWindow &ui, QSettings &settings)
		: ui(ui)
		, settings(settings) {
	raii_settings_group _(settings, "gui");
	{
		raii_settings_group _(settings, "reference");
		for (auto &&category : sheet::all_iso216_category())
			get_checkbox(ui, category)->setChecked(settings.value(QString("%1_checked").arg(name(category)), false).toBool());

		ui.drawInverted->setChecked(settings.value("inverted", WORKSPACE_CUSTOM_REFERENCE_INVERTED).toBool());
		ui.drawCustom->setChecked(settings.value("custom_checked", WORKSPACE_CUSTOM_REFERENCE_CHECKED).toBool());
		ui.drawCustomW->setValue(settings.value("custom_w", WORKSPACE_CUSTOM_REFERENCE_W).toDouble());
		ui.drawCustomH->setValue(settings.value("custom_h", WORKSPACE_CUSTOM_REFERENCE_H).toDouble());
	}

	{
		raii_settings_group _(settings, "viewport");
		ui.dpi->setValue(settings.value("dpi", WORKSPACE_DPI).toInt());
		ui.grid->setValue(settings.value("grid_mm", WORKSPACE_GRID_MM).toDouble());
		ui.workspaceScale->setValue(settings.value("scale", WORKSPACE_SCALE).toDouble());
		ui.scaleObjectsWithDpi->setChecked(settings.value("scale_objects_with_dpi", WORKSPACE_SCALE_OBJECTS_WITH_DPI).toBool());
	}

	{
		raii_settings_group _(settings, "engraver");
		ui.engraveObjectFromCenter->setChecked(settings.value("engrave_object_from_center", false).toBool());
		ui.engraveFromCurrentPosition->setChecked(settings.value("engrave_from_current_positon", true).toBool());
	}

	{
		raii_settings_group _(settings, "laser");
		ui.laser_pwr->setValue(settings.value("power", LASER_POWER).toInt());
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
		settings.setValue("dpi", ui.dpi->value());
		settings.setValue("grid_mm", ui.grid->value());
		settings.setValue("scale", ui.workspaceScale->value());
		settings.setValue("scale_objects_with_dpi", ui.scaleObjectsWithDpi->isChecked());
	}

	{
		raii_settings_group _(settings, "engraver");
		settings.setValue("engrave_object_from_center", ui.engraveObjectFromCenter->isChecked());
		settings.setValue("engrave_from_current_positon", ui.engraveFromCurrentPosition->isChecked());
	}

	{
		raii_settings_group _(settings, "laser");
		settings.setValue("power", ui.laser_pwr->value());
	}
}
