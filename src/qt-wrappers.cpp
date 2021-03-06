#include "qt-wrappers.h"

#include <QProgressDialog>
#include <QString>
#include <QTimer>

#include <src/semi-gcode.hpp>

using namespace std::chrono_literals;

void qt_generate_progress_dialog(QString &&title, progress_t &progress) {
	QProgressDialog dialog;
	QTimer timer;
	QObject::connect(&timer, &QTimer::timeout, &dialog, [&dialog, &progress]() {
		dialog.setValue(static_cast<int>(progress * 1000.0));
	});

	dialog.setLabelText(title);
	dialog.setMinimum(0);
	dialog.setMaximum(1000);
	dialog.setCancelButton(nullptr);
	timer.start(5ms);
	dialog.exec();
}

auto semi::generator::qt::from_image(const QImage &image, semi::options opts) -> semi::gcodes {
	return qt_progress_task<semi::gcodes>(QObject::tr("Generating semi-gcode for post processing"), [&image, opts{std::move(opts)}](progress_t &progress) {
		return semi::generator::from_image(image, opts, progress);
	});
}
