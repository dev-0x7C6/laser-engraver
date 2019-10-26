#pragma once

#include <src/engraver-settings.h>
#include <src/gcode-generator.hpp>

#include <QObject>

class EngraverConnection : public QObject {
	Q_OBJECT
public:
	EngraverConnection(const engraver::settings::configuration &settings);

	bool isOpen() const noexcept;
	upload_instruction process();

	void updateEngraverParameters(const engraver::settings::movement_parameters &parameters);

	const auto &name() const noexcept { return m_name; }

signals:
	void gcodeSended(const QString &line);
	void gcodeReceived(const QString &line);

private:
	QSerialPort m_port;
	QString m_name;
};
