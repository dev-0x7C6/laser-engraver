#include "graphical-object-model.h"

#include <QGraphicsItem>
#include <QIcon>

#include <algorithm>

void graphical::model::insertObject(graphical::object::properties &&properties) {
	beginInsertRows({}, m_list.size(), m_list.size() + 1);
	m_list.emplace_back(std::move(properties));
	endInsertRows();
}

void graphical::model::removeObject(QGraphicsItem *id) {
	if (auto it = std::find_if(m_list.cbegin(), m_list.cend(), [id](auto &&value) -> bool { return value.item == id; }); it != m_list.end()) {
		removeRows(std::distance(m_list.cbegin(), it), 1, {});
	}
}

graphical::object::properties graphical::model::value(const QModelIndex &index) const noexcept {
	return m_list[index.row()];
}

bool graphical::model::is_empty() const noexcept {
	return rowCount({}) == 0;
}

int graphical::model::rowCount(const QModelIndex &) const {
	return m_list.size();
}

QVariant graphical::model::data(const QModelIndex &index, int role) const {
	if (index.column() != 0)
		return {};

	auto &&value = m_list[index.row()];

	if (Qt::DisplayRole == role)
		return value.name;

	if (Qt::DecorationRole == role) {
		switch (value.flavor) {
			case graphical::object::type::font:
				return QIcon::fromTheme("font-x-generic");
			case graphical::object::type::image:
				return QIcon::fromTheme("image-x-generic");
		}
	}

	return {};
}

bool graphical::model::removeRows(int row, int count, const QModelIndex &parent) {
	beginRemoveRows({}, row, row + count - 1);
	m_list.erase(m_list.begin() + row);
	endRemoveRows();
	return true;
}
