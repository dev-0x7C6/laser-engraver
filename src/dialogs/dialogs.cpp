#include "dialogs.hpp"

#include <QMessageBox>
#include <QWidget>

namespace dialogs {


void dialog_empty_workspace(QWidget *parent)
{
QMessageBox::warning(parent, "Warning", "Workspace is empty, operation aborted.", QMessageBox::Ok);
}


}
