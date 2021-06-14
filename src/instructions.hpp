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
	constexpr power() noexcept = default;
	constexpr power(const i32 duty) noexcept
			: duty(duty) {}
	constexpr power(const power &) noexcept = default;
	constexpr power(power &&) noexcept = default;
	constexpr power &operator=(const power &) noexcept = default;
	constexpr power &operator=(power &&) noexcept = default;

	i32 duty{};

	constexpr bool operator<=>(const power &rhs) const noexcept = default;
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

	constexpr move(etype type, int feedrate = {}, power pwr = {}) noexcept
			: feedrate(feedrate)
			, type(type)
			, pwr(pwr) {}

	constexpr move(const move &) noexcept = default;
	constexpr move(move &&) noexcept = default;

	constexpr move &operator=(const move &) noexcept = default;
	constexpr move &operator=(move &&) noexcept = default;

	constexpr auto is_next_move_adaptive(const move &rhs) {
		bool is_adaptive{true};
		is_adaptive &= (y == rhs.y);
		is_adaptive &= (pwr == rhs.pwr);
		is_adaptive &= (feedrate == rhs.feedrate);
		is_adaptive &= (scale == rhs.scale);
		is_adaptive &= (type == rhs.type);
		return is_adaptive;
	}

	float x{};
	float y{};
	power pwr{0};
	int feedrate{1000};
	escale scale{escale::dpi};
	etype type{etype::precise};
};
} // namespace instruction
