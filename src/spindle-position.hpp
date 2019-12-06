#pragma once

#include <src/semi-gcode.hpp>

namespace engraver {
namespace helper {

struct spindle_position {
	float x{};
	float y{};

	semi::gcodes preview_gcode() noexcept {
		return is_preview_on_state ? semi::gcodes{instruction::laser_on{}, instruction::power{1}} : semi::gcodes{instruction::power{0}, instruction::laser_off{}};
	}

	semi::gcodes set_preview_on(const bool value) noexcept {
		is_preview_on_state = value;
		return preview_gcode();
	}

	constexpr instruction::move_mm move_mm_x(const float step) noexcept {
		return {x += step, y};
	}

	constexpr instruction::move_mm move_mm_y(const float step) noexcept {
		return {x, y += step};
	}

	constexpr instruction::move_mm reset_mm() noexcept {
		return {x = 0.0f, y = 0.0f};
	}

	constexpr instruction::set_home_position reset_home() noexcept {
		return {x = 0.0f, y = 0.0f};
	}

private:
	u64 is_preview_on_state{false};
};
} // namespace helper
} // namespace engraver
