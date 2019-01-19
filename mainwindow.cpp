#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QTimer>

#include <grid-scene.h>

#include <chrono>
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
struct go_home {};

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

using gcode_variant = std::variant<std::monostate, laser_on, laser_off, go_home, dwell, move, power>;

//static_assert (sizeof (gcode_variant) == sizeof(i16) + sizeof(i16));
}

std::vector<semi_gcodes::gcode_variant> semi_gcode_generator(const QImage &pixmap) {
	std::vector<semi_gcodes::gcode_variant> ret;

	constexpr static auto ir_size = sizeof(semi_gcodes::gcode_variant);
	constexpr static auto ir_extra = ir_size * 100;
	const auto w = static_cast<std::size_t>(pixmap.width());
	const auto h = static_cast<std::size_t>(pixmap.height());

	ret.reserve(ir_size * w * h + ir_extra);

	auto emplace = [&ret](auto &&value) {
		ret.emplace_back(std::move(value));
	};

	emplace(semi_gcodes::go_home{});
	emplace(semi_gcodes::power{0});
	emplace(semi_gcodes::laser_on{});

	for (std::size_t y = 0; y < h; ++y) {
		for (std::size_t x = 0; x < w; ++x) {
			auto light = pixmap.pixelColor(static_cast<int>(x), static_cast<int>(y)).lightness();

			if (light == 0xff)
				continue;

			ret.emplace_back(semi_gcodes::move{static_cast<decltype(semi_gcodes::move::x)>(x), static_cast<decltype(semi_gcodes::move::y)>(y)});
			ret.emplace_back(semi_gcodes::power{static_cast<i16>(light * 2)});
			ret.emplace_back(semi_gcodes::dwell{1});
			ret.emplace_back(semi_gcodes::power{0});
		}
	}

	emplace(semi_gcodes::power{0});
	emplace(semi_gcodes::laser_off{});
	emplace(semi_gcodes::go_home{});

	return ret;
}

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
	canvas.save(QDir::homePath() + QDir::separator() + "result.png");
	auto semi = semi_gcode_generator(canvas.toImage());
	std::cout << semi.size() << std::endl;
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
