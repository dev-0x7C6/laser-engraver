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

TEST(semi_generator, from_image) {
	auto progress = progress_t{};
	const auto image = create_image();
	const auto codes = semi::generator::from_image(image, {}, progress);
	EXPECT_GE(codes.size(), image.width() * image.height());
	EXPECT_DOUBLE_EQ(progress, 1.0);
}
