#pragma once

#include <src/engraver-settings.h>
#include <src/gcode-generator.hpp>

class EngraverConnection {
public:
	EngraverConnection(const EngraverSettings &configuration);
	bool isOpen() const noexcept;
	upload_instruction process();

private:
	QSerialPort m_port;
};
