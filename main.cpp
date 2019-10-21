#include <QApplication>
#include <QtSerialPort>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
	qRegisterMetaType<QSerialPort::DataBits>("DataBits");
	QApplication application(argc, argv);
	MainWindow window;
	window.show();

	return application.exec();
}
