#pragma once

#include <memory>

#include <QActionGroup>
#include <QElapsedTimer>
#include <QMainWindow>
#include <QSettings>

#include <externals/common/std/raii/raii-tail-call.hpp>
#include <src/engraver-connection.h>
#include <src/engraver-manager.h>
#include <src/engraver/spindle/manager.h>
#include <src/gui-settings.h>
#include <src/log/model.h>
#include <src/sheets.hpp>

class Workspace;
class QGraphicsItem;
class QCheckBox;

namespace Ui {
class MainWindow;
}

class MainWindow final : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	MainWindow(const MainWindow &) = delete;
	MainWindow(MainWindow &&) = delete;

	MainWindow &operator=(const MainWindow &) = delete;
	MainWindow &operator=(MainWindow &&) = delete;

	~MainWindow();

	[[nodiscard]] auto prepareImage() -> QImage;

private:
	void toggleFullscreen();
	void toggleSpindle();

	void insertImageObject();
	void insertTextObject();

	[[nodiscard]] bool prepare();
	void print();
	void preview();
	void connectEngraver();
	void disconnectEngraver();
	void applyMovementSettings();
	void updateSheetReferences();
	void editLabelObject();

	void saveAs();

private:
	bool is_scene_ready();
	bool is_connection_ready();

private:
	void zoomInObject();
	void zoomOutObject();

private:
	[[nodiscard]] auto is_connected() const noexcept -> bool;

	[[nodiscard]] auto gcode_opts_from_ui() const noexcept -> gcode::options;
	[[nodiscard]] auto safety_gcode_raii() noexcept -> raii_tail_call;
	[[nodiscard]] auto semi_opts_from_ui() const noexcept -> semi::options;

	void append_log(QString log);

	void updateItemAngle(int value);
	void updateItemOpacity(int value);
	void updateItemScale(double value) noexcept;

private:
	std::unique_ptr<Ui::MainWindow> m_ui;
	std::unique_ptr<logs::model> m_log;
	QSettings m_settings;
	std::unique_ptr<GuiSettings> m_guiSettings;
	std::unique_ptr<EngraverConnection> m_connection;

	engraver::spindle::manager m_spindle;

	QActionGroup m_enableIfEngraverConnected{nullptr};
	QActionGroup m_enableIfObjectIsSelected{nullptr};
	QAction *m_actionConnectEngraver{nullptr};
	QAction *m_actionDisconnectEngraver{nullptr};
	QAction *m_actionLaserState{nullptr};
	int m_append_log_count{0};

	EngraverManager m_engraverManager;
	Workspace *m_grid{nullptr};
};
