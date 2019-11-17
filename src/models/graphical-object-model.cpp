#include "graphical-object-model.h"

#include <QGraphicsItem>
#include <QIcon>

void graphical::model::insertObject(graphical::object::properties &&properties) {
	beginInsertRows({}, m_list.size(), m_list.size() + 1);
	m_list.emplace_back(std::move(properties));
	endInsertRows();
}

void graphical::model::removeObject(QGraphicsItem *id) {
	m_list.erase(std::remove_if(m_list.begin(),
					 m_list.end(),
					 [id](auto &&value) -> bool { return value.item == id; }),
		m_list.end());
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
