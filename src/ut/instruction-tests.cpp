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

TEST(semi_generator, from_image) {
	auto progress = progress_t{};
	const auto image = create_image();
	const auto codes = semi::generator::from_image(image, {}, progress);
	EXPECT_GE(codes.size(), image.width() * image.height());
	EXPECT_DOUBLE_EQ(progress, 1.0);
}
