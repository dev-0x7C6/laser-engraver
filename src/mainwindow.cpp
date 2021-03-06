#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QMessageBox>
#include <QProgressDialog>
#include <QScrollBar>
#include <QTextStream>
#include <QTimer>

#include <externals/common/qt/raii/raii-settings-group.hpp>
#include <src/dialogs/dialogs.hpp>
#include <src/dialogs/font-dialog.h>
#include <src/engraver-connection.h>
#include <src/qt-wrappers.h>
#include <src/utils.hpp>
#include <src/workspace.h>

using namespace std::chrono_literals;

namespace {
constexpr auto grid_size = 5000;
constexpr auto scale_zoom_step = 0.25;
constexpr auto scale_zoom_in_step = 1.00 + scale_zoom_step;
constexpr auto scale_zoom_out_step = 1.00 - scale_zoom_step;
} // namespace

MainWindow::MainWindow(QWidget *parent)
		: QMainWindow(parent)
		, m_ui(std::make_unique<Ui::MainWindow>())
		, m_log(std::make_unique<log::model>())
		, m_settings("Laser", "Engraver")
		, m_spindle(m_connection)
		, m_engraverManager(m_settings, this)
		, m_grid(new Workspace(-grid_size, -grid_size, grid_size * 2, grid_size * 2)) {
	m_ui->setupUi(this);
	m_ui->view->setScene(m_grid);
	m_ui->objectList->setModel(m_grid->model());
	m_ui->logView->setModel(m_log.get());

	m_spindle.set_move_distance_provider([this]() { return m_ui->move_step->value(); });

	connect(m_ui->objectList, &QListView::clicked, [this, model{m_grid->model()}](const QModelIndex &index) {
		m_grid->clearSelection();
		auto &&properties = model->value(index);
		properties.item->setSelected(true);
	});

	setWindowTitle("Laser engraver");
	setWindowIcon(QIcon::fromTheme("document-print"));

	auto menu = m_ui->menu;
	auto file = menu->addMenu("&File");
	m_actionConnectEngraver = file->addAction("Connect", this, &MainWindow::connectEngraver);
	m_actionDisconnectEngraver = file->addAction("Disconnect", this, &MainWindow::disconnectEngraver);
	m_actionConnectEngraver->setIcon(QIcon::fromTheme("network-wired"));
	m_actionDisconnectEngraver->setIcon(QIcon::fromTheme("network-offline"));
	auto print = file->addAction(QIcon::fromTheme("document-print"), "&Print", this, &MainWindow::print, QKeySequence::Print);
	file->addAction(QIcon::fromTheme("document-print-preview"), "Preview", this, &MainWindow::preview, QKeySequence(Qt::Key::Key_P));
	file->addAction(QIcon::fromTheme("document-save"), "&Save As", this, &MainWindow::saveAs, QKeySequence::SaveAs);
	auto open = file->addAction(QIcon::fromTheme("document-open"), "&Open", this, &MainWindow::insertImageObject, QKeySequence::Open);
	file->addSeparator();
	file->addAction(QIcon::fromTheme("application-exit"), "&Close", this, &MainWindow::close, QKeySequence::Quit);

	connect(m_ui->dpi, qOverload<int>(&QSpinBox::valueChanged), this, [this](auto &&value) { m_grid->updateDpi(value, m_ui->scaleObjectsWithDpi->isChecked()); });

	for (auto &&category : sheet::all_iso216_category())
		connect(get_checkbox(*m_ui, category), &QCheckBox::clicked, this, &MainWindow::updateSheetReferences);

	connect(m_ui->drawInverted, &QCheckBox::clicked, this, &MainWindow::updateSheetReferences);
	connect(m_ui->drawCustom, &QGroupBox::clicked, this, &MainWindow::updateSheetReferences);
	connect(m_ui->drawCustomH, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](auto &&) { updateSheetReferences(); });
	connect(m_ui->drawCustomW, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](auto &&) { updateSheetReferences(); });
	updateSheetReferences();

	auto workspace = menu->addMenu("&Workspace");
	workspace->addAction(QIcon::fromTheme("font-x-generic"), "Insert font", this, &MainWindow::insertTextObject);
	workspace->addAction(QIcon::fromTheme("image-x-generic"), "Insert image", this, &MainWindow::insertImageObject);

	auto object = menu->addMenu("&Object");
	auto move_up = object->addAction(QIcon::fromTheme("go-up"), "Move Up", m_grid, &Workspace::selected_object_move_up, QKeySequence::Forward);
	auto move_down = object->addAction(QIcon::fromTheme("go-down"), "Move Down", m_grid, &Workspace::selected_object_move_down, QKeySequence::Back);
	auto move_top = object->addAction(QIcon::fromTheme("go-top"), "Raise to Top", m_grid, &Workspace::selected_object_raise_to_top, QKeySequence(Qt::Key::Key_Home));
	auto move_bottom = object->addAction(QIcon::fromTheme("go-bottom"), "Lower to Bottom", m_grid, &Workspace::selected_object_lower_to_bottom, QKeySequence(Qt::Key::Key_End));

	object->addSeparator();
	auto object_zoom_in_half = object->addAction(QIcon::fromTheme("zoom-in"), "Zoom in", this, &MainWindow::zoomInObject, QKeySequence(Qt::Key::Key_Plus));
	auto object_zoom_out_half = object->addAction(QIcon::fromTheme("zoom-out"), "Zoom out", this, &MainWindow::zoomOutObject, QKeySequence(Qt::Key::Key_Minus));
	object->addSeparator();
	auto center_object = object->addAction(QIcon::fromTheme("format-justify-center"), "Center", m_grid, &Workspace::selected_object_center, QKeySequence(Qt::Key::Key_C));
	object->addSeparator();
	auto remove = object->addAction(QIcon::fromTheme("edit-delete"), "Delete", m_grid, &Workspace::selected_object_remove, QKeySequence::Delete);
	auto edit_label = object->addAction("Edit label", this, &MainWindow::editLabelObject);

	for (auto &&action : {move_up, move_down, move_top, move_bottom, remove, object_zoom_in_half, object_zoom_out_half, center_object, edit_label})
		m_enableIfObjectIsSelected.addAction(action);

	m_enableIfObjectIsSelected.setEnabled(false);

	using namespace engraver;

	auto tool = menu->addMenu("&Commands");
	auto home = tool->addAction(QIcon::fromTheme("go-home"), "Home", &m_spindle, &spindle::manager::home, QKeySequence(Qt::Key::Key_H));
	auto new_home = tool->addAction(QIcon::fromTheme("go-home"), "Save as Home", &m_spindle, &spindle::manager::home_and_save, QKeySequence(Qt::Key::Key_S));
	tool->addSeparator();
	m_actionLaserState = tool->addAction(
		"Laser preview", this, &MainWindow::toggleSpindle, QKeySequence(Qt::Key::Key_Space));
	tool->addSeparator();
	auto go_u = tool->addAction(QIcon::fromTheme("go-up"), "Go Up", &m_spindle, &spindle::manager::move_up, QKeySequence(Qt::Key::Key_Up));
	auto go_d = tool->addAction(QIcon::fromTheme("go-down"), "Go Down", &m_spindle, &spindle::manager::move_down, QKeySequence(Qt::Key::Key_Down));
	auto go_l = tool->addAction(QIcon::fromTheme("go-previous"), "Go Left", &m_spindle, &spindle::manager::move_left, QKeySequence(Qt::Key::Key_Left));
	auto go_r = tool->addAction(QIcon::fromTheme("go-next"), "Go Right", &m_spindle, &spindle::manager::move_right, QKeySequence(Qt::Key::Key_Right));

	m_actionLaserState->setCheckable(true);
	m_ui->turnLaserOn->setDefaultAction(m_actionLaserState);

	m_ui->tabWidget->setCurrentIndex(0);
	m_ui->moveToolGroupBox->setEnabled(false);
	m_ui->movementSettingsGroupBox->setEnabled(false);

	for (auto &&action : {home, go_u, go_d, go_l, go_r, new_home, m_actionLaserState})
		m_enableIfEngraverConnected.addAction(action);

	m_enableIfEngraverConnected.setEnabled(false);
	m_enableIfEngraverConnected.setExclusive(false);
	m_actionDisconnectEngraver->setVisible(false);

	auto window = menu->addMenu("&Window");
	window->addAction(QIcon::fromTheme("view-fullscreen"), "Fullscreen", this, &MainWindow::toggleFullscreen, QKeySequence(Qt::Key_F11));

	auto machine = menu->addMenu("&Settings");
	machine->addAction(QIcon::fromTheme("list-add"), "Add engraver", &m_engraverManager, &EngraverManager::addEngraver);
	auto remove_engraver = machine->addAction(QIcon::fromTheme("list-remove"), "Remove engraver", &m_engraverManager, &EngraverManager::removeEngraver);
	remove_engraver->setEnabled(m_engraverManager.atLeastOneEngraverAvailable());

	connect(&m_engraverManager, &EngraverManager::engraverListChanged, [this, remove_engraver]() {
		remove_engraver->setEnabled(m_engraverManager.atLeastOneEngraverAvailable());
	});

	auto help = menu->addMenu("&Help");
	help->addAction(QIcon::fromTheme("help-about"), "About Qt", &QApplication::aboutQt);

	auto toolbar = m_ui->tool;
	toolbar->addAction(open);
	toolbar->addAction(m_actionConnectEngraver);
	toolbar->addAction(m_actionDisconnectEngraver);
	toolbar->addSeparator();
	toolbar->addAction(print);
	toolbar->addSeparator();
	toolbar->addAction(move_up);
	toolbar->addAction(remove);

	connect(m_ui->grid, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](auto &&value) {
		m_grid->setGridSize(value);
	});

	connect(m_ui->angle, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::updateItemAngle);
	connect(m_ui->opacity, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::updateItemOpacity);
	connect(m_ui->itemScale, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateItemScale);

	connect(m_ui->workspaceScale, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](auto &&value) {
		m_ui->view->resetTransform();
		m_ui->view->scale(value, value);
	});

	connect(m_grid, qOverload<bool>(&Workspace::objectSelectionChanged), &m_enableIfObjectIsSelected, &QActionGroup::setEnabled);
	connect(m_grid, qOverload<bool>(&Workspace::objectSelectionChanged), m_ui->itemGroup, &QGroupBox::setEnabled);
	connect(m_grid, qOverload<QGraphicsItem *>(&Workspace::objectSelectionChanged), [this](auto &&item) {
		if (item != nullptr)
			return;

		updateItemAngle(static_cast<int>(item->rotation()));
		updateItemOpacity(static_cast<int>(item->opacity() * 100.0));
		updateItemScale(item->scale());
	});

	auto timer = new QTimer(this);
	connect(timer, &QTimer::timeout, [this]() {
		auto x = m_grid->itemsBoundingRect();
		m_ui->statusBar->showMessage(QString("X: %1 px, Y: %2 px, W: %3 px, H: %4 px").arg(QString::number(x.x()), QString::number(x.y()), QString::number(x.width()), QString::number(x.height())));
	});
	timer->start(50ms);

	m_ui->itemGroup->setEnabled(false);

	m_ui->moveTopButton->setDefaultAction(move_up);
	m_ui->removeItemButton->setDefaultAction(remove);

	m_ui->goHome->setDefaultAction(home);
	m_ui->goUp->setDefaultAction(go_u);
	m_ui->goDown->setDefaultAction(go_d);
	m_ui->goLeft->setDefaultAction(go_l);
	m_ui->goRight->setDefaultAction(go_r);

	for (auto &&widget : {m_ui->goHome, m_ui->goUp, m_ui->goDown, m_ui->goLeft, m_ui->goRight})
		widget->setIconSize({42, 42});

	connect(m_ui->applyMovementSettings, &QToolButton::clicked, this, &MainWindow::applyMovementSettings);

	for (auto &&category : sheet::all_iso216_category()) {
		const auto metric = sheet::make_metric(category);
		get_label(*m_ui, category)->setText(QString("%1 <font color=\"gray\">(%2 mm x %3 mm)</font>").arg(name(category), QString::number(metric.w), QString::number(metric.h)));
	}

	m_guiSettings = std::make_unique<GuiSettings>(*m_ui, m_settings);
}

MainWindow::~MainWindow() {
	if (m_connection)
		disconnectEngraver();
}

void MainWindow::insertImageObject() {
	m_grid->insertPixmapObject(QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Image Files (*.png *.jpg *.bmp *.svg)")));
}

void MainWindow::insertTextObject() {
	FontDialog dialog;
	dialog.exec();

	if (const auto ret = dialog.result(); ret)
		m_grid->insertTextObject(ret.value());
}

bool MainWindow::prepare() {
	if (!is_scene_ready())
		return false;

	if (!is_connection_ready())
		return false;

	return true;
}

void MainWindow::editLabelObject() {
	if (!m_grid->selected_object())
		return;

	auto text = dynamic_cast<QGraphicsTextItem *>(m_grid->selected_object());

	if (!text)
		return;

	FontDialog dialog(text->font(), text->toPlainText());
	dialog.exec();

	if (const auto ret = dialog.result(); ret) {
		text->setFont(ret->font);
		text->setPlainText(ret->text);
	}
}

QImage MainWindow::prepareImage() {
	auto rect = m_grid->itemsBoundingRect().toRect();
	rect.moveTopLeft({0, 0});
	QPixmap canvas(rect.width(), rect.height());
	canvas.fill(Qt::white);
	QPainter painter(&canvas);
	m_grid->clearSelection();
	m_grid->setDisableBackground(true);
	m_grid->render(&painter, canvas.rect(), m_grid->itemsBoundingRect());
	m_grid->setDisableBackground(false);
	return canvas.toImage();
}

void MainWindow::toggleFullscreen() {
	if (isFullScreen())
		showMaximized();
	else
		showFullScreen();
}

void MainWindow::toggleSpindle() {
	m_spindle.set_preview_mode(m_actionLaserState->isChecked());
}

bool MainWindow::is_connected() const noexcept {
	return m_connection && m_connection->isOpen();
}

raii_tail_call MainWindow::safety_gcode_raii() noexcept {
	return {[&]() {
		if (is_connected())
			m_connection->process_safe_gcode();
		toggleSpindle();
	}};
}

gcode_generation_options MainWindow::make_gcode_generation_options_from_ui() const noexcept {
	gcode_generation_options ret;
	ret.dpi = m_ui->dpi->value();
	return ret;
}

semi::options MainWindow::make_semi_options_from_ui() const noexcept {
	semi::options ret;
	ret.power_multiplier = divide(m_ui->laser_pwr->value(), m_ui->laser_pwr->maximum());
	ret.center_object = m_ui->engraveObjectFromCenter->isChecked();
	ret.force_dwell_time = 0;
	return ret;
}

bool MainWindow::is_scene_ready() {
	if (m_grid->model()->is_empty()) {
		dialogs::warn_empty_workspace(this);
		return false;
	}

	return true;
}

bool MainWindow::is_connection_ready() {
	if (!is_connected()) {
		connectEngraver();
		if (!is_connected())
			return false;
	}

	return true;
}

auto MainWindow::generateSemiGCodeFromImage(const QImage &image) -> semi::gcodes {
	const auto opts = make_semi_options_from_ui();

	return qt_progress_task<semi::gcodes>(tr("Generating semi-gcode for post processing"), [image{std::move(image)}, opts](progress_t &progress) {
		return semi::generator::from_image(image, opts, progress);
	});
}

void MainWindow::print() {
	if (!prepare())
		return;

	const auto _{safety_gcode_raii()};

	if (m_ui->engraveFromCurrentPosition->isChecked())
		m_spindle.reset_home();

	const auto image = prepareImage();
	const auto semi_opts = make_semi_options_from_ui();
	const auto gen_opts = make_gcode_generation_options_from_ui();

	auto semi = generateSemiGCodeFromImage(image);

	auto target = [this]() { return m_connection->process(); };

	while (true) {
		generate_gcode(semi::generator::workspace_preview(image, semi_opts), gen_opts, dialogs::add_dialog_layer("Workspace", "Please inspect workspace coordinates", target()));
		const auto response = dialogs::ask_repeat_workspace_preview(this);

		if (QMessageBox::No == response)
			break;

		if (QMessageBox::Cancel == response)
			return;
	}

	generate_gcode(std::move(semi), gen_opts, dialogs::add_dialog_layer("Uploading", {}, target()));
}

auto MainWindow::targetFile(const QString &path) -> upload_instruction {
	auto file = std::make_unique<QFile>(path);
	file->open(QIODevice::ReadWrite);
	auto out = std::make_unique<QTextStream>(file.get());
	return [file{std::shared_ptr(std::move(file))},
			   out(std::shared_ptr{std::move(out)})](std::string &&gcode, double) -> upload_instruction_ret {
		(*out) << QString::fromStdString(gcode) << "\n";
		return upload_instruction_ret::keep_going;
	};
}

void MainWindow::saveAs() {
	if (!is_scene_ready())
		return;

	if (m_ui->engraveFromCurrentPosition->isChecked())
		m_spindle.reset_home();

	auto semi = generateSemiGCodeFromImage(prepareImage());

	auto file_path = QFileDialog::getSaveFileName(this, tr("Save to gcode file"), QDir::homePath(), "(*.gcode) GCode Files");

	if (file_path.isEmpty())
		return;

	generate_gcode(std::move(semi), make_gcode_generation_options_from_ui(), dialogs::add_dialog_layer("Uploading", {}, targetFile(file_path)));
}

void MainWindow::preview() {
	if (!prepare())
		return;

	const auto _ = safety_gcode_raii();

	if (m_ui->engraveFromCurrentPosition->isChecked())
		m_spindle.reset_home();

	generate_gcode(semi::generator::workspace_preview(prepareImage(), make_semi_options_from_ui()), make_gcode_generation_options_from_ui(), dialogs::add_dialog_layer("Workspace", "Please inspect workspace coordinates", m_connection->process()));
}

void MainWindow::connectEngraver() {
	if (!m_engraverManager.atLeastOneEngraverAvailable()) {
		QMessageBox::information(this, "Information", "Please add engraver machine before printing.", QMessageBox::StandardButton::Close);
		return;
	}

	auto engraver = m_engraverManager.selectEngraver();

	if (!engraver)
		return;

	QProgressDialog progress(this);
	progress.setWindowIcon(QIcon::fromTheme("network-wired"));
	progress.setWindowTitle("Connect with Engraver");
	progress.setLabelText("Connecting...");
	progress.setMinimumWidth(400);
	progress.setRange(0, 0);
	progress.setValue(0);
	progress.setCancelButton(nullptr);
	progress.setModal(true);
	progress.show();

	auto connection = std::make_unique<EngraverConnection>(engraver.value());
	connect(connection.get(), &EngraverConnection::gcodeSended, this, &MainWindow::append_log);
	connect(connection.get(), &EngraverConnection::gcodeReceived, this, &MainWindow::append_log);

	connect(m_ui->gcodeToSend, &QLineEdit::returnPressed, [this]() {
		if (m_connection && m_connection->isOpen()) {
			m_connection->process()(m_ui->gcodeToSend->text().toStdString() + '\n', {});
			m_ui->gcodeToSend->clear();
		}
	});

	if (!connection->isOpen()) {
		QMessageBox::critical(this, "Error", "Unable to open engraver communication port.", QMessageBox::StandardButton::Close);
		return;
	}

	m_connection = std::move(connection);
	m_enableIfEngraverConnected.setEnabled(true);
	m_actionConnectEngraver->setVisible(false);
	m_actionDisconnectEngraver->setVisible(true);
	m_ui->moveToolGroupBox->setEnabled(true);
	m_ui->movementSettingsGroupBox->setEnabled(true);
	m_ui->movementSettings->setParameters(engraver->movement_params);

	QMessageBox::information(this, "Information", "Engraver connected.", QMessageBox::StandardButton::Ok);
}

void MainWindow::disconnectEngraver() {
	m_connection.reset(nullptr);
	m_enableIfEngraverConnected.setEnabled(false);
	m_actionConnectEngraver->setVisible(true);
	m_actionDisconnectEngraver->setVisible(false);
	m_ui->moveToolGroupBox->setEnabled(false);
	m_ui->movementSettingsGroupBox->setEnabled(false);
}

void MainWindow::applyMovementSettings() {
	const auto movement_params = m_ui->movementSettings->parameters();
	m_connection->updateEngraverParameters(movement_params);
	m_engraverManager.update(m_connection->name(), movement_params);
}

void MainWindow::updateSheetReferences() {
	std::vector<inverter<sheet::metrics>> sheets;
	const auto is_inverted = m_ui->drawInverted->isChecked();

	auto emplace_if = [&sheets](auto check, sheet::metrics &&metrics, bool is_inverted = false) {
		if (check->isChecked())
			sheets.emplace_back(inverter<sheet::metrics>{std::move(metrics), is_inverted});
	};

	for (auto &&category : sheet::all_iso216_category())
		emplace_if(get_checkbox(*m_ui, category), sheet::make_metric(category), is_inverted);

	if (m_ui->drawCustom->isChecked())
		sheets.push_back({{"Custom", m_ui->drawCustomW->value(), m_ui->drawCustomH->value()}});

	m_grid->updateDpi(m_ui->dpi->value(), m_ui->scaleObjectsWithDpi->isChecked());
	m_grid->drawSheetAreas(std::move(sheets));
}

void MainWindow::zoomInObject() {
	if (const auto value = m_grid->selected_object()->scale() * scale_zoom_in_step; m_ui->itemScale->maximum() > value) {
		m_grid->selected_object()->setScale(value);
		m_ui->itemScale->setValue(value);
	}
}

void MainWindow::zoomOutObject() {
	if (const auto value = m_grid->selected_object()->scale() * scale_zoom_out_step; m_ui->itemScale->minimum() < value) {
		m_grid->selected_object()->setScale(value);
		m_ui->itemScale->setValue(value);
	}
}

void MainWindow::append_log(QString line) {
	m_log->insert(std::move(line), QDateTime::currentDateTime());
	m_ui->logView->scrollTo(m_log->index(m_log->rowCount({}) - 1));
}

void MainWindow::updateItemAngle(const int value) {
	m_grid->selected_object()->setRotation(value);
	m_ui->angle->setValue(value);
}

void MainWindow::updateItemOpacity(const int value) {
	m_grid->selected_object()->setOpacity(divide(value, 100.0));
	m_ui->opacity->setValue(value);
}

void MainWindow::updateItemScale(const double value) noexcept {
	m_grid->selected_object()->setScale(value);
	m_ui->itemScale->setValue(value);
}
