#include "gcode-generator.hpp"

gcode::generator::grbl::grbl(const double dpi)
		: m_precision(calculate_precision(dpi)) {}

std::string gcode::generator::grbl::operator()(const instruction::dwell v) const noexcept { return "G4 P" + std::to_string(divide(v.delay, 1000.0)); }

std::string gcode::generator::grbl::operator()(const instruction::wait_for_movement_finish) const noexcept { return "G4 P0"; }

std::string gcode::generator::grbl::operator()(const std::monostate) const noexcept { return {}; }

std::string gcode::generator::grbl::operator()(const instruction::power v) const noexcept { return "S" + std::to_string(v.duty); }

std::string gcode::generator::grbl::operator()(const instruction::move_mm v) const noexcept {
	std::string ret{"G0"};
	if (v.x)
		ret += " X" + std::to_string(divide(v.x.value(), m_precision));

	if (v.y)
		ret += " Y" + std::to_string(divide(v.y.value(), m_precision));

	return ret;
}

std::string gcode::generator::grbl::operator()(const instruction::move_dpi v) const noexcept {
	std::string ret{"G0"};
	if (v.x)
		ret += " X" + std::to_string(divide(v.x.value(), m_precision));

	if (v.y)
		ret += " Y" + std::to_string(divide(v.y.value(), m_precision));

	if (v.power)
		ret += " S" + std::to_string(v.power.value());

	return ret;
}

std::string gcode::generator::grbl::operator()(const instruction::laser_on) const noexcept { return "M3"; }

std::string gcode::generator::grbl::operator()(const instruction::laser_off) const noexcept { return "M5"; }

std::string gcode::generator::grbl::operator()(const instruction::set_home_position) const noexcept { return "G92 X0 Y0 Z0"; }

std::string gcode::generator::grbl::operator()(const instruction::home) const noexcept { return "G0 X0 Y0"; }
