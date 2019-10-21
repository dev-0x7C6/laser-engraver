#ifndef MOVETOOLDIALOG_H
#define MOVETOOLDIALOG_H

#include <QDialog>

namespace Ui {
class MoveToolDialog;
}

class MoveToolDialog : public QDialog {
	Q_OBJECT

public:
	explicit MoveToolDialog(QWidget *parent = nullptr);
	~MoveToolDialog();

private:
	Ui::MoveToolDialog *ui;
};

#endif // MOVETOOLDIALOG_H
