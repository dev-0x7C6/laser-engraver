#include "qt-wrappers.h"

#include <QProgressDialog>
#include <QString>
#include <QTimer>

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
