#include <QApplication>

#include <clocale>
#include <iostream>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
	// use dot separator for gcode generation
	std::setlocale(LC_NUMERIC, "C.UTF-8");

	if (auto sample = std::to_string(1.2); sample.find('.') == std::string::npos) {
		std::cerr << "C library implementation not supporting C.UTF-8" << std::endl;
		return 1;
	}

	QApplication application(argc, argv);
	MainWindow window;
	window.showMaximized();

	return application.exec();
}
