#pragma once

#include <externals/common/types.hpp>

#include <atomic>
#include <variant>
#include <vector>

struct image {
	constexpr image(const std::size_t w, const std::size_t h, const u8 *data)
			: w(w)
			, h(h)
			, data(data) {
	}

	std::size_t w{};
	std::size_t h{};
	const u8 *data;

	constexpr auto count() const noexcept {
		return w * h;
	}
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

semi_gcodes image_to_semi_gcode(image, progress_t &);
