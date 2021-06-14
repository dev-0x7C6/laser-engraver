#include <gtest/gtest.h>

#include <src/gcode-generator.hpp>
#include <src/instructions.hpp>
#include <src/semi-gcode.hpp>

#include <QImage>
#include <QPixmap>

auto create_image() -> QImage {
	QPixmap ret(256, 256);
	ret.fill(Qt::black);
	return ret.toImage();
}

TEST(qt_colors, make_sure) {
	QColor color(Qt::black);
	EXPECT_EQ(color.black(), 255);
}

TEST(semi_filters, black_and_white_filter) {
	semi::filters::options filters;
	filters.black_and_white_treshold = 0.01f;
	EXPECT_DOUBLE_EQ(semi::filters::black_and_white_treshold_filter(filters, 0.0f), 0.0f);
	EXPECT_DOUBLE_EQ(semi::filters::black_and_white_treshold_filter(filters, 0.01f), 1.0f);
	EXPECT_DOUBLE_EQ(semi::filters::black_and_white_treshold_filter(filters, 1.0f), 1.0f);
}

TEST(calculations, power) {
	semi::options opts;
	opts.spindle_max_power = 1000;
	opts.spindle_power_multiplier = 1.0f;

	EXPECT_DOUBLE_EQ(semi::calculate::power(QColor(Qt::black).rgb(), opts), 1000);
	opts.spindle_power_multiplier = 0.5f;

	EXPECT_DOUBLE_EQ(semi::calculate::power(QColor(Qt::black).rgb(), opts), 500);
	opts.spindle_power_multiplier = 0.1f;

	EXPECT_DOUBLE_EQ(semi::calculate::power(QColor(Qt::black).rgb(), opts), 100);
	opts.spindle_power_multiplier = 0.001f;

	EXPECT_DOUBLE_EQ(semi::calculate::power(QColor(Qt::black).rgb(), opts), 1);
}

TEST(semi_move_instruction, adaptive) {
	instruction::move begin;
	instruction::move end;

	auto is_adaptive = [&]() { return begin.is_next_move_adaptive(end); };

	EXPECT_TRUE(is_adaptive());

	begin.feedrate = 3000;
	begin.pwr = 3000;
	end.feedrate = 3000;
	end.pwr = 3000;

	EXPECT_TRUE(is_adaptive());

	begin.feedrate = 3000;
	begin.pwr = 3000;
	end.feedrate = 3001;
	end.pwr = 3000;

	EXPECT_FALSE(is_adaptive());

	begin.feedrate = 3000;
	begin.pwr = 3000;
	end.feedrate = 3000;
	end.pwr = 3001;

	EXPECT_FALSE(is_adaptive());
}

TEST(from_image, three_points) {
	semi::options opts;
	opts.strat = semi::strategy::lines;
	progress_t progress;
	QPixmap data(100, 1);
	data.fill(Qt::white);
	QImage line = data.toImage();
	line.setPixelColor(0, 0, Qt::black);
	line.setPixelColor(50, 0, Qt::black);
	line.setPixelColor(99, 0, Qt::black);
	auto ret = semi::generator::from_image(line, opts, progress);

	gcode::generator::grbl gen;
	auto cnt = std::count_if(ret.begin(), ret.end(), [&](auto &&v) {
		if (std::holds_alternative<instruction::move>(v)) {
			auto move = std::get<instruction::move>(v);
			std::cout << std::visit(gen, v) << std::endl;
			return move.pwr.duty == 255;
		}

		return false;
	});

	EXPECT_EQ(cnt, 3);
}

TEST(from_image, linear_gradient) {
	semi::options opts;
	opts.strat = semi::strategy::lines;
	progress_t progress;
	QPixmap data(256, 1);
	data.fill(Qt::white);
	QImage line = data.toImage();
	for (int i = 0; i < 256; ++i)
		line.setPixelColor(i, 0, QColor::fromRgb(i, i, i));
	auto ret = semi::generator::from_image(line, opts, progress);

	gcode::generator::grbl gen;
	auto cnt = std::count_if(ret.begin(), ret.end(), [&](auto &&v) {
		if (std::holds_alternative<instruction::move>(v)) {
			auto move = std::get<instruction::move>(v);
			std::cout << std::visit(gen, v) << std::endl;
			return move.pwr.duty != 0;
		}

		return false;
	});

	EXPECT_EQ(cnt, 255);
}

TEST(semi_generator, from_image) {
	auto progress = progress_t{};
	const auto image = create_image();
	const auto codes = semi::generator::from_image(image, {}, progress);
	EXPECT_GE(codes.size(), image.width() * image.height());
	EXPECT_DOUBLE_EQ(progress, 1.0);
}
