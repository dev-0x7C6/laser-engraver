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
};

using progress_t = std::atomic<double>;

struct laser_on {};
struct laser_off {};
struct home {};

struct dwell {
	i16 delay;
};

struct move {
	i16 x;
	i16 y;
};

struct power {
	i16 duty;
};

using semi_gcode = std::variant<std::monostate, laser_on, laser_off, home, dwell, move, power>;
using semi_gcodes = std::vector<semi_gcode>;

semi_gcodes image_to_semi_gcode(const QImage &img, options, progress_t &);
