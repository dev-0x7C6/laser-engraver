#include "engraver-connection.h"

#include <QApplication>
#include <iostream>

EngraverConnection::EngraverConnection(const EngraverSettings &configuration)
		: m_port(configuration.port) {
	m_port.setBaudRate(configuration.baud);
	m_port.setParity(configuration.parity);
	m_port.setDataBits(configuration.bits);
	m_port.setFlowControl(configuration.flow_control);
	m_port.setStopBits(configuration.stop_bits);

	if (m_port.open(QSerialPort::ReadWrite)) {
		m_port.clear();
		for (auto i = 0; i < 3000; ++i) {
			const auto response = m_port.readLine();
			if (!response.isEmpty())
				std::cout << response.toStdString() << std::endl;
			m_port.waitForReadyRead(1);
		}
		m_port.clear();
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
