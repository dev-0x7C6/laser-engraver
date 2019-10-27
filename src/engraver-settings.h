#pragma once

#include <QSerialPort>
#include <QSettings>

#include <vector>

enum class GcodeFlavor {
	Grbl
};

namespace engraver {
namespace settings {
struct movement_parameters {
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

struct serial_parameters {
	QString name;
	QString port;
	QSerialPort::DataBits bits{QSerialPort::DataBits::Data8};
	QSerialPort::BaudRate baud{QSerialPort::BaudRate::Baud115200};
	QSerialPort::Parity parity{QSerialPort::Parity::NoParity};
	QSerialPort::FlowControl flow_control{QSerialPort::FlowControl::NoFlowControl};
	QSerialPort::StopBits stop_bits{QSerialPort::StopBits::OneStop};
};

struct configuration {
	QString name;
	engraver::settings::serial_parameters serial_params;
	engraver::settings::movement_parameters movement_params;
};

inline auto load(QSettings &handle) {
	configuration ret;
	ret.name = handle.value("name").toString();
	ret.serial_params.port = handle.value("port").toString();
	ret.serial_params.bits = static_cast<QSerialPort::DataBits>(handle.value("bits", QSerialPort::DataBits::Data8).toULongLong());
	ret.serial_params.baud = static_cast<QSerialPort::BaudRate>(handle.value("baud", QSerialPort::BaudRate::Baud115200).toULongLong());
	ret.serial_params.parity = static_cast<QSerialPort::Parity>(handle.value("parity", QSerialPort::Parity::NoParity).toULongLong());
	ret.serial_params.flow_control = static_cast<QSerialPort::FlowControl>(handle.value("flow_control", QSerialPort::FlowControl::NoFlowControl).toULongLong());
	ret.serial_params.stop_bits = static_cast<QSerialPort::StopBits>(handle.value("stop_bits", QSerialPort::StopBits::OneStop).toULongLong());

	engraver::settings::movement_parameters default_movement_params;
	ret.movement_params.x_acceleration = handle.value("x_acceleration", default_movement_params.x_acceleration).toDouble();
	ret.movement_params.x_max_speed = handle.value("x_max_speed", default_movement_params.x_max_speed).toDouble();
	ret.movement_params.x_steps_per_mm = handle.value("x_steps_per_mm", default_movement_params.x_steps_per_mm).toDouble();

	ret.movement_params.y_acceleration = handle.value("y_acceleration", default_movement_params.y_acceleration).toDouble();
	ret.movement_params.y_max_speed = handle.value("y_max_speed", default_movement_params.y_max_speed).toDouble();
	ret.movement_params.y_steps_per_mm = handle.value("y_steps_per_mm", default_movement_params.y_steps_per_mm).toDouble();

	ret.movement_params.z_acceleration = handle.value("z_acceleration", default_movement_params.z_acceleration).toDouble();
	ret.movement_params.z_max_speed = handle.value("z_max_speed", default_movement_params.z_max_speed).toDouble();
	ret.movement_params.z_steps_per_mm = handle.value("z_steps_per_mm", default_movement_params.z_steps_per_mm).toDouble();
	return ret;
}

inline void save(QSettings &handle, configuration &settings) {
	handle.setValue("name", settings.name);
	handle.setValue("port", settings.serial_params.port);
	handle.setValue("bits", settings.serial_params.bits);
	handle.setValue("baud", settings.serial_params.baud);
	handle.setValue("parity", settings.serial_params.parity);
	handle.setValue("flow_control", settings.serial_params.flow_control);
	handle.setValue("stop_bits", settings.serial_params.stop_bits);

	handle.setValue("x_acceleration", settings.movement_params.x_acceleration);
	handle.setValue("x_max_speed", settings.movement_params.x_max_speed);
	handle.setValue("x_steps_per_mm", settings.movement_params.x_steps_per_mm);

	handle.setValue("y_acceleration", settings.movement_params.y_acceleration);
	handle.setValue("y_max_speed", settings.movement_params.y_max_speed);
	handle.setValue("y_steps_per_mm", settings.movement_params.y_steps_per_mm);

	handle.setValue("z_acceleration", settings.movement_params.z_acceleration);
	handle.setValue("z_max_speed", settings.movement_params.z_max_speed);
	handle.setValue("z_steps_per_mm", settings.movement_params.z_steps_per_mm);
}

using configurations = std::vector<configuration>;

} // namespace settings
} // namespace engraver
