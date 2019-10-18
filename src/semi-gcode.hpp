#pragma once

#include <externals/common/types.hpp>

#include <atomic>
#include <variant>
#include <functional>
#include <optional>
#include <vector>

#include <QImage>

struct options {
	double power_multiplier{1.0};
	std::optional<i16> force_dwell_time;
	bool center_object{true};
};

using progress_t = std::atomic<double>;

struct laser_on {};
struct laser_off {};
struct home {};
struct wait_for_movement_finish {};

struct dwell {
	i32 delay;
};

struct move {
	i32 x;
	i32 y;
};

struct power {
	i32 duty;
};

using semi_gcode = std::variant<std::monostate, laser_on, laser_off, home, dwell, move, power, wait_for_movement_finish>;
using semi_gcodes = std::vector<semi_gcode>;

semi_gcodes image_to_semi_gcode(const QImage &img, options, progress_t &);
semi_gcodes generate_workspace_demo(const QImage &img, options);
semi_gcodes generate_begin_section();
semi_gcodes generate_end_section();
