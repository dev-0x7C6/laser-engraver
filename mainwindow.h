#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QActionGroup>

#include <memory>

#include <src/engraver-manager.h>
#include <src/engraver-connection.h>

enum class direction {
	up,
	down,
	left,
	right,
	home,
	new_home,
};

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
	void connectEngraver();
	void disconnectEngraver();
	void turnLaser(bool on);
	void applyMovementSettings();

	void go(direction);

private:
	void zoomInObject();
	void zoomOutObject();

private:
	bool isItemSelected() const noexcept;

	void itemMoveTop();
	void removeItem();

	void updateItemAngle(int value);
	void updateItemOpacity(int value);
	void updateItemScale(double value) noexcept;

private:
	std::unique_ptr<Ui::MainWindow> m_ui;
	std::unique_ptr<EngraverConnection> m_connection;

	double m_x{};
	double m_y{};

	QActionGroup m_enableIfEngraverConnected{nullptr};
	QActionGroup m_enableIfObjectIsSelected{nullptr};
	QAction *m_actionConnectEngraver{nullptr};
	QAction *m_actionDisconnectEngraver{nullptr};
	QAction *m_actionLaserOn{nullptr};
	QAction *m_actionLaserOff{nullptr};

	QSettings m_settings;
	EngraverManager m_engraverManager;
	QGraphicsItem *m_selectedItem{nullptr};
	GridScene *m_grid{nullptr};
};
