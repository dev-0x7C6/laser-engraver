#pragma once

#include <memory>

#include <QActionGroup>
#include <QMainWindow>
#include <QSettings>

#include <externals/common/std/raii/raii-tail-call.hpp>
#include <src/engraver-connection.h>
#include <src/engraver-manager.h>
#include <src/sheets.hpp>
#include <src/spindle-position.hpp>

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
	void preview();
	void connectEngraver();
	void disconnectEngraver();
	void turnLaser(bool on);
	void applyMovementSettings();
	void updateSheetReferences();

private:
	void zoomInObject();
	void zoomOutObject();

private:
	bool is_connected() const noexcept;
	raii_tail_call safety_gcode_raii() noexcept;
	gcode_generation_options make_gcode_generation_options_from_ui() const noexcept;
	semi::options make_semi_options_from_ui() const noexcept;

	bool isItemSelected() const noexcept;

	void itemMoveTop();
	void removeItem();

	void updateItemAngle(int value);
	void updateItemOpacity(int value);
	void updateItemScale(double value) noexcept;

	void command(semi::gcodes &&gcodes);

	float moveStep() const noexcept;

private:
	std::unique_ptr<Ui::MainWindow> m_ui;
	std::unique_ptr<EngraverConnection> m_connection;

	engraver::helper::spindle_position spindle_position{};

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
