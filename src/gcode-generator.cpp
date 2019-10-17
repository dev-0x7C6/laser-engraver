#include "gcode-generator.hpp"

#include <thread>

namespace {

constexpr double precision_multiplier(double dpi = 600) {
	return dpi / 25.4;
}

class gcode_generator {
public:
	gcode_generator(const double dpi = 300)
			: m_precision(precision_multiplier(dpi)) {}

	std::string operator()(const dwell v) const noexcept {
		std::this_thread::sleep_for(std::chrono::milliseconds(v.delay));
		return {};
	} //return "G4 P0.00" + std::to_string(v.delay); }
	std::string operator()(const home) const noexcept { return "G0 X0.000 Y0.000"; }
	std::string operator()(const laser_off) const noexcept { return "M5"; }
	std::string operator()(const laser_on) const noexcept { return "M3"; }
	std::string operator()(const move v) const noexcept { return "G0 X" + std::to_string(static_cast<double>(v.x) / m_precision) + " Y" + std::to_string(static_cast<double>(v.y) / m_precision); }
	std::string operator()(const power v) const noexcept { return "S" + std::to_string(v.duty); }
	std::string operator()(const std::monostate) const noexcept { return {}; }

private:
	const double m_precision{1.0};
};
}

void generate_gcode(semi_gcodes &&gcodes, const upload_instruction &instruction) {
	gcode_generator visitor;
	for (auto i = 0u; i < gcodes.size(); ++i) {
		auto &&gcode = gcodes[i];
		switch (instruction(std::visit(visitor, gcode), static_cast<double>(i) / static_cast<double>(gcodes.size() - 1))) {
			case upload_instruction_ret::keep_going:
				break;
			case upload_instruction_ret::cancel:
				return;
		}
	}
}
