#include <QApplication>

#include <clocale>
#include <iostream>

#include "mainwindow.h"

auto set_numerical_locale() -> int {
	std::setlocale(LC_NUMERIC, "C.UTF-8");

	if (const auto sample = std::to_string(1.2); sample.find('.') == std::string::npos) {
		std::cerr << "C library implementation not supporting C.UTF-8" << std::endl;
		return 1;
	}

	return 0;
}

auto main(int argc, char *argv[]) -> int {
	QApplication application(argc, argv);

	if (const auto ret = set_numerical_locale(); ret != 0)
		return ret;

	MainWindow window;
	window.showMaximized();

	return application.exec();
}
