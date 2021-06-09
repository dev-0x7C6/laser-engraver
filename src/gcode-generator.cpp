#include "gcode-generator.hpp"

gcode::generator::grbl::grbl(const double dpi)
		: m_precision(calculate_precision(dpi)) {}

std::string gcode::generator::grbl::operator()(const instruction::dwell v) const noexcept { return "G4 P" + std::to_string(divide(v.delay, 1000.0)); }

std::string gcode::generator::grbl::operator()(const instruction::wait_for_movement_finish) const noexcept { return "G4 P0"; }

std::string gcode::generator::grbl::operator()(const std::monostate) const noexcept { return {}; }

std::string gcode::generator::grbl::operator()(const instruction::power v) const noexcept { return "S" + std::to_string(v.duty); }

namespace {
auto movement(instruction::move v, float precision) -> std::string {
	std::string ret;
	auto calc = [&](auto value) -> float {
		if (v.scale)
			return divide(value, precision);

		return value;
	};

	if (v.x)
		ret += " X" + std::to_string(calc(v.x.value()));

	if (v.y)
		ret += " Y" + std::to_string(calc(v.y.value()));

	if (v.feedrate)
		ret += " F" + std::to_string(v.feedrate.value());

	if (v.power)
		ret += " S" + std::to_string(v.power.value());

	return ret;
}
} // namespace

std::string gcode::generator::grbl::operator()(const instruction::move_fast v) const noexcept {
	return "G0" + movement(v, m_precision);
}

std::string gcode::generator::grbl::operator()(const instruction::move v) const noexcept {
	return "G1" + movement(v, m_precision);
}

std::string gcode::generator::grbl::operator()(const instruction::laser_on) const noexcept { return "M3"; }

std::string gcode::generator::grbl::operator()(const instruction::laser_off) const noexcept { return "M5"; }

std::string gcode::generator::grbl::operator()(const instruction::set_home_position) const noexcept { return "G92 X0 Y0 Z0"; }

std::string gcode::generator::grbl::operator()(const instruction::home) const noexcept { return "G0 X0 Y0"; }
