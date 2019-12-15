#pragma once

#include <QAbstractListModel>

#include <unordered_map>

class QGraphicsItem;

namespace graphical {
namespace object {
enum class type {
	font,
	image,
};

struct properties {
	QGraphicsItem *item{nullptr};
	QString name;
	double order{0.0};
	type flavor;
};
} // namespace object

class model : public QAbstractListModel {
public:
	model() = default;

	void insertObject(graphical::object::properties &&properties);
	void removeObject(QGraphicsItem *id);

	graphical::object::properties value(const QModelIndex &index) const noexcept;

	bool is_empty() const noexcept;

	const auto &values() const noexcept { return m_list; }

private:
	int rowCount(const QModelIndex &parent) const final;
	QVariant data(const QModelIndex &index, int role) const final;
	bool removeRows(int row, int count, const QModelIndex &parent) final;

	std::vector<graphical::object::properties> m_list;
};

}; // namespace graphical
