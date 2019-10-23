#pragma once

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>

#include <vector>

enum class GcodeFlavor {
	Grbl
};

struct EngraverParameters {
	double x_steps_per_mm{80.00};
	double y_steps_per_mm{80.00};
	double z_steps_per_mm{80.00};
	double x_max_speed{4000.00};
	double y_max_speed{4000.00};
	double z_max_speed{4000.00};
	double x_acceleration{200.0};
	double y_acceleration{200.0};
	double z_acceleration{200.0};
};

struct EngraverSettings {
	QString name;
	QString port;
	QSerialPort::DataBits bits{QSerialPort::DataBits::Data8};
	QSerialPort::BaudRate baud{QSerialPort::BaudRate::Baud115200};
	QSerialPort::Parity parity{QSerialPort::Parity::NoParity};
	QSerialPort::FlowControl flow_control{QSerialPort::FlowControl::NoFlowControl};
	QSerialPort::StopBits stop_bits{QSerialPort::StopBits::OneStop};
	EngraverParameters params;
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

	EngraverParameters default_params;
	ret.params.x_acceleration = handle.value("x_acceleration", default_params.x_acceleration).toDouble();
	ret.params.x_max_speed = handle.value("x_max_speed", default_params.x_max_speed).toDouble();
	ret.params.x_steps_per_mm = handle.value("x_steps_per_mm", default_params.x_steps_per_mm).toDouble();

	ret.params.y_acceleration = handle.value("y_acceleration", default_params.y_acceleration).toDouble();
	ret.params.y_max_speed = handle.value("y_max_speed", default_params.y_max_speed).toDouble();
	ret.params.y_steps_per_mm = handle.value("y_steps_per_mm", default_params.y_steps_per_mm).toDouble();

	ret.params.z_acceleration = handle.value("z_acceleration", default_params.z_acceleration).toDouble();
	ret.params.z_max_speed = handle.value("z_max_speed", default_params.z_max_speed).toDouble();
	ret.params.z_steps_per_mm = handle.value("z_steps_per_mm", default_params.z_steps_per_mm).toDouble();
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

	handle.setValue("x_acceleration", settings.params.x_acceleration);
	handle.setValue("x_max_speed", settings.params.x_max_speed);
	handle.setValue("x_steps_per_mm", settings.params.x_steps_per_mm);

	handle.setValue("y_acceleration", settings.params.y_acceleration);
	handle.setValue("y_max_speed", settings.params.y_max_speed);
	handle.setValue("y_steps_per_mm", settings.params.y_steps_per_mm);

	handle.setValue("z_acceleration", settings.params.z_acceleration);
	handle.setValue("z_max_speed", settings.params.z_max_speed);
	handle.setValue("z_steps_per_mm", settings.params.z_steps_per_mm);
}

using Engravers = std::vector<EngraverSettings>;
