#include "engraver-connection.h"

#include <QApplication>
#include <QElapsedTimer>
#include <iostream>

using namespace std::chrono_literals;

EngraverConnection::EngraverConnection(const engraver::settings::configuration &settings)
		: m_port(settings.serial_params.port)
		, m_name(settings.name) {
	m_port.setBaudRate(settings.serial_params.baud);
	m_port.setParity(settings.serial_params.parity);
	m_port.setDataBits(settings.serial_params.bits);
	m_port.setFlowControl(settings.serial_params.flow_control);
	m_port.setStopBits(settings.serial_params.stop_bits);

	if (m_port.open(QSerialPort::ReadWrite)) {
		m_port.clear();
		if (!updateEngraverParameters(settings.movement_params))
			m_port.close();
	}
}

bool EngraverConnection::isOpen() const noexcept { return m_port.isOpen(); }

auto EngraverConnection::process(std::chrono::milliseconds timeout) -> upload_instruction {
	return [this, timeout](std::string &&instruction, double) -> upload_instruction_ret {
		emit gcodeSended(QString::fromStdString(instruction));
		instruction += '\n';
		m_port.write(instruction.c_str(), static_cast<i64>(instruction.size()));
		m_port.waitForBytesWritten();

		auto timer = QElapsedTimer{};
		timer.start();

		for (;;) {
			m_port.waitForReadyRead(1);
			QApplication::processEvents(QEventLoop::AllEvents, 1);

			if (const auto response = m_port.readLine(); !response.isEmpty()) {
				emit gcodeReceived(QString::fromUtf8(response));
				break;
			}

			if (timer.hasExpired(timeout.count()))
				return upload_instruction_ret::timeout;
		}

		return upload_instruction_ret::keep_going;
	};
}

void EngraverConnection::process_safe_gcode() {
	gcode::transform(semi::generator::finalization(), {}, process());
}

bool EngraverConnection::updateEngraverParameters(const engraver::settings::movement_parameters &parameters) {
	const auto call = process(3s);

	auto instructions = std::vector<std::string>{
		{"$100=" + std::to_string(parameters.x_steps_per_mm)},
		{"$100=" + std::to_string(parameters.x_steps_per_mm)},
		{"$101=" + std::to_string(parameters.y_steps_per_mm)},
		{"$102=" + std::to_string(parameters.z_steps_per_mm)},
		{"$110=" + std::to_string(parameters.x_max_speed)},
		{"$111=" + std::to_string(parameters.y_max_speed)},
		{"$112=" + std::to_string(parameters.z_max_speed)},
		{"$120=" + std::to_string(parameters.x_acceleration)},
		{"$121=" + std::to_string(parameters.y_acceleration)},
		{"$122=" + std::to_string(parameters.z_acceleration)},
	};

	for (auto &&instruction : instructions) {
		switch (call(std::string(instruction), {})) {
			case upload_instruction_ret::cancel:
				return false;
			case upload_instruction_ret::timeout:
				return false;
			case upload_instruction_ret::keep_going:
				continue;
		}
	}

	return true;
}
