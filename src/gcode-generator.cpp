#include "gcode-generator.hpp"

#include <string>

#include <src/utils.hpp>

namespace {

class gcode_generator {
public:
	gcode_generator(const double dpi = 300.0)
			: m_precision(calculate_precision(dpi)) {}

	std::string operator()(const instruction::dwell v) const noexcept { return "G4 P" + std::to_string(divide(v.delay, 1000.0)); }
	std::string operator()(const instruction::home) const noexcept { return "G0 X0 Y0"; }
	std::string operator()(const instruction::set_home_position) const noexcept { return "G92 X0 Y0 Z0"; }
	std::string operator()(const instruction::laser_off) const noexcept { return "M5"; }
	std::string operator()(const instruction::laser_on) const noexcept { return "M3"; }
	std::string operator()(const instruction::move_dpi v) const noexcept {
		std::string ret{"G0"};
		if (v.x)
			ret += " X" + std::to_string(divide(v.x.value(), m_precision));

		if (v.y)
			ret += " Y" + std::to_string(divide(v.y.value(), m_precision));

		if (v.power)
			ret += " S" + std::to_string(v.power.value());

		return ret;
	}
	std::string operator()(const instruction::move_mm v) const noexcept {
		std::string ret{"G0"};
		if (v.x)
			ret += " X" + std::to_string(divide(v.x.value(), m_precision));

		if (v.y)
			ret += " Y" + std::to_string(divide(v.y.value(), m_precision));

		return ret;
	}

	std::string operator()(const instruction::power v) const noexcept { return "S" + std::to_string(v.duty); }
	std::string operator()(const std::monostate) const noexcept { return {}; }
	std::string operator()(const instruction::wait_for_movement_finish) const noexcept { return "G4 P0"; }

private:
	const double m_precision{calculate_precision(300.0)};
};
} // namespace

void generate_gcode(semi::gcodes &&gcodes, const gcode_generation_options &opts, const upload_instruction &instruction) {
	gcode_generator visitor(opts.dpi);
	for (auto i = 0u; i < gcodes.size(); ++i) {
		auto &&gcode = gcodes[i];
		switch (instruction(std::visit(visitor, gcode), divide(i, gcodes.size() - 1))) {
			case upload_instruction_ret::keep_going:
				break;
			case upload_instruction_ret::cancel:
				return;
		}
	}
}
