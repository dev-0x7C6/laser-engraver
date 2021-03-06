#include "dialogs.hpp"

#include <QMessageBox>
#include <QWidget>

namespace dialogs {

void warn_empty_workspace(QWidget *parent) {
	QMessageBox::warning(parent, "Warning", "Workspace is empty, operation aborted.", QMessageBox::Ok);
}

bool ask_about_cancel(QWidget *parent) {
	return QMessageBox::question(parent, "Question", "Do you want to cancel process?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
}

int ask_repeat_workspace_preview(QWidget *parent) {
	return QMessageBox::question(parent, "Question", "Do you want to repeat workspace inspection?", QMessageBox::No | QMessageBox::Cancel | QMessageBox::Retry);
}

} // namespace dialogs
