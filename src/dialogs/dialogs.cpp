#include "dialogs.hpp"

#include <QApplication>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QProgressDialog>
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

upload_instruction add_dialog_layer(const QString &title, const QString &text, upload_instruction &&interpreter) {
	auto dialog = std::make_unique<QProgressDialog>(nullptr);
	dialog->setWindowTitle(title);
	dialog->setLabelText(text);
	dialog->setRange(0, 10000);
	dialog->setValue(0);
	dialog->setMinimumSize(400, 100);
	dialog->setModal(true);
	dialog->show();

	auto elapsed = std::make_unique<QElapsedTimer>();
	elapsed->start();

	return [elapsed{std::shared_ptr(std::move(elapsed))},
			   dialog{std::shared_ptr(std::move(dialog))},
			   interpreter{std::move(interpreter)},
			   show_gcode{text.isEmpty()}](std::string &&instruction, double progress) -> upload_instruction_ret {
		dialog->setValue(static_cast<int>(progress * 10000));

		if (show_gcode && elapsed->hasExpired(16)) {
			dialog->setLabelText("GCODE: " + QString::fromStdString(instruction));
			elapsed->restart();
		}

		if (dialog->wasCanceled()) {
			if (dialogs::ask_about_cancel(dialog->parentWidget()))
				return upload_instruction_ret::cancel;

			dialog->reset();
		}

		QApplication::processEvents(QEventLoop::AllEvents, 0);
		return interpreter(std::move(instruction), progress);
	};
}

} // namespace dialogs
