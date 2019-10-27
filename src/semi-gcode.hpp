#pragma once

#include <externals/common/types.hpp>

#include <atomic>
#include <variant>
#include <vector>
#include <optional>

class QImage;

struct options {
	double power_multiplier{1.0};
	std::optional<u16> force_dwell_time;
	bool center_object{true};
};

using progress_t = std::atomic<double>;

namespace instruction {

struct laser_on {};
struct laser_off {};
struct set_home_position {
	float x;
	float y;
};
struct home {};
struct wait_for_movement_finish {};

struct dwell {
	u16 delay;
};

struct move_dpi {
	float x;
	float y;
	u16 power;
};

struct move_mm {
	float x;
	float y;
};

struct power {
	i32 duty;
};
} // namespace instruction

using semi_gcode = std::variant<std::monostate, instruction::laser_on, instruction::laser_off, instruction::home, instruction::set_home_position, instruction::dwell, instruction::move_dpi, instruction::move_mm, instruction::power, instruction::wait_for_movement_finish>;
using semi_gcodes = std::vector<semi_gcode>;

semi_gcodes image_to_semi_gcode(const QImage &img, options, progress_t &);
semi_gcodes generate_workspace_demo(const QImage &img, options);
semi_gcodes generate_begin_section();
semi_gcodes generate_end_section();
