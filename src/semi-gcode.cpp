#include "semi-gcode.hpp"

namespace {
template <typename type>
class raii_progress {
public:
	raii_progress(type &progress)
			: m_progress(progress) {
		m_progress = 0.0;
	}

	~raii_progress() {
		m_progress = 1.0;
	}

private:
	type &m_progress;
};
} // namespace

semi_gcodes image_to_semi_gcode(image img, progress_t &progress) {
	raii_progress _(progress);
	semi_gcodes ret;

	constexpr static auto ir_size = sizeof(semi_gcode);
	constexpr static auto ir_extra = ir_size * 100;

	ret.reserve(ir_size * img.count() + ir_extra);

	auto encode = [&ret](auto &&value) {
		ret.emplace_back(std::forward<decltype(value)>(value));
	};

	encode(home{});
	encode(power{0});
	encode(laser_on{});

	auto data = img.data;

	for (std::size_t y = 0; y < img.h; ++y) {
		for (std::size_t x = 0; x < img.w; ++x) {
			const auto r = *(++data);
			const auto g = *(++data);
			const auto b = *(++data);

			encode(move{static_cast<decltype(move::x)>(x), static_cast<decltype(move::y)>(y)});
			encode(power{static_cast<i16>(1000 - r - g - b)});
			encode(dwell{1});
			encode(power{0});
		}

		progress = static_cast<double>(y) / static_cast<double>(img.h);
	}

	encode(power{0});
	encode(laser_off{});
	encode(home{});
	return ret;
}
