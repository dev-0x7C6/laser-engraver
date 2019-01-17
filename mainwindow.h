#pragma once

#include <QAction>
#include <QMainWindow>

#include <memory>

class QGraphicsItem;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	MainWindow(const MainWindow &) = delete;
	MainWindow(MainWindow &&) = delete;

	MainWindow &operator=(const MainWindow &) = delete;
	MainWindow &operator=(MainWindow &&) = delete;

	~MainWindow() final;

private:
	void open();
	void print();

private:
	bool isItemSelected() const noexcept;

	void itemMoveTop();
	void removeItem();

	void updateAngle(int value);
	void updateOpacity(int value);

private:
	std::unique_ptr<Ui::MainWindow> m_ui;
	QGraphicsItem *m_selectedItem{nullptr};
};
