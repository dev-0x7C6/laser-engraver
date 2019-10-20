#pragma once

#include <QDialog>

namespace Ui {
	class AddPrinterDialog;
}

class AddPrinterDialog : public QDialog
{
	Q_OBJECT

public:
	explicit AddPrinterDialog(QWidget *parent = nullptr);
	~AddPrinterDialog();

private:
	Ui::AddPrinterDialog *m_ui;
};
