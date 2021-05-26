#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QDateTime>

namespace logs {

struct message {
	QString text;
	QDateTime date;
};

class model : public QAbstractListModel {
public:
	model();

	int rowCount(const QModelIndex &parent) const final;
	QVariant data(const QModelIndex &index, int role) const final;

	void insert(QString &&text, QDateTime &&date);

private:
	QList<message> m_logs;
};

} // namespace logs
