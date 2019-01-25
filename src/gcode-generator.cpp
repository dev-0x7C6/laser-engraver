#include "gcode-generator.hpp"

namespace {

constexpr double precision_multiplier(double dpi = 600) {
	return dpi / 25.4;
}

class gcode_generator {
public:
	gcode_generator(std::ostream &stream, const double dpi = 600)
			: m_stream(stream)
			, m_precision(precision_multiplier(dpi)) {}

	void operator()(const dwell &value) { m_stream << "G4 P0.00" << value.delay << std::endl; }
	void operator()(home) { m_stream << "G0 X0 Y0" << std::endl; }
	void operator()(laser_off) { m_stream << "M5" << std::endl; }
	void operator()(laser_on) { m_stream << "M3" << std::endl; }
	void operator()(const move &value) { m_stream << "G0 X" << (static_cast<double>(value.x) / m_precision) << " Y" << (static_cast<double>(value.y) / m_precision) << std::endl; }
	void operator()(const power &value) { m_stream << "S" << value.duty << std::endl; }
	void operator()(std::monostate) {}

private:
	std::ostream &m_stream;
	const double m_precision{1.0};
};
}

void generate_gcode(std::string &&dir, semi_gcodes &&gcodes) {
	std::ofstream file(dir + "/result.gcode", std::ios::trunc);

	gcode_generator visitor(file);

	for (auto &&gcode : gcodes)
		std::visit(visitor, gcode);
}
