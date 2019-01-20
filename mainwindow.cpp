#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QTimer>

#include <grid-scene.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <variant>
#include <vector>

#include <externals/common/types.hpp>

using namespace std::chrono_literals;

MainWindow::MainWindow(QWidget *parent)
		: QMainWindow(parent)
		, m_ui(std::make_unique<Ui::MainWindow>()) {
	m_ui->setupUi(this);

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
	edit->addSeparator();
	auto remove = edit->addAction("Delete", this, &MainWindow::removeItem);

	remove->setShortcuts(QKeySequence::Delete);
	remove->setIcon(QIcon::fromTheme("edit-delete"));
	edit->addSeparator();

	auto tool = m_ui->tool;
	tool->addAction(open);
	tool->addSeparator();
	tool->addAction(print);
	tool->addSeparator();
	tool->addAction(move_up);
	tool->addAction(remove);

	constexpr auto grid_size = 5000;

	auto scene = std::make_unique<GridScene>(-grid_size, -grid_size, grid_size * 2, grid_size * 2);
	m_ui->view->setScene(scene.release());

	for (auto &&v : {10, 25, 50, 100, 200, 400, 800}) {
		m_ui->scale->addItem(QString::number(v) + "%", v);
	}

	m_ui->scale->setCurrentText("100%");

	connect(m_ui->grid, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](auto &&value) {
		auto scene = dynamic_cast<GridScene *>(m_ui->view->scene());
		scene->setGridSize(value);
	});

	connect(m_ui->angle, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::updateAngle);
	connect(m_ui->angleDial, &QDial::valueChanged, this, &MainWindow::updateAngle);
	connect(m_ui->opacity, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::updateOpacity);
	connect(m_ui->opacitySlider, &QSlider::valueChanged, this, &MainWindow::updateOpacity);

	connect(m_ui->scale, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](auto &&index) {
		const auto v = static_cast<double>(m_ui->scale->itemData(index).toInt()) / 100.0;
		m_ui->view->resetTransform();
		m_ui->view->scale(v, v);
	});

	connect(m_ui->view->scene(), &QGraphicsScene::selectionChanged, [this]() {
		auto list = m_ui->view->scene()->selectedItems();
		if (list.isEmpty()) {
			m_ui->itemWidget->setEnabled(false);
			m_selectedItem = nullptr;
			return;
		}

		m_selectedItem = list.first();

		updateAngle(static_cast<int>(m_selectedItem->rotation()));
		updateOpacity(static_cast<int>(m_selectedItem->opacity() * 100.0));

		m_ui->itemWidget->setEnabled(true);
	});

	auto timer = new QTimer(this);
	connect(timer, &QTimer::timeout, [this]() {
		auto x = m_ui->view->scene()->itemsBoundingRect();
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

	auto item = m_ui->view->scene()->addPixmap({path});
	item->setTransformationMode(Qt::SmoothTransformation);
	item->setFlag(QGraphicsItem::ItemIsMovable);
	item->setFlag(QGraphicsItem::ItemIsSelectable);
	item->setTransformOriginPoint(item->boundingRect().width() / 2, item->boundingRect().height() / 2);
	item->setX(item->boundingRect().width() / -2);
	item->setY(item->boundingRect().height() / -2);
	item->setZValue(item->topLevelItem()->zValue() + 1.0);
}

namespace semi_gcodes {
struct laser_on {};
struct laser_off {};
struct home {};

struct dwell {
	i16 delay;
};

struct move {
	i16 x;
	i16 y;
};

struct power {
	i16 duty;
};

using gcode = std::variant<std::monostate, laser_on, laser_off, home, dwell, move, power>;
using gcodes = std::vector<gcode>;

//static_assert (sizeof (gcode_variant) == sizeof(i16) + sizeof(i16));
}

semi_gcodes::gcodes semi_gcode_generator(const QImage &pixmap) {
	using namespace semi_gcodes;

	gcodes ret;

	constexpr static auto ir_size = sizeof(gcode);
	constexpr static auto ir_extra = ir_size * 100;
	const auto w = static_cast<std::size_t>(pixmap.width());
	const auto h = static_cast<std::size_t>(pixmap.height());

	ret.reserve(ir_size * w * h + ir_extra);

	auto emplace = [&ret](auto &&value) {
		ret.emplace_back(std::move(value));
	};

	emplace(semi_gcodes::home{});
	emplace(semi_gcodes::power{0});
	emplace(semi_gcodes::laser_on{});

	for (std::size_t y = 0; y < h; ++y) {
		for (std::size_t x = 0; x < w; ++x) {
			auto light = pixmap.pixelColor(static_cast<int>(x), static_cast<int>(y)).lightnessF();

			if (light >= 0.99)
				continue;

			ret.emplace_back(move{static_cast<decltype(move::x)>(x), static_cast<decltype(move::y)>(y)});
			ret.emplace_back(power{static_cast<i16>((1.0 - light) * 1000.0)});
			ret.emplace_back(dwell{1});
			ret.emplace_back(power{0});
		}
	}

	emplace(semi_gcodes::power{0});
	emplace(semi_gcodes::laser_off{});
	emplace(semi_gcodes::home{});

	return ret;
}

constexpr double precision_multiplier(double dpi = 600) {
	return dpi / 25.4;
}

class gcode_generator {
public:
	gcode_generator(std::ostream &stream, const double dpi = 600)
			: m_stream(stream)
			, m_precision(precision_multiplier(dpi)) {}

	void operator()(const semi_gcodes::dwell &value) { m_stream << "G4 P0.00" << value.delay << std::endl; }
	void operator()(semi_gcodes::home) { m_stream << "G0 X0 Y0" << std::endl; }
	void operator()(semi_gcodes::laser_off) { m_stream << "M5" << std::endl; }
	void operator()(semi_gcodes::laser_on) { m_stream << "M3" << std::endl; }
	void operator()(const semi_gcodes::move &value) { m_stream << "G0 X" << (static_cast<double>(value.x) / m_precision) << " Y" << (static_cast<double>(value.y) / m_precision) << std::endl; }
	void operator()(const semi_gcodes::power &value) { m_stream << "S" << value.duty << std::endl; }
	void operator()(std::monostate) {}

private:
	std::ostream &m_stream;
	const double m_precision{1.0};
};

void generate_gcode(std::string &&dir, semi_gcodes::gcodes &&gcodes) {
	using namespace semi_gcodes;

	std::ofstream file(dir + "/result.gcode", std::ios::trunc);

	gcode_generator visitor(file);

	for (auto &&gcode : gcodes)
		std::visit(visitor, gcode);
}

#include <QFile>
#include <QSerialPort>
#include <QTextStream>

void MainWindow::print() {
	auto scene = m_ui->view->scene();
	auto rect = scene->itemsBoundingRect().toRect();
	rect.moveTopLeft({0, 0});
	QPixmap canvas(rect.width(), rect.height());
	canvas.fill(Qt::white);
	QPainter painter(&canvas);
	scene->clearSelection();
	dynamic_cast<GridScene *>(scene)->setDisableBackground(true);
	scene->render(&painter, canvas.rect(), scene->itemsBoundingRect());
	dynamic_cast<GridScene *>(scene)->setDisableBackground(false);
	//canvas.save(QDir::homePath() + QDir::separator() + "result.png");
	generate_gcode(QDir::homePath().toStdString(), semi_gcode_generator(canvas.toImage()));

	QFile file(QDir::homePath() + QDir::separator() + "result.gcode");
	file.open(QFile::ReadWrite | QFile::Text);
	QSerialPort port("/dev/ttyUSB0");
	port.setBaudRate(115200);
	port.open(QSerialPort::ReadWrite);

	QTextStream in(&file);
	while (!in.atEnd()) {
		QString line = in.readLine();
		std::cout <<  "w: " << line.toUtf8().toStdString() << std::endl;
		port.write(line.toLatin1());
		port.write("\n\r");
		port.waitForBytesWritten();
		port.waitForReadyRead();
		auto response = port.readLine();
		std::cout <<  "r: " << response.toStdString() << std::endl;
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
	if (isItemSelected())
		delete m_selectedItem;
}

void MainWindow::updateAngle(int value) {
	if (isItemSelected())
		m_selectedItem->setRotation(value);

	m_ui->angle->setValue(value);
	m_ui->angleDial->setValue(value);
}

void MainWindow::updateOpacity(int value) {
	if (isItemSelected())
		m_selectedItem->setOpacity(static_cast<double>(value) / 100.0);

	m_ui->opacity->setValue(value);
	m_ui->opacitySlider->setValue(value);
}

MainWindow::~MainWindow() = default;
