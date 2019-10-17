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

template <typename gcode_begin, typename gcode_end = gcode_begin>
class raii_gcode {
public:
	raii_gcode(semi_gcodes &gcodes, gcode_begin &&begin, gcode_end &&end = gcode_begin{})
			: m_gcodes(gcodes)
			, m_end(std::move(end)) {
		m_gcodes.emplace_back(begin);
	}

	~raii_gcode() {
		m_gcodes.emplace_back(std::move(m_end));
	}

private:
	semi_gcodes &m_gcodes;
	gcode_end m_end;
};
} // namespace

semi_gcodes image_to_semi_gcode(const QImage &img, options opts, progress_t &progress) {
	raii_progress progress_raii(progress);
	semi_gcodes ret;

	if (img.isNull())
		return ret;

	constexpr static auto ir_size = sizeof(semi_gcode);
	constexpr static auto ir_extra = ir_size * 100;

	ret.reserve(ir_size * img.width() * img.height() + ir_extra);

	auto encode = [&ret](auto &&value) {
		ret.emplace_back(std::forward<decltype(value)>(value));
	};

	raii_gcode home_raii(ret, home{});
	raii_gcode power_raii(ret, power{0});
	raii_gcode laser_raii(ret, laser_on{}, laser_off{});

	auto schedule_power_off{false};

	for (std::size_t y = 0; y < img.height(); ++y) {
		for (std::size_t px = 0; px < img.width(); ++px) {
			const auto x = ((y % 2) == 0) ? px : img.width() - px - 1;
			const auto pwr = 1.0 - QColor::fromRgb(img.pixel(x, y)).lightnessF();

			if (pwr != 0.0) {
				encode(move{static_cast<decltype(move::x)>(x), static_cast<decltype(move::y)>(y)});
				encode(power{static_cast<i16>(255 * (pwr * opts.power_multiplier))});
				if (opts.force_dwell_time)
					encode(dwell{opts.force_dwell_time.value()});
				schedule_power_off = true;
			} else {
				if (schedule_power_off) {
					encode(power{0});
					schedule_power_off = false;
				}
			}
		}
		encode(power{0});

		progress = static_cast<double>(y) / static_cast<double>(img.height());
	}

	return ret;
}

semi_gcodes show_workspace(const QImage &img, int times) {
	semi_gcodes ret;

	if (img.isNull())
		return ret;

	auto encode = [&ret](auto &&value) {
		ret.emplace_back(std::forward<decltype(value)>(value));
	};

	raii_gcode home_raii(ret, home{});
	raii_gcode power_raii(ret, power{0});
	raii_gcode laser_raii(ret, laser_on{}, laser_off{});

	encode(power{1});

	const auto w = static_cast<i16>(img.width());
	const auto h = static_cast<i16>(img.height());

	for (auto i = 0; i < times; ++i) {
		encode(power{1});
		encode(move{0, 0});
		encode(move{0, w});
		encode(move{h, w});
		encode(move{h, 0});
		encode(move{0, 0});
	}

	return ret;
}
