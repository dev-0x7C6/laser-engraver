#include "dialogs.hpp"

#include <QApplication>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QIcon>
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

auto wait_connect_engraver() -> std::unique_ptr<QProgressDialog> {
	auto progress = std::make_unique<QProgressDialog>();
	progress->setWindowIcon(QIcon::fromTheme("network-wired"));
	progress->setWindowTitle(QObject::tr("Connect with Engraver"));
	progress->setLabelText(QObject::tr("Connecting..."));
	progress->setMinimumWidth(400);
	progress->setRange(0, 0);
	progress->setValue(0);
	progress->setCancelButton(nullptr);
	progress->setModal(true);
	progress->show();
	return progress;
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

auto ask_gcode_file(QWidget *parent, std::function<void(QString &&path)> &&save) -> void {
	auto path = QFileDialog::getSaveFileName(parent, QObject::tr("Save to gcode file"), QDir::homePath(), "(*.gcode) GCode Files");

	if (!path.isEmpty())
		save(std::move(path));
}

} // namespace dialogs
