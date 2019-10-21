#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTextStream>
#include <QTimer>

#include <chrono>
#include <future>
#include <iostream>
#include <thread>

#include <grid-scene.h>
#include <src/gcode-generator.hpp>
#include <src/add-engraver-dialog.h>
#include <src/select-engraver-dialog.h>

using namespace std::chrono_literals;

namespace {
constexpr auto grid_size = 5000;
}

MainWindow::MainWindow(QWidget *parent)
		: QMainWindow(parent)
		, m_ui(std::make_unique<Ui::MainWindow>())
		, m_settings("Laser", "Engraver")
		, m_grid(new GridScene(-grid_size, -grid_size, grid_size * 2, grid_size * 2)) {
	m_ui->setupUi(this);
	m_ui->view->setScene(m_grid);

	m_settings.beginGroup("devices");
	for (auto &&key : m_settings.childGroups()) {
		m_settings.beginGroup(key);
		m_engravers.emplace_back(load(m_settings));
		m_settings.endGroup();
	}
	m_settings.endGroup();

	setWindowTitle("Laser engraver");
	setWindowIcon(QIcon::fromTheme("document-print"));

	auto menu = m_ui->menu;
	auto file = menu->addMenu("&File");
	auto print = file->addAction("&Print", this, &MainWindow::print);
	auto open = file->addAction("&Open", this, &MainWindow::open);
	file->addSeparator();
	auto exit = file->addAction("&Close", this, &MainWindow::close);

	open->setShortcuts(QKeySequence::Open);
	open->setIcon(QIcon::fromTheme("document-open"));
	print->setShortcut(QKeySequence::Print);
	print->setIcon(QIcon::fromTheme("document-print"));
	exit->setShortcuts(QKeySequence::Quit);
	exit->setIcon(QIcon::fromTheme("application-exit"));

	auto edit = menu->addMenu("&Edit");
	auto move_up = edit->addAction("Move Up", this, &MainWindow::itemMoveTop);
	move_up->setShortcut(QKeySequence::Forward);
	move_up->setIcon(QIcon::fromTheme("go-top"));
	move_up->setEnabled(false);
	edit->addSeparator();
	auto remove = edit->addAction("Delete", this, &MainWindow::removeItem);

	remove->setShortcuts(QKeySequence::Delete);
	remove->setIcon(QIcon::fromTheme("edit-delete"));
	remove->setEnabled(false);
	edit->addSeparator();

	auto machine = menu->addMenu("&Machine");
	auto add_engraver = machine->addAction("Add engraver", this, &MainWindow::addEngraver);
	add_engraver->setIcon(QIcon::fromTheme("list-add"));

	auto tool = m_ui->tool;
	tool->addAction(open);
	tool->addSeparator();
	tool->addAction(print);
	tool->addSeparator();
	tool->addAction(move_up);
	tool->addAction(remove);
	tool->addSeparator();
	tool->addAction(add_engraver);

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

	connect(m_grid, &QGraphicsScene::selectionChanged, [this, move_up, remove]() {
		auto list = m_grid->selectedItems();
		const auto enabled = !list.isEmpty();

		m_ui->itemWidget->setEnabled(enabled);
		move_up->setEnabled(enabled);
		remove->setEnabled(enabled);

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

	m_ui->itemWidget->setEnabled(false);

	m_ui->moveTopButton->setDefaultAction(move_up);
	m_ui->removeItemButton->setDefaultAction(remove);
}

MainWindow::~MainWindow() {
	m_settings.beginGroup("devices");
	for (auto &&engraver : m_engravers) {
		m_settings.beginGroup(engraver.name);
		save(m_settings, engraver);
		m_settings.endGroup();
	}
	m_settings.endGroup();
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
	const auto img = prepareImage();

	options opts;
	opts.power_multiplier = static_cast<double>(m_ui->laser_pwr->value()) / static_cast<double>(m_ui->laser_pwr->maximum());
	opts.center_object = m_ui->center_object->isChecked();

	auto semi = qt_progress_task<semi_gcodes>(tr("Generating semi-gcode for post processing"), [&img, opts](progress_t &progress) {
		return image_to_semi_gcode(img, opts, progress);
	});

	gcode_generation_options generation_options;
	generation_options.dpi = m_ui->dpi->value();

	SelectEngraverDialog dialog(m_engravers, this);
	dialog.exec();
	const auto settings = dialog.result();

	if (!settings) {
		QMessageBox::critical(this, "Error", "Please select engraver machine.", QMessageBox::StandardButton::Close);
		return;
	}

	QSerialPort port(settings->port);
	port.setBaudRate(settings->baud);
	port.setParity(settings->parity);
	port.setDataBits(settings->bits);
	port.setFlowControl(settings->flow_control);
	port.setStopBits(settings->stop_bits);
	if (!port.open(QSerialPort::ReadWrite)) {
		QMessageBox::critical(this, "Error", "Unable to open engraver communication port.", QMessageBox::StandardButton::Close);
		return;
	}

	port.clear();
	for (auto i = 0; i < 3000; ++i) {
		const auto response = port.readLine();
		if (!response.isEmpty())
			std::cout << response.toStdString() << std::endl;
		port.waitForReadyRead(1);
	}
	port.clear();

	auto write_serial = [&](auto &&instruction, double) -> upload_instruction_ret {
		std::cout << "GCODE: " << instruction << std::endl;
		instruction += "\n";
		port.write(instruction.c_str(), instruction.size());
		port.waitForBytesWritten();

		for (int retry = 0; retry < 30000; ++retry) {
			port.waitForReadyRead(1);
			QApplication::processEvents(QEventLoop::AllEvents, 1);
			const auto response = port.readLine();
			if (!response.isEmpty()) {
				std::cout << response.toStdString() << std::endl;
				break;
			}
		}

		return upload_instruction_ret::keep_going;
	};

	while (true) {
		generate_gcode(generate_workspace_demo(img, opts), generation_options, add_dialog_layer(this, "Workspace", "Please inspect workspace coordinates", write_serial));
		generate_gcode(generate_end_section(), generation_options, write_serial);
		const auto response = QMessageBox::question(this, "Question", "Do you want to repeat workspace inspection?", QMessageBox::No | QMessageBox::Cancel | QMessageBox::Retry);

		if (QMessageBox::No == response)
			break;

		if (QMessageBox::Cancel == response)
			return;
	}

	generate_gcode(std::move(semi), generation_options, add_dialog_layer(this, "Uploading", {}, write_serial));
	generate_gcode(generate_end_section(), generation_options, write_serial);
}

void MainWindow::addEngraver() {
	AddEngraverDialog dialog;
	dialog.exec();

	if (dialog.result())
		m_engravers.emplace_back(dialog.result().value());
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
