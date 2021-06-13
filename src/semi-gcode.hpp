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

namespace filters {
struct options {
	std::optional<float> black_and_white_treshold;
};

float black_and_white_treshold_filter(const options &opts, float in);
} // namespace filters

enum strategy {
	dot,
	lines
};

struct feedrate {
	std::optional<float> rapid;
	std::optional<float> precise;
};

struct options {
	float spindle_power_multiplier{1.0f};
	int spindle_max_power{255};
	u32 repeat_line_count{0};
	std::optional<u16> force_dwell_time;
	strategy strat{strategy::dot};
	feedrate speed;
	filters::options filters{};
	bool center_object{true};
};

using gcode = std::variant<
	std::monostate,
	instruction::dwell,
	instruction::home,
	instruction::laser_off,
	instruction::laser_on,
	instruction::move,
	instruction::power,
	instruction::set_home_position,
	instruction::wait_for_movement_finish>;

using gcodes = std::vector<gcode>;

namespace calculate {
i32 power(int color, const semi::options &opts) noexcept;
}

namespace generator {
semi::gcodes from_image(const QImage &img, semi::options, progress_t &);
semi::gcodes workspace_preview(const QImage &img, semi::options);
semi::gcodes finalization();
} // namespace generator

} // namespace semi
