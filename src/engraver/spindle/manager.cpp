#include "manager.h"

#include <src/engraver-connection.h>
#include <src/gcode-generator.hpp>

using namespace engraver::spindle;

namespace {
constexpr auto default_move_distance = 10.00f;
}

manager::manager(std::unique_ptr<EngraverConnection> &connection)
		: m_connection(connection) {
}

void manager::set_move_distance_provider(std::function<double()> &&callable) {
	m_move_distance_provider = std::move(callable);
}

void manager::move_up() {
	process({m_position.move_mm_y(-calculate_move_distance())});
}

void manager::move_down() {
	process({m_position.move_mm_y(calculate_move_distance())});
}

void manager::move_left() {
	process({m_position.move_mm_x(-calculate_move_distance())});
}

void manager::move_right() {
	process({m_position.move_mm_x(calculate_move_distance())});
}

void manager::reset_home() {
	process({m_position.reset_home()});
}

void manager::home() {
	m_position.reset_home();
	process({instruction::home{}});
}

void manager::home_and_save() {
	process({m_position.reset_home(), instruction::home{}});
}

void manager::set_preview_mode(const bool state) {
	process(m_position.set_preview_on(state));
}

float manager::calculate_move_distance() {
	if (m_move_distance_provider)
		return static_cast<float>(m_move_distance_provider());

	return default_move_distance;
}

void manager::process(semi::gcodes &&gcodes) {
	gcode::transform(std::move(gcodes), {}, m_connection->process());
}
