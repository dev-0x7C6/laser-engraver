#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QTimer>

#include <grid-scene.h>
#include <chrono>

using namespace std::chrono_literals;

MainWindow::MainWindow(QWidget *parent)
		: QMainWindow(parent)
		, m_ui(std::make_unique<Ui::MainWindow>()) {
	m_ui->setupUi(this);

	auto menu = m_ui->menu;
	auto file = menu->addMenu("&File");
	auto open = file->addAction("&Open", [this]() {
		auto path = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Image Files (*.png *.jpg *.bmp)"));

		if (path.isEmpty())
			return;

		auto item = m_ui->view->scene()->addPixmap({path});
		item->setFlag(QGraphicsItem::ItemIsMovable);
		item->setFlag(QGraphicsItem::ItemIsSelectable);
		item->setTransformOriginPoint(item->boundingRect().width() / 2, item->boundingRect().height() / 2);
		item->setX(item->boundingRect().width() / -2);
		item->setY(item->boundingRect().height() / -2);
		item->setZValue(item->topLevelItem()->zValue() + 1.0);
	});
	file->addSeparator();
	auto exit = file->addAction("&Close", [this]() { close(); });

	open->setShortcuts(QKeySequence::Open);
	open->setIcon(QIcon::fromTheme("document-open"));
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
		////m_ui->view->set
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

bool MainWindow::isItemSelected() const noexcept {
	return m_selectedItem != nullptr;
}

void MainWindow::itemMoveTop() {
	if (m_selectedItem)
		m_selectedItem->setZValue(m_selectedItem->topLevelItem()->zValue() + 1.0);
}

void MainWindow::removeItem()
{
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
