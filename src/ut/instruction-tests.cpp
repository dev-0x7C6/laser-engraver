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
	filters.black_and_white_treshold = 1;
	EXPECT_EQ(semi::filters::black_and_white_treshold_filter(filters, 0), 0);
	EXPECT_EQ(semi::filters::black_and_white_treshold_filter(filters, 1), 255);
	EXPECT_EQ(semi::filters::black_and_white_treshold_filter(filters, 255), 255);

	for (auto i = 0; i < 255; ++i) {
		filters.black_and_white_treshold = i;
		for (auto j = 0; j < 255; ++j)
			EXPECT_EQ(semi::filters::black_and_white_treshold_filter(filters, j), j >= i ? 255 : 0);
	}
}

TEST(semi_generator, from_image) {
	auto progress = progress_t{};
	const auto image = create_image();
	const auto codes = semi::generator::from_image(image, {}, progress);
	EXPECT_GE(codes.size(), image.width() * image.height());
	EXPECT_DOUBLE_EQ(progress, 1.0);
}
