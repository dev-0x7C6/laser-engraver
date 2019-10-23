#pragma once

#include <src/engraver-settings.h>
#include <src/gcode-generator.hpp>

class EngraverConnection {
public:
	EngraverConnection(const EngraverSettings &settings);

	bool isOpen() const noexcept;
	upload_instruction process();

	void updateEngraverParameters(const EngraverParameters &parameters);

	const auto &name() const noexcept { return m_name; }

private:
	QSerialPort m_port;
	QString m_name;
};
