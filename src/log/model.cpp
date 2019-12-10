#include "model.h"

using namespace log;

namespace {
constexpr auto MAXIMUM_MESSAGE_COUNT = 32;
}

model::model() {
	m_logs.reserve(MAXIMUM_MESSAGE_COUNT * 2);
}

int model::rowCount(const QModelIndex &) const {
	return m_logs.size();
}

QVariant model::data(const QModelIndex &index, int role) const {
	const auto column = index.column();
	auto &&msg = m_logs[index.row()];

	if (Qt::DisplayRole == role && column == 0)
		return msg.date.toString() + ": " + msg.text;

	return {};
}

void model::insert(QString &&text, QDateTime &&date) {
	const auto size = m_logs.size();

	if (m_logs.size() >= MAXIMUM_MESSAGE_COUNT) {
		beginRemoveRows({}, 0, 1);
		m_logs.removeFirst();
		endRemoveRows();
	}

	beginInsertRows({}, size, size + 1);
	m_logs.append(message{std::move(text), std::move(date)});
	endInsertRows();
}
