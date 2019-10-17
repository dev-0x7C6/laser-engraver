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

using namespace std::chrono_literals;

namespace {
constexpr auto grid_size = 5000;
}

MainWindow::MainWindow(QWidget *parent)
		: QMainWindow(parent)
		, m_ui(std::make_unique<Ui::MainWindow>())
		, m_grid(new GridScene(-grid_size, -grid_size, grid_size * 2, grid_size * 2)) {
	m_ui->setupUi(this);
	m_ui->view->setScene(m_grid);

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

	auto tool = m_ui->tool;
	tool->addAction(open);
	tool->addSeparator();
	tool->addAction(print);
	tool->addSeparator();
	tool->addAction(move_up);
	tool->addAction(remove);

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

void MainWindow::open() {
	auto path = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Image Files (*.png *.jpg *.bmp)"));

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

void MainWindow::print() {
	auto rect = m_grid->itemsBoundingRect().toRect();
	rect.moveTopLeft({0, 0});
	QPixmap canvas(rect.width(), rect.height());
	canvas.fill(Qt::white);
	QPainter painter(&canvas);
	m_grid->clearSelection();
	m_grid->setDisableBackground(true);
	m_grid->render(&painter, canvas.rect(), m_grid->itemsBoundingRect());
	m_grid->setDisableBackground(false);

	auto img = canvas.toImage();

	std::cout << "w: " << img.width() << std::endl;
	std::cout << "h: " << img.height() << std::endl;
	std::cout << "s: " << img.sizeInBytes() << std::endl;

	options opts;
	opts.power_multiplier = 0.2;
	opts.force_dwell_time = 1;

	auto semi = qt_progress_task<semi_gcodes>(tr("Generating semi-gcode for post processing"), [&img, opts](progress_t &progress) {
		return image_to_semi_gcode(img, opts, progress);
	});

	gcode_generation_options generation_options;
	generation_options.dpi = m_ui->dpi->value();

	QSerialPort port(QSerialPortInfo::availablePorts().back());
	port.open(QSerialPort::ReadWrite);
	port.setBaudRate(115200);

	port.waitForReadyRead(5000);
	port.readAll();

	QProgressDialog dialog;
	dialog.setWindowTitle("Showing workspace...");
	dialog.setLabelText("Please inspect workspace");
	dialog.setRange(0, 0);
	dialog.setValue(0);
	dialog.setMinimumSize(400, 100);
	dialog.setCancelButton(nullptr);
	dialog.setModal(true);
	dialog.show();

	auto upload_gcode = [&port, &dialog](auto &&instruction, double progress) -> upload_instruction_ret {
		if (instruction.empty())
			return upload_instruction_ret::keep_going;

		if (dialog.wasCanceled() && dialog.isVisible())
			return upload_instruction_ret::cancel;

		if (dialog.minimum() != dialog.maximum()) {
			dialog.setValue(static_cast<int>(progress * 10000));
			dialog.setLabelText("GCODE: " + QString::fromStdString(instruction));
		}

		QApplication::processEvents(QEventLoop::AllEvents, 1);

		instruction += "\n";
		std::cout << progress * 100.0 << "%: " << instruction;
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
		generate_gcode(generate_workspace_demo(img), generation_options, upload_gcode);
		const auto response = QMessageBox::question(this, "Question", "Do you want to repeat workspace inspection?", QMessageBox::No | QMessageBox::Cancel | QMessageBox::Retry);

		if (QMessageBox::No == response)
			break;

		if (QMessageBox::Cancel == response)
			return;
	}

	dialog.setWindowTitle("Uploading gcodes...");
	dialog.setRange(0, 10000);
	dialog.setValue(0);
	dialog.setCancelButton(new QPushButton("Cancel"));
	dialog.show();
	generate_gcode(std::move(semi), generation_options, upload_gcode);

	dialog.setModal(false);
	dialog.hide();
	dialog.reset();

	generate_gcode(generate_safety_shutdown(), generation_options, upload_gcode);
	QApplication::processEvents(QEventLoop::AllEvents, 1000);
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

MainWindow::~MainWindow() = default;
