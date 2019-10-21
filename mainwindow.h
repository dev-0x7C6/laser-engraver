#pragma once

#include <QMainWindow>
#include <QSettings>

#include <memory>

#include <src/engraver-settings.h>

class GridScene;
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

	QImage prepareImage();

private:
	void open();
	void print();
	void addEngraver();

private:
	bool isItemSelected() const noexcept;

	void itemMoveTop();
	void removeItem();

	void updateItemAngle(int value);
	void updateItemOpacity(int value);
	void updateItemScale(double value) noexcept;

private:
	std::unique_ptr<Ui::MainWindow> m_ui;
	std::vector<EngraverSettings> m_engravers;
	QSettings m_settings;
	QGraphicsItem *m_selectedItem{nullptr};
	GridScene *m_grid{nullptr};
};
