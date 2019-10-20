#include "add-printer-dialog.h"
#include "ui_add-printer-dialog.h"

#include <QSerialPort>

AddPrinterDialog::AddPrinterDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::AddPrinterDialog)
{
	m_ui->setupUi(this);
	setWindowIcon(QIcon::fromTheme("document-print"));
	setWindowTitle("Add printer");
}

AddPrinterDialog::~AddPrinterDialog()
{
	delete m_ui;
}
