#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QProgressDialog>
#include <QScrollBar>
#include <QTimer>
#include <QDateTime>

#include <chrono>
#include <future>
#include <thread>

#include <grid-scene.h>
#include <src/engraver-connection.h>

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
		, m_settings("Laser", "Engraver")
		, m_engraverManager(m_settings, this)
		, m_grid(new GridScene(-grid_size, -grid_size, grid_size * 2, grid_size * 2)) {
	m_ui->setupUi(this);
	m_ui->view->setScene(m_grid);

	setWindowTitle("Laser engraver");
	setWindowIcon(QIcon::fromTheme("document-print"));

	auto menu = m_ui->menu;
	auto file = menu->addMenu("&File");
	m_actionConnectEngraver = file->addAction("Connect", this, &MainWindow::connectEngraver);
	m_actionDisconnectEngraver = file->addAction("Disconnect", this, &MainWindow::disconnectEngraver);
	m_actionConnectEngraver->setIcon(QIcon::fromTheme("network-wired"));
	m_actionDisconnectEngraver->setIcon(QIcon::fromTheme("network-offline"));
	auto print = file->addAction("&Print", this, &MainWindow::print);
	auto open = file->addAction("&Open", this, &MainWindow::open);
	file->addSeparator();
	auto exit = file->addAction("&Close", this, &MainWindow::close);

	connect(m_ui->dpi, qOverload<int>(&QSpinBox::valueChanged), m_grid, &GridScene::updateDpi);

	for (auto &&checkbox : {m_ui->drawA0, m_ui->drawA1, m_ui->drawA2, m_ui->drawA3, m_ui->drawA4, m_ui->drawA5, m_ui->drawA6, m_ui->drawA7, m_ui->drawA8})
		connect(checkbox, &QCheckBox::clicked, this, &MainWindow::updateSheetReferences);
	updateSheetReferences();

	open->setShortcuts(QKeySequence::Open);
	open->setIcon(QIcon::fromTheme("document-open"));
	print->setShortcut(QKeySequence::Print);
	print->setIcon(QIcon::fromTheme("document-print"));
	exit->setShortcuts(QKeySequence::Quit);
	exit->setIcon(QIcon::fromTheme("application-exit"));

	auto object = menu->addMenu("&Object");

	auto move_up = object->addAction("Move Up", this, &MainWindow::itemMoveTop);
	object->addSeparator();
	auto object_zoom_in_half = object->addAction("Zoom in", this, &MainWindow::zoomInObject);
	auto object_zoom_out_half = object->addAction("Zoom out", this, &MainWindow::zoomOutObject);
	auto remove = object->addAction("Delete", this, &MainWindow::removeItem);
	object->addSeparator();

	object_zoom_in_half->setShortcut(QKeySequence(Qt::Key::Key_Plus));
	object_zoom_out_half->setShortcut(QKeySequence(Qt::Key::Key_Minus));
	object_zoom_in_half->setIcon(QIcon::fromTheme("zoom-in"));
	object_zoom_out_half->setIcon(QIcon::fromTheme("zoom-out"));
	move_up->setShortcut(QKeySequence::Forward);
	move_up->setIcon(QIcon::fromTheme("go-top"));
	remove->setShortcuts(QKeySequence::Delete);
	remove->setIcon(QIcon::fromTheme("edit-delete"));

	for (auto &&action : {move_up, remove, object_zoom_in_half, object_zoom_out_half})
		m_enableIfObjectIsSelected.addAction(action);

	m_enableIfObjectIsSelected.setEnabled(false);

	auto tool = menu->addMenu("&Commands");
	auto home = tool->addAction("Home", [this]() { go(direction::home); });
	auto new_home = tool->addAction("Save as Home", [this]() { go(direction::new_home); });
	tool->addSeparator();
	m_actionLaserOn = tool->addAction("Laser On", [this]() { turnLaser(true); });
	m_actionLaserOff = tool->addAction("Laser Off", [this]() { turnLaser(false); });
	tool->addSeparator();
	auto go_u = tool->addAction("Go Up", [this]() { go(direction::up); });
	auto go_d = tool->addAction("Go Down", [this]() { go(direction::down); });
	auto go_l = tool->addAction("Go Left", [this]() { go(direction::left); });
	auto go_r = tool->addAction("Go Right", [this]() { go(direction::right); });

	m_actionLaserOn->setVisible(true);
	m_actionLaserOff->setVisible(false);
	m_actionLaserOn->setShortcut(QKeySequence(Qt::Key::Key_Space));
	m_actionLaserOff->setShortcut(QKeySequence(Qt::Key::Key_Space));
	m_ui->turnLaserOn->setDefaultAction(m_actionLaserOn);
	m_ui->turnLaserOff->setDefaultAction(m_actionLaserOff);

	new_home->setIcon(QIcon::fromTheme("go-home"));
	new_home->setShortcut(QKeySequence(Qt::Key::Key_S));
	home->setIcon(QIcon::fromTheme("go-home"));
	home->setShortcut(QKeySequence(Qt::Key::Key_H));
	go_u->setIcon(QIcon::fromTheme("go-up"));
	go_u->setShortcut(QKeySequence(Qt::Key::Key_Up));
	go_d->setIcon(QIcon::fromTheme("go-down"));
	go_d->setShortcut(QKeySequence(Qt::Key::Key_Down));
	go_l->setIcon(QIcon::fromTheme("go-previous"));
	go_l->setShortcut(QKeySequence(Qt::Key::Key_Left));
	go_r->setIcon(QIcon::fromTheme("go-next"));
	go_r->setShortcut(QKeySequence(Qt::Key::Key_Right));

	m_ui->tabWidget->setCurrentIndex(0);
	m_ui->moveToolGroupBox->setEnabled(false);
	m_ui->movementSettingsGroupBox->setEnabled(false);

	for (auto &&action : {home, go_u, go_d, go_l, go_r, print, new_home, m_actionLaserOn, m_actionLaserOff})
		m_enableIfEngraverConnected.addAction(action);

	m_enableIfEngraverConnected.setEnabled(false);
	m_actionDisconnectEngraver->setVisible(false);

	auto machine = menu->addMenu("&Settings");
	auto add_engraver = machine->addAction("Add engraver", &m_engraverManager, &EngraverManager::addEngraver);
	auto remove_engraver = machine->addAction("Remove engraver", &m_engraverManager, &EngraverManager::removeEngraver);
	add_engraver->setIcon(QIcon::fromTheme("list-add"));
	remove_engraver->setIcon(QIcon::fromTheme("list-remove"));
	remove_engraver->setEnabled(m_engraverManager.atLeastOneEngraverAvailable());

	connect(&m_engraverManager, &EngraverManager::engraverListChanged, [this, remove_engraver]() {
		remove_engraver->setEnabled(m_engraverManager.atLeastOneEngraverAvailable());
	});

	auto toolbar = m_ui->tool;
	toolbar->addAction(open);
	toolbar->addAction(m_actionConnectEngraver);
	toolbar->addAction(m_actionDisconnectEngraver);
	toolbar->addSeparator();
	toolbar->addAction(print);
	toolbar->addSeparator();
	toolbar->addAction(move_up);
	toolbar->addAction(remove);

	for (auto &&v : {10, 25, 50, 100, 200, 400, 800}) {
		m_ui->scale->addItem(QString::number(v) + "%", v);
	}

	m_ui->scale->setCurrentText("100%");

	connect(m_ui->grid, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](auto &&value) {
		m_grid->setGridSize(value);
	});

	connect(m_ui->angle, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::updateItemAngle);
	connect(m_ui->opacity, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::updateItemOpacity);
	connect(m_ui->itemScale, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &MainWindow::updateItemScale);

	connect(m_ui->scale, qOverload<int>(&QComboBox::currentIndexChanged), [this](auto &&index) {
		const auto v = static_cast<double>(m_ui->scale->itemData(index).toInt()) / 100.0;
		m_ui->view->resetTransform();
		m_ui->view->scale(v, v);
	});

	connect(m_grid, &QGraphicsScene::selectionChanged, [this]() {
		auto list = m_grid->selectedItems();
		const auto enabled = !list.isEmpty();

		m_ui->itemGroup->setEnabled(enabled);
		m_enableIfObjectIsSelected.setEnabled(enabled);

		if (!enabled) {
			m_selectedItem = nullptr;
			return;
		}

		m_selectedItem = list.first();
		updateItemAngle(static_cast<int>(m_selectedItem->rotation()));
		updateItemOpacity(static_cast<int>(m_selectedItem->opacity() * 100.0));
		updateItemScale(m_selectedItem->scale());
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

	m_ui->outgoingGCode->append(QDateTime::currentDateTime().toString() + '\n');
}

MainWindow::~MainWindow() {
	if (m_connection)
		disconnectEngraver();
}

void MainWindow::open() {
	auto path = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Image Files (*.png *.jpg *.bmp *.svg)"));

	if (path.isEmpty())
		return;

	auto item = m_grid->addPixmap({path});
	item->setTransformationMode(Qt::SmoothTransformation);
	item->setFlag(QGraphicsItem::ItemIsMovable);
	item->setFlag(QGraphicsItem::ItemIsSelectable);
	item->setTransformOriginPoint(item->boundingRect().width() / 2, item->boundingRect().height() / 2);
	item->setX(item->boundingRect().width() / -2);
	item->setY(item->boundingRect().height() / -2);
	item->setZValue(item->topLevelItem()->zValue() + 1.0);
}

void qt_generate_progress_dialog(QString &&title, progress_t &progress) {
	QProgressDialog dialog;
	QTimer timer;
	QObject::connect(&timer, &QTimer::timeout, &dialog, [&dialog, &progress]() {
		dialog.setValue(static_cast<int>(progress * 1000.0));
	});

	dialog.setLabelText(title);
	dialog.setMinimum(0);
	dialog.setMaximum(1000);
	dialog.setCancelButton(nullptr);
	timer.start(5ms);
	dialog.exec();
}

template <typename return_type>
return_type qt_progress_task(QString &&title, std::function<return_type(progress_t &)> &&callable) {
	progress_t progress{};
	auto task = std::packaged_task<return_type()>([callable{std::move(callable)}, &progress]() {
		return callable(progress);
	});

	auto result = task.get_future();

	std::thread thread(std::move(task));

	qt_generate_progress_dialog(std::move(title), progress);
	result.wait();
	thread.join();

	return result.get();
}

upload_instruction add_dialog_layer(QWidget *parent, const QString &title, const QString &text, upload_instruction interpreter) {
	auto dialog = new QProgressDialog(parent);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->setAutoClose(true);
	dialog->setAutoReset(true);
	dialog->setWindowTitle(title);
	dialog->setLabelText(text);
	dialog->setRange(0, 10000);
	dialog->setValue(0);
	dialog->setMinimumSize(400, 100);
	dialog->setModal(true);
	dialog->show();

	return [dialog, interpreter{std::move(interpreter)}, show_gcode{text.isEmpty()}](std::string &&instruction, double progress) -> upload_instruction_ret {
		dialog->setValue(static_cast<int>(progress * 10000));

		if (show_gcode)
			dialog->setLabelText("GCODE: " + QString::fromStdString(instruction));

		if (dialog->wasCanceled()) {
			dialog->close();
			return upload_instruction_ret::cancel;
		}

		QApplication::processEvents(QEventLoop::AllEvents, 1);
		return interpreter(std::move(instruction), progress);
	};
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

void MainWindow::print() {
	if (!m_connection)
		return;

	if (!m_connection->isOpen())
		return;

	if (m_ui->saveHomeAfterMove->isChecked()) {
		generate_gcode({new_home{0, 0}}, {}, m_connection->process());
		m_x = 0.0;
		m_y = 0.0;
	}

	const auto img = prepareImage();

	options opts;
	opts.power_multiplier = static_cast<double>(m_ui->laser_pwr->value()) / static_cast<double>(m_ui->laser_pwr->maximum());
	opts.center_object = m_ui->center_object->isChecked();
	opts.force_dwell_time = 0;

	auto semi = qt_progress_task<semi_gcodes>(tr("Generating semi-gcode for post processing"), [&img, opts](progress_t &progress) {
		return image_to_semi_gcode(img, opts, progress);
	});

	gcode_generation_options generation_options;
	generation_options.dpi = m_ui->dpi->value();

	while (true) {
		generate_gcode(generate_workspace_demo(img, opts), generation_options, add_dialog_layer(this, "Workspace", "Please inspect workspace coordinates", m_connection->process()));
		generate_gcode(generate_end_section(), generation_options, m_connection->process());
		const auto response = QMessageBox::question(this, "Question", "Do you want to repeat workspace inspection?", QMessageBox::No | QMessageBox::Cancel | QMessageBox::Retry);

		if (QMessageBox::No == response)
			break;

		if (QMessageBox::Cancel == response)
			return;
	}

	generate_gcode(std::move(semi), generation_options, add_dialog_layer(this, "Uploading", {}, m_connection->process()));
	generate_gcode(generate_end_section(), generation_options, m_connection->process());
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
	connect(connection.get(), &EngraverConnection::gcodeSended, this, [this](auto &&line) {
		m_ui->outgoingGCode->append(line);
		auto scroll = m_ui->outgoingGCode->verticalScrollBar();
		scroll->setValue(scroll->maximum());
	});
	connect(connection.get(), &EngraverConnection::gcodeReceived, this, [this](auto &&line) {
		m_ui->outgoingGCode->append(line);
		auto scroll = m_ui->outgoingGCode->verticalScrollBar();
		scroll->setValue(scroll->maximum());
	});
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

void MainWindow::turnLaser(const bool on) {
	if (on)
		generate_gcode({laser_on{}, power{1}}, {}, m_connection->process());
	else
		generate_gcode({power{0}, laser_off{}}, {}, m_connection->process());

	m_actionLaserOn->setVisible(!on);
	m_actionLaserOff->setVisible(on);
}

void MainWindow::applyMovementSettings() {
	const auto movement_params = m_ui->movementSettings->parameters();
	m_connection->updateEngraverParameters(movement_params);
	m_engraverManager.update(m_connection->name(), movement_params);
}

void MainWindow::updateSheetReferences() {
	std::vector<sheet_metrics> sheets;
	if (m_ui->drawA0->isChecked())
		sheets.push_back({"A0", 841, 1189});
	if (m_ui->drawA1->isChecked())
		sheets.push_back({"A1", 594, 841});
	if (m_ui->drawA2->isChecked())
		sheets.push_back({"A2", 420, 594});
	if (m_ui->drawA3->isChecked())
		sheets.push_back({"A3", 297, 420});
	if (m_ui->drawA4->isChecked())
		sheets.push_back({"A4", 210, 297});
	if (m_ui->drawA5->isChecked())
		sheets.push_back({"A5", 148, 210});
	if (m_ui->drawA6->isChecked())
		sheets.push_back({"A6", 105, 148});
	if (m_ui->drawA7->isChecked())
		sheets.push_back({"A7", 74, 105});
	if (m_ui->drawA8->isChecked())
		sheets.push_back({"A8", 52, 74});
	m_grid->updateDpi(m_ui->dpi->value());
	m_grid->drawSheetAreas(std::move(sheets));
}

void MainWindow::go(const direction value) {
	const auto step = m_ui->move_step->value();
	switch (value) {
		case direction::up: return generate_gcode({::move_raw{m_x, (m_y -= step)}}, {}, m_connection->process());
		case direction::down: return generate_gcode({::move_raw{m_x, (m_y += step)}}, {}, m_connection->process());
		case direction::left: return generate_gcode({::move_raw{(m_x -= step), m_y}}, {}, m_connection->process());
		case direction::right: return generate_gcode({::move_raw{(m_x += step), m_y}}, {}, m_connection->process());
		case direction::home:
			m_x = 0.0;
			m_y = 0.0;
			return generate_gcode({home{}}, {}, m_connection->process());
		case direction::new_home:
			generate_gcode({new_home{0, 0}, home{}}, {}, m_connection->process());
			m_x = 0.0;
			m_y = 0.0;
			return;
	}
}

void MainWindow::zoomInObject() {
	if (const auto value = m_selectedItem->scale() * scale_zoom_in_step; m_ui->itemScale->maximum() > value) {
		m_selectedItem->setScale(value);
		m_ui->itemScale->setValue(value);
	}
}

void MainWindow::zoomOutObject() {
	if (const auto value = m_selectedItem->scale() * scale_zoom_out_step; m_ui->itemScale->minimum() < value) {
		m_selectedItem->setScale(value);
		m_ui->itemScale->setValue(value);
	}
}

bool MainWindow::isItemSelected() const noexcept {
	return m_selectedItem != nullptr;
}

void MainWindow::itemMoveTop() {
	if (m_selectedItem)
		m_selectedItem->setZValue(m_selectedItem->topLevelItem()->zValue() + 1.0);
}

void MainWindow::removeItem() {
	delete m_selectedItem;
}

void MainWindow::updateItemAngle(const int value) {
	m_selectedItem->setRotation(value);
	m_ui->angle->setValue(value);
}

void MainWindow::updateItemOpacity(const int value) {
	m_selectedItem->setOpacity(static_cast<double>(value) / 100.0);
	m_ui->opacity->setValue(value);
}

void MainWindow::updateItemScale(const double value) noexcept {
	m_selectedItem->setScale(value);
	m_ui->itemScale->setValue(value);
}
