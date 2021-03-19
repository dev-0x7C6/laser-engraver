#include <benchmark/benchmark.h>

#include <src/gcode-generator.hpp>
#include <src/semi-gcode.hpp>

#include <QApplication>
#include <QImage>
#include <QPixmap>

auto create_image() -> QImage {
	QPixmap ret(256, 256);
	ret.fill(Qt::black);
	return ret.toImage();
}

static void generate_semi_gcode_from_image(benchmark::State &state) {
	const auto image = create_image();
	progress_t progress{};
	while (state.KeepRunning()) {
		auto ret = semi::generator::from_image(image, {}, progress);
		benchmark::DoNotOptimize(ret);
	}
}

BENCHMARK(generate_semi_gcode_from_image);

auto main(int argc, char **argv) -> int {
	QApplication app(argc, argv);
	::benchmark::Initialize(&argc, argv);
	::benchmark::RunSpecifiedBenchmarks();
	return 0;
}
