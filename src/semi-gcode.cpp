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

auto center_offset(const QImage &img, const options &opts) {
	return std::make_pair(opts.center_object ? img.width() / 2 : 0,
		opts.center_object ? img.height() / 2 : 0);
}

void move_gcodes(semi_gcodes &&source, semi_gcodes &destination) {
	std::move(source.begin(), source.end(), std::back_inserter(destination));
}

} // namespace

semi_gcodes image_to_semi_gcode(const QImage &img, options opts, progress_t &progress) {
	raii_progress progress_raii(progress);
	auto ret = generate_begin_section();

	if (img.isNull())
		return {};

	auto encode = [&ret](auto &&value) {
		ret.emplace_back(std::forward<decltype(value)>(value));
	};

	auto schedule_power_off{false};

	auto gcode_move = [&, offsets{center_offset(img, opts)}](const i32 x, const i32 y) {
		const auto [x_offset, y_offset] = offsets;
		encode(move{x - x_offset, y - y_offset});
	};

	for (auto y = 0; y < img.height(); ++y) {
		for (auto px = 0; px < img.width(); ++px) {
			const auto x = ((y % 2) == 0) ? px : img.width() - px - 1;
			const auto pwr = 1.0 - QColor::fromRgb(img.pixel(x, y)).lightnessF();

			if (pwr != 0.0) {
				gcode_move(x, y);
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

	move_gcodes(generate_end_section(), ret);
	return ret;
}

semi_gcodes generate_workspace_demo(const QImage &img, options opts) {
	auto ret = generate_begin_section();

	if (img.isNull())
		return {};

	auto encode = [&ret](auto &&value) {
		ret.emplace_back(std::forward<decltype(value)>(value));
	};

	const auto w = static_cast<i16>(img.width());
	const auto h = static_cast<i16>(img.height());

	auto gcode_move = [&, offsets{center_offset(img, opts)}](const i32 x, const i32 y) {
		const auto [x_offset, y_offset] = offsets;
		encode(move{(x - x_offset), (y - y_offset)});
		encode(wait_for_movement_finish{});
	};

	gcode_move(0, 0);
	encode(power{1});
	gcode_move(w, 0);
	gcode_move(w, h);
	gcode_move(0, h);
	gcode_move(0, 0);

	move_gcodes(generate_end_section(), ret);
	return ret;
}

semi_gcodes generate_begin_section() {
	return {power{0}, home{}, wait_for_movement_finish{}, laser_on{}};
};

semi_gcodes generate_end_section() {
	return {power{0}, home{}, wait_for_movement_finish{}, laser_off{}};
};
