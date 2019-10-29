#pragma once

#include <externals/common/types.hpp>

#include <atomic>
#include <variant>
#include <vector>
#include <optional>

#include <src/instructions.hpp>

class QImage;

using progress_t = std::atomic<double>;

namespace semi {

struct options {
	double power_multiplier{1.0};
	std::optional<u16> force_dwell_time;
	bool center_object{true};
};

using gcode = std::variant<std::monostate, instruction::laser_on, instruction::laser_off, instruction::home, instruction::set_home_position, instruction::dwell, instruction::move_dpi, instruction::move_mm, instruction::power, instruction::wait_for_movement_finish>;
using gcodes = std::vector<gcode>;

namespace generator {
semi::gcodes from_image(const QImage &img, semi::options, progress_t &);
semi::gcodes workspace_preview(const QImage &img, semi::options);
semi::gcodes finalization();
} // namespace generator

} // namespace semi
