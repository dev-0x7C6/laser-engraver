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

struct power {
	constexpr power() = default;
	constexpr power(const i32 duty) noexcept
			: duty(duty) {}
	i32 duty{};
};

struct move {
	enum class escale {
		dpi,
		off,
	};

	enum class etype {
		precise,
		rapid,
	};

	constexpr move() noexcept = default;
	constexpr move(float x, float y) noexcept
			: x(x)
			, y(y) {
	}

	constexpr move(float x, float y, escale type) noexcept
			: x(x)
			, y(y)
			, scale(type){};

	constexpr move(etype type, std::optional<int> feedrate = {}, std::optional<power> pwr = {}) noexcept
			: feedrate(feedrate)
			, type(type)
			, pwr(pwr) {}

	constexpr move(const move &) noexcept = default;
	constexpr move(move &&) noexcept = default;

	std::optional<float> x;
	std::optional<float> y;
	std::optional<power> pwr;
	std::optional<int> feedrate;
	escale scale{escale::dpi};
	etype type{etype::precise};
};
} // namespace instruction
