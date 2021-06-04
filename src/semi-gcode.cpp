#include "semi-gcode.hpp"

#include <QImage>

#include <src/utils.hpp>

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

auto center_offset(const QImage &img, const semi::options &opts) {
	return std::make_pair(static_cast<float>(opts.center_object ? img.width() / 2 : 0),
		static_cast<float>(opts.center_object ? img.height() / 2 : 0));
}

void move_gcodes(semi::gcodes &&source, semi::gcodes &destination) {
	std::move(source.begin(), source.end(), std::back_inserter(destination));
}

semi::gcodes initialization() {
	return {instruction::power{0}, instruction::home{}, instruction::wait_for_movement_finish{}, instruction::laser_on{}};
}
} // namespace

semi::gcodes semi::generator::from_image(const QImage &img, semi::options opts, progress_t &progress) {
	raii_progress progress_raii(progress);
	auto ret = initialization();

	if (img.isNull())
		return {};

	auto encode = [&ret](auto &&value) {
		ret.emplace_back(std::forward<decltype(value)>(value));
	};

	auto schedule_power_off{false};

	auto gcode_move = [&, offsets{center_offset(img, opts)}](const std::optional<float> x, const std::optional<float> y, const u16 pwr) {
		const auto [x_offset, y_offset] = offsets;
		encode(instruction::move_dpi{
			x ? std::optional<float>(x.value() - x_offset) : std::nullopt,
			y ? std::optional<float>(y.value() - y_offset) : std::nullopt,
			pwr});
	};

	for (auto y = 0; y < img.height(); ++y) {
		gcode_move({}, y, 0);
		for (auto x = 0; x < img.width(); ++x) {
			const auto px = ((y % 2) == 0) ? x : img.width() - x - 1;
			const auto color = QColor::fromRgb(img.pixel(px, y));
			const auto pwr = std::min(255, static_cast<int>(color.black() * opts.power_multiplier));

			if (pwr != 0) {
				gcode_move(px, {}, 0);
				encode(instruction::power{pwr});
				if (opts.force_dwell_time)
					encode(instruction::dwell{opts.force_dwell_time.value()});
				schedule_power_off = true;
			} else {
				if (schedule_power_off) {
					encode(instruction::power{0});
					schedule_power_off = false;
				}
			}
		}
		encode(instruction::power{0});

		progress = divide(y, img.height());
	}

	move_gcodes(finalization(), ret);
	return ret;
}

struct coordinate {
	std::optional<float> x;
	std::optional<float> y;
};

struct coordinates {
	coordinate current;
	coordinate begin_move;
};

semi::gcodes semi::generator::optimize_treshold_max(semi::gcodes &&gcodes) {
	coordinates coordinate;
	std::optional<i32> pwr;
	auto laser_on_state{false};

	auto ret = initialization();

	for (auto &&gcode : gcodes) {
		if (std::holds_alternative<instruction::power>(gcode))
			pwr = std::get<instruction::power>(gcode).duty;

		if (std::holds_alternative<instruction::move_dpi>(gcode)) {
			const auto value = std::get<instruction::move_dpi>(gcode);
			if (value.x.has_value())
				coordinate.current.x = value.x.value();

			if (value.y.has_value())
				coordinate.current.y = value.y.value();

			//if (value.power.has_value())
				//pwr = value.power;
		}

		if (pwr.value_or(0) > 0) {
			if (!coordinate.begin_move.x.has_value())
				coordinate.begin_move.x = coordinate.current.x;

			if (!coordinate.begin_move.y.has_value())
				coordinate.begin_move.y = coordinate.current.y;
		} else {
			if (coordinate.begin_move.x.has_value() &&
				coordinate.begin_move.y.has_value()) {
				ret.emplace_back(instruction::move_dpi{coordinate.begin_move.x.value(),
					coordinate.begin_move.y.value(), 0});
				ret.emplace_back(instruction::move_dpi{coordinate.current.x.value(),
					coordinate.current.y.value(), 255});
				coordinate.begin_move.x.reset();
				coordinate.begin_move.y.reset();
			}
		}
	}

	move_gcodes(finalization(), ret);
	return ret;
}

semi::gcodes semi::generator::workspace_preview(const QImage &img, semi::options opts) {
	auto ret = initialization();

	if (img.isNull())
		return {};

	auto encode = [&ret](auto &&value) {
		ret.emplace_back(std::forward<decltype(value)>(value));
	};

	const auto w = static_cast<i16>(img.width());
	const auto h = static_cast<i16>(img.height());

	auto gcode_move = [&, offsets{center_offset(img, opts)}](const float x, const float y, const u16 pwr) {
		const auto [x_offset, y_offset] = offsets;
		encode(instruction::move_dpi{(x - x_offset), (y - y_offset), pwr});
		encode(instruction::wait_for_movement_finish{});
	};

	gcode_move(0, 0, 0);
	gcode_move(w, 0, 1);
	gcode_move(w, h, 1);
	gcode_move(0, h, 1);
	gcode_move(0, 0, 1);

	move_gcodes(finalization(), ret);
	return ret;
}

semi::gcodes semi::generator::finalization() {
	return {instruction::power{0}, instruction::home{}, instruction::wait_for_movement_finish{}, instruction::laser_off{}};
}
