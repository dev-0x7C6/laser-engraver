#pragma once

#include <externals/common/types.hpp>
#include <optional>

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

struct move {
	constexpr move() noexcept = default;
	constexpr move(float x, float y) noexcept
			: x(x)
			, y(y) {
	}

	constexpr move(float x, float y, bool scale) noexcept
			: x(x)
			, y(y)
			, scale(scale){};

	constexpr move(const move &) noexcept = default;
	constexpr move(move &&) noexcept = default;

	std::optional<float> x;
	std::optional<float> y;
	std::optional<float> feedrate;
	std::optional<float> power;
	bool scale{true};
};

struct move_fast : public move {
	using move::move;
};

struct power {
	i32 duty;
};
} // namespace instruction
