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
	std::optional<u8> black_and_white_treshold;
};

u8 black_and_white_treshold_filter(const options &opts, u8 in);
} // namespace filters

enum strategy {
	dot,
	lines
};

struct options {
	double power_multiplier{1.0};
	std::optional<u16> force_dwell_time;
	strategy strat{strategy::dot};
	filters::options filters{};
	bool center_object{true};
};

using gcode = std::variant<
	std::monostate,
	instruction::dwell,
	instruction::move_fast,
	instruction::home,
	instruction::laser_off,
	instruction::laser_on,
	instruction::move,
	instruction::power,
	instruction::set_home_position,
	instruction::wait_for_movement_finish>;

using gcodes = std::vector<gcode>;

namespace generator {
semi::gcodes from_image(const QImage &img, semi::options, progress_t &);
semi::gcodes workspace_preview(const QImage &img, semi::options);
semi::gcodes finalization();
} // namespace generator

} // namespace semi
