#pragma once

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>

#include <vector>

enum class GcodeFlavor {
	Grbl
};

struct EngraverSettings {
	QString name;
	QString port;
	QSerialPort::DataBits bits{QSerialPort::DataBits::Data8};
	QSerialPort::BaudRate baud{QSerialPort::BaudRate::Baud115200};
	QSerialPort::Parity parity{QSerialPort::Parity::NoParity};
	QSerialPort::FlowControl flow_control{QSerialPort::FlowControl::NoFlowControl};
	QSerialPort::StopBits stop_bits{QSerialPort::StopBits::OneStop};
};

inline auto load(QSettings &handle) {
	EngraverSettings ret;
	ret.name = handle.value("name").toString();
	ret.port = handle.value("port").toString();
	ret.bits = static_cast<QSerialPort::DataBits>(handle.value("bits", QSerialPort::DataBits::Data8).toULongLong());
	ret.baud = static_cast<QSerialPort::BaudRate>(handle.value("baud", QSerialPort::BaudRate::Baud115200).toULongLong());
	ret.parity = static_cast<QSerialPort::Parity>(handle.value("parity", QSerialPort::Parity::NoParity).toULongLong());
	ret.flow_control = static_cast<QSerialPort::FlowControl>(handle.value("flow_control", QSerialPort::FlowControl::NoFlowControl).toULongLong());
	ret.stop_bits = static_cast<QSerialPort::StopBits>(handle.value("stop_bits", QSerialPort::StopBits::OneStop).toULongLong());
	return ret;
}

inline void save(QSettings &handle, EngraverSettings &settings) {
	EngraverSettings ret;
	handle.setValue("name", settings.name);
	handle.setValue("port", settings.port);
	handle.setValue("bits", settings.bits);
	handle.setValue("baud", settings.baud);
	handle.setValue("parity", settings.parity);
	handle.setValue("flow_control", settings.flow_control);
	handle.setValue("stop_bits", settings.stop_bits);
}

using Engravers = std::vector<EngraverSettings>;
