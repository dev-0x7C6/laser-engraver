#include "glbr-settings.hpp"
#include "ui_glbr-settings.h"

#include <externals/common/types.hpp>
#include <variant>
#include <bitset>
#include <chrono>
#include <array>

struct speed_mm_per_min {
	double value;
};

struct acceleration_mm_per_sec_square {
	double value;
};

namespace glbr {

struct step_point_invert {
	bool x : 1 {false};
	bool y : 1 {false};
	bool z : 1 {false};
};

/*
$0=10
$1=25
$2=0
$3=0
$4=0
$5=0
$6=0
$10=1
$11=0.010
$12=0.002
$13=0
$20=0
$21=0
$22=1
$23=0
$24=25.000
$25=500.000
$26=250
$27=1.000
$30=1000.
$31=0.
$32=0
$100=250.000
$101=250.000
$102=250.000
$110=500.000
$111=500.000
$112=500.000
$120=10.000
$121=10.000
$122=10.000
$130=200.000
$131=200.000
$132=200.000
*/

enum glbr_setting_type {
	boolean,
	mask,
	microsecs,
	milisecs,
	mm,
	mm_per_min,
	mm_per_sec_square,
	rpm,
	steps_per_mm,

};

struct setting {
	u8 id;
	double value;
	std::string_view name;
	glbr_setting_type type;
};

const auto ma = std::vector<setting>{{
	{0, 10, "Step pulse", microsecs},
	{1, 25, "Step idle delay", milisecs},
	{2, 0, "Step port invert", mask},
	{3, 0, "Direction port invert", mask},
	{4, 0, "Step enable invert", boolean},
	{5, 0, "Limit pins invert", boolean},
	{6, 0, "Probe pin invert", boolean},
	{10, 1, "Status report", mask},
	{11, 0.010, "Junction deviation", mm},
	{12, 0.002, "Arc tolerance", mm},
	{13, 0, "Report inches", boolean},
	{20, 0, "Soft limits", boolean},
	{21, 0, "Hard limits", boolean},
	{22, 1, "Homing cycle", boolean},
	{23, 0, "Homing dir invert", mask},
	{24, 25, "Homing feed", mm_per_min},
	{25, 500, "Homing seek", mm_per_min},
	{26, 250, "Homing debounce", milisecs},
	{27, 1, "Homing pull-off", mm},
	{30, 1000, "Max spindle speed", rpm},
	{31, 0, "Min spindle speed", rpm},
	{32, 0, "Laser mode", boolean},
	{100, 80.00000, "Steps per X", steps_per_mm},
	{101, 80.00000, "Steps per Y", steps_per_mm},
	{102, 80.00000, "Steps per Z", steps_per_mm},
	{110, 4000.000, "Max rate X", mm_per_min},
	{111, 4000.000, "Max rate Y", mm_per_min},
	{112, 4000.000, "Max rate Z", mm_per_min},
	{120, 100.0000, "Acceleration X", mm_per_sec_square},
	{121, 100.0000, "Acceleration Y", mm_per_sec_square},
	{122, 100.0000, "Acceleration Z", mm_per_sec_square},
	{130, 4000.000, "Max travel X", mm},
	{131, 4000.000, "Max travel Y", mm},
	{132, 4000.000, "Max travel Z", mm},
}};

} // namespace glbr

GlbrSettings::GlbrSettings(QWidget *parent)
		: QMainWindow(parent)
		, m_ui(std::make_unique<Ui::GlbrSettings>()) {
	m_ui->setupUi(this);
}

GlbrSettings::~GlbrSettings() = default;
