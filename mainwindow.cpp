#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsScene>
#include <QFileDialog>
#include <QDir>

#include <grid-scene.h>

#include <QGraphicsPixmapItem>

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
		item->setOpacity(0.5);
	});
	file->addSeparator();
	auto exit = file->addAction("&Close", [this]() { close(); });

	open->setShortcuts(QKeySequence::Open);
	exit->setShortcuts(QKeySequence::Quit);

	constexpr auto grid_size = 5000;

	auto scene = std::make_unique<GridScene>(-grid_size, -grid_size, grid_size * 2, grid_size * 2);
	m_ui->view->setScene(scene.release());

	for (auto &&v : {25, 50, 100, 200, 400}) {
		m_ui->scale->addItem(QString::number(v) + "%", v);
	}

	m_ui->scale->setCurrentText("100%");

	connect(m_ui->grid, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](auto &&value) {
		auto scene = dynamic_cast<GridScene*>(m_ui->view->scene());
		scene->setGridSize(value);
	});

	connect(m_ui->scale, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](auto &&index) {
		const auto v = static_cast<double>(m_ui->scale->itemData(index).toInt()) / 100.0;
		m_ui->view->resetTransform();
		m_ui->view->scale(v, v);
		////m_ui->view->set
	});
}

MainWindow::~MainWindow() = default;
