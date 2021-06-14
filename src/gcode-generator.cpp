#include "gcode-generator.hpp"

#include <cstdio>

gcode::generator::grbl::grbl(const double dpi)
		: m_precision(calculate_precision(dpi)) {}

std::string gcode::generator::grbl::operator()(const instruction::dwell v) const noexcept { return "G4 P" + std::to_string(divide(v.delay, 1000.0)); }

std::string gcode::generator::grbl::operator()(const instruction::wait_for_movement_finish) const noexcept { return "G4 P0"; }

std::string gcode::generator::grbl::operator()(const std::monostate) const noexcept { return {}; }

std::string gcode::generator::grbl::operator()(const instruction::power v) const noexcept { return "S" + std::to_string(v.duty); }

namespace {
auto code(const instruction::move &move) -> const char * {
	switch (move.type) {
		case instruction::move::etype::precise:
			return "G1";

		case instruction::move::etype::rapid:
			return "G0";
	}

	return nullptr;
}

auto convert(const float v) -> std::string {
	std::string ret;
	ret.resize(std::snprintf(nullptr, 0, "%.3f", v), 0);
	std::snprintf(ret.data(), ret.size() + 1, "%.3f", v);
	return ret;
}

} // namespace

std::string gcode::generator::grbl::operator()(const instruction::move v) const noexcept {
	std::string ret = code(v);
	auto calc = [&](auto value) -> float {
		if (instruction::move::escale::dpi == v.scale)
			return divide(value, m_precision);

		return value;
	};

	ret += " X" + convert(calc(v.x));
	ret += " Y" + convert(calc(v.y));
	ret += " F" + std::to_string(v.feedrate);
	ret += " S" + std::to_string(v.pwr.duty);

	return ret;
}

std::string gcode::generator::grbl::operator()(const instruction::laser_on) const noexcept { return "M3"; }

std::string gcode::generator::grbl::operator()(const instruction::laser_off) const noexcept { return "M5"; }

std::string gcode::generator::grbl::operator()(const instruction::set_home_position) const noexcept { return "G92 X0 Y0 Z0"; }

std::string gcode::generator::grbl::operator()(const instruction::home) const noexcept { return "G0 X0 Y0"; }
