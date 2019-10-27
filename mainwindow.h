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

struct spindle_position {
	float x{};
	float y{};

	semi_gcodes preview_gcode() noexcept {
		return is_preview_on_state ? semi_gcodes{instruction::laser_on{}, instruction::power{1}} : semi_gcodes{instruction::power{0}, instruction::laser_off{}};
	}

	semi_gcodes set_preview_on(const bool value) noexcept {
		is_preview_on_state = value;
		return preview_gcode();
	}

	constexpr instruction::move_mm move_mm_x(const float step) noexcept {
		return {x += step, y};
	}

	constexpr instruction::move_mm move_mm_y(const float step) noexcept {
		return {x, y += step};
	}

	constexpr instruction::move_mm reset_mm() noexcept {
		return {x = 0.0f, y = 0.0f};
	}

	constexpr instruction::set_home_position reset_home() noexcept {
		return {x = 0.0f, y = 0.0f};
	}

private:
	bool is_preview_on_state{false};
};

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
	void updateSheetReferences();

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

	void command(semi_gcodes &&gcodes);

private:
	std::unique_ptr<Ui::MainWindow> m_ui;
	std::unique_ptr<EngraverConnection> m_connection;

	spindle_position position{};

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
