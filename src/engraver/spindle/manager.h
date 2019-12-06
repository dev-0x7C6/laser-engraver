#pragma once

#include <QObject>
#include <functional>
#include <memory>

#include <src/spindle-position.hpp>

class EngraverConnection;

namespace engraver {
namespace spindle {
class manager : public QObject {
	Q_OBJECT
public:
	manager(std::unique_ptr<EngraverConnection> &connection);

	void set_move_distance_provider(std::function<double()> &&callable);

	void move_up();
	void move_down();
	void move_left();
	void move_right();

	void reset_home();
	void home();
	void home_and_save();

	void set_preview_mode(bool state);

private:
	float calculate_move_distance();
	void process(semi::gcodes &&gcodes);

private:
	engraver::helper::spindle_position m_position{};
	std::unique_ptr<EngraverConnection> &m_connection;
	std::function<double()> m_move_distance_provider;
};
} // namespace spindle
} // namespace engraver
