#include "semi-gcode.hpp"

semi_gcodes image_to_semi_gcode(const u8 *data, std::size_t w, std::size_t h, progress_t &progress) {
	progress = 0.0;

	semi_gcodes ret;

	constexpr static auto ir_size = sizeof(semi_gcode);
	constexpr static auto ir_extra = ir_size * 100;

	ret.reserve(ir_size * w * h + ir_extra);

	auto encode = [&ret](auto &&value) {
		ret.emplace_back(std::forward<decltype(value)>(value));
	};

	encode(home{});
	encode(power{0});
	encode(laser_on{});

	std::size_t index{0};

	for (std::size_t y = 0; y < h; ++y) {
		for (std::size_t x = 0; x < w; ++x) {
			const auto r = *(++data);
			const auto g = *(++data);
			const auto b = *(++data);

			encode(move{static_cast<decltype(move::x)>(x), static_cast<decltype(move::y)>(y)});
			encode(power{static_cast<i16>(1000 - r - g - b)});
			encode(dwell{1});
			encode(power{0});
		}

		progress = static_cast<double>(y) / static_cast<double>(h);
	}

	encode(power{0});
	encode(laser_off{});
	encode(home{});
	progress = 1.0;

	return ret;
}
