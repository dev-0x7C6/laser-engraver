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

i32 semi::calculate::power(const int color, const options &opts) noexcept {
	const auto qcolor = QColor::fromRgb(color);
	const auto f1 = filters::black_and_white_treshold_filter(opts.filters, qcolor.blackF());
	const auto pwr = std::min(opts.spindle_max_power, static_cast<int>(opts.spindle_max_power * opts.spindle_power_multiplier * f1));
	return pwr;
}

semi::gcodes semi::generator::from_image(const QImage &img, semi::options opts, progress_t &progress) {
	raii_progress progress_raii(progress);
	auto ret = initialization();

	if (img.isNull())
		return {};

	auto encode = [&ret](auto &&value) {
		ret.emplace_back(std::forward<decltype(value)>(value));
	};

	auto gcode_move = [&, offsets{center_offset(img, opts)}](
						  const std::optional<float> x,
						  const std::optional<float> y,
						  const u16 pwr,
						  const instruction::move::etype type,
						  std::optional<int> feedrate = {}) {
		const auto [x_offset, y_offset] = offsets;
		instruction::move move(type, feedrate, instruction::power(pwr));

		if (x)
			move.x = x.value() - x_offset;
		if (y)
			move.y = y.value() - y_offset;

		encode(std::move(move));
	};

	encode(instruction::move(instruction::move::etype::rapid, opts.speed.rapid));
	encode(instruction::move(instruction::move::etype::precise, opts.speed.precise));

	for (auto y = 0; y < img.height(); ++y) {
		gcode_move({}, y, 0, instruction::move::etype::rapid);
		auto schedule_power_off{false};
		for (auto x = 0; x < img.width(); ++x) {
			const auto px = ((y % 2) == 0) ? x : img.width() - x - 1;
			const auto pwr = semi::calculate::power(img.pixel(px, y), opts);

			if (strategy::dot == opts.strat) {
				if (pwr != 0) {
					gcode_move(px, {}, 0, instruction::move::etype::rapid);
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

			if (strategy::lines == opts.strat) {
				if (pwr != 0) {
					if (schedule_power_off)
						gcode_move(px, {}, pwr, instruction::move::etype::precise);
					else
						gcode_move(px, {}, 0, instruction::move::etype::rapid);
					schedule_power_off = true;
				}

				if (pwr == 0 && schedule_power_off) {
					schedule_power_off = false;
				}
			}
		}

		progress = divide(y, img.height());
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

	const auto w = static_cast<float>(img.width());
	const auto h = static_cast<float>(img.height());

	auto gcode_move = [&, offsets{center_offset(img, opts)}](const float x, const float y, const u16 pwr) {
		const auto [x_offset, y_offset] = offsets;
		instruction::move move(instruction::move::etype::rapid, opts.speed.rapid, instruction::power(pwr));
		move.x = x - x_offset;
		move.y = y - y_offset;

		encode(std::move(move));
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

float semi::filters::black_and_white_treshold_filter(const options &opts, const float in) {
	if (opts.black_and_white_treshold)
		return (opts.black_and_white_treshold.value() > in) ? 0.0f : 1.0f;
	return in;
}
