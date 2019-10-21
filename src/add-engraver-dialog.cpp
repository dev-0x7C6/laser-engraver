#include "add-engraver-dialog.h"
#include "ui_add-engraver-dialog.h"

#include <QSerialPort>
#include <QSerialPortInfo>

Q_DECLARE_METATYPE(GcodeFlavor);
Q_DECLARE_METATYPE(QSerialPort::BaudRate);
Q_DECLARE_METATYPE(QSerialPort::DataBits);
Q_DECLARE_METATYPE(QSerialPort::FlowControl);
Q_DECLARE_METATYPE(QSerialPort::Parity);
Q_DECLARE_METATYPE(QSerialPort::StopBits);
Q_DECLARE_METATYPE(QSerialPortInfo);

AddEngraverDialog::AddEngraverDialog(QWidget *parent)
		: QDialog(parent)
		, m_ui(std::make_unique<Ui::AddPrinterDialog>()) {
	m_ui->setupUi(this);
	setWindowIcon(QIcon::fromTheme("document-print"));
	setWindowTitle("Add printer");

	for (auto &&port : QSerialPortInfo::availablePorts())
		m_ui->device->addItem(port.systemLocation(), QVariant::fromValue(port));

	for (auto &&baud : QSerialPortInfo::standardBaudRates())
		m_ui->baud->addItem(QString::number(baud), baud);

	m_ui->baud->setCurrentIndex(m_ui->baud->findText("115200"));

	m_ui->flow_control->addItem("None", QVariant::fromValue(QSerialPort::FlowControl::NoFlowControl));
	m_ui->flow_control->addItem("Software", QSerialPort::FlowControl::SoftwareControl);
	m_ui->flow_control->addItem("Hardware", QSerialPort::FlowControl::HardwareControl);

	m_ui->data_bits->addItem("5 bits", QSerialPort::DataBits::Data5);
	m_ui->data_bits->addItem("6 bits", QSerialPort::DataBits::Data6);
	m_ui->data_bits->addItem("7 bits", QSerialPort::DataBits::Data7);
	m_ui->data_bits->addItem("8 bits", QSerialPort::DataBits::Data8);
	m_ui->data_bits->setCurrentIndex(3);

	m_ui->parity->addItem("None", QSerialPort::Parity::NoParity);
	m_ui->parity->addItem("Odd", QSerialPort::Parity::OddParity);
	m_ui->parity->addItem("Even", QSerialPort::Parity::EvenParity);
	m_ui->parity->addItem("Mark", QSerialPort::Parity::MarkParity);
	m_ui->parity->addItem("Space", QSerialPort::Parity::SpaceParity);

	m_ui->stop_bits->addItem("1", QSerialPort::StopBits::OneStop);
	m_ui->stop_bits->addItem("1.5", QSerialPort::StopBits::OneAndHalfStop);
	m_ui->stop_bits->addItem("2", QSerialPort::StopBits::TwoStop);

	m_ui->gcode_flavor->addItem("Grbl", QVariant::fromValue(GcodeFlavor::Grbl));

	connect(m_ui->buttons, &QDialogButtonBox::accepted, this, &AddEngraverDialog::saveResult);
}

AddEngraverDialog::~AddEngraverDialog() = default;

void AddEngraverDialog::saveResult() {
	EngraverSettings settings;
	settings.name = m_ui->name->text();
	settings.port = m_ui->device->currentText();
	settings.baud = m_ui->baud->currentData().value<QSerialPort::BaudRate>();
	settings.parity = m_ui->parity->currentData().value<QSerialPort::Parity>();
	settings.stop_bits = m_ui->stop_bits->currentData().value<QSerialPort::StopBits>();
	settings.bits = m_ui->data_bits->currentData().value<QSerialPort::DataBits>();
	settings.flow_control = m_ui->flow_control->currentData().value<QSerialPort::FlowControl>();
	m_result = settings;
}
