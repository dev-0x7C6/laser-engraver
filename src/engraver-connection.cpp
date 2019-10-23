#include "engraver-connection.h"

#include <QApplication>
#include <iostream>

EngraverConnection::EngraverConnection(const EngraverSettings &settings)
		: m_port(settings.port)
		, m_name(settings.name) {
	m_port.setBaudRate(settings.baud);
	m_port.setParity(settings.parity);
	m_port.setDataBits(settings.bits);
	m_port.setFlowControl(settings.flow_control);
	m_port.setStopBits(settings.stop_bits);

	if (m_port.open(QSerialPort::ReadWrite)) {
		m_port.clear();
		for (auto i = 0; i < 3000; ++i) {
			const auto response = m_port.readLine();
			QApplication::processEvents(QEventLoop::AllEvents, 1);
			if (!response.isEmpty())
				std::cout << response.toStdString() << std::endl;
			m_port.waitForReadyRead(1);
		}
		m_port.clear();

		updateEngraverParameters(settings.params);
	}
}

bool EngraverConnection::isOpen() const noexcept { return m_port.isOpen(); }

upload_instruction EngraverConnection::process() {
	return [this](std::string &&instruction, double) -> upload_instruction_ret {
		std::cout << "GCODE: " << instruction << std::endl;
		instruction += "\n";
		m_port.write(instruction.c_str(), instruction.size());
		m_port.waitForBytesWritten();

		for (int retry = 0; retry < 30000; ++retry) {
			m_port.waitForReadyRead(1);
			QApplication::processEvents(QEventLoop::AllEvents, 1);
			const auto response = m_port.readLine();
			if (!response.isEmpty()) {
				std::cout << response.toStdString() << std::endl;
				break;
			}
		}

		return upload_instruction_ret::keep_going;
	};
}

void EngraverConnection::updateEngraverParameters(const EngraverParameters &parameters) {
	const auto callable = process();
	callable("$100=" + std::to_string(parameters.x_steps_per_mm), {});
	callable("$100=" + std::to_string(parameters.x_steps_per_mm), {});
	callable("$101=" + std::to_string(parameters.y_steps_per_mm), {});
	callable("$102=" + std::to_string(parameters.z_steps_per_mm), {});
	callable("$110=" + std::to_string(parameters.x_max_speed), {});
	callable("$111=" + std::to_string(parameters.y_max_speed), {});
	callable("$112=" + std::to_string(parameters.z_max_speed), {});
	callable("$120=" + std::to_string(parameters.x_acceleration), {});
	callable("$121=" + std::to_string(parameters.y_acceleration), {});
	callable("$122=" + std::to_string(parameters.z_acceleration), {});
}
