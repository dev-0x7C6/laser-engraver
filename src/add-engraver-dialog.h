#pragma once

#include <optional>
#include <QDialog>
#include <QSerialPort>
#include <QSerialPortInfo>

enum class GcodeFlavor {
	Grbl
};

struct EngraverSettings {
	std::string name;
	std::string port;
	QSerialPort::DataBits bits{QSerialPort::DataBits::Data8};
	QSerialPort::BaudRate baud{QSerialPort::BaudRate::Baud115200};
	QSerialPort::Parity parity{QSerialPort::Parity::NoParity};
	QSerialPort::FlowControl flow_control{QSerialPort::FlowControl::NoFlowControl};
	QSerialPort::StopBits stop_bits{QSerialPort::StopBits::OneStop};
};

namespace Ui {
class AddPrinterDialog;
}

class AddEngraverDialog : public QDialog {
	Q_OBJECT

public:
	explicit AddEngraverDialog(QWidget *parent = nullptr);
	~AddEngraverDialog();

	auto result() { return m_result; }

private:
	void saveResult();

private:
	std::optional<EngraverSettings> m_result;
	Ui::AddPrinterDialog *m_ui;
};
