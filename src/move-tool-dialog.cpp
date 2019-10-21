#include "move-tool-dialog.h"
#include "ui_move-tool-dialog.h"

MoveToolDialog::MoveToolDialog(QWidget *parent)
		: QDialog(parent)
		, ui(new Ui::MoveToolDialog) {
	ui->setupUi(this);
}

MoveToolDialog::~MoveToolDialog() {
	delete ui;
}
