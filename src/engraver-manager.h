#pragma once

#include <QObject>
#include <src/engraver-settings.h>

class QSettings;

class EngraverManager : public QObject {
	Q_OBJECT
public:
	EngraverManager(QSettings &settings, QWidget *parent);
	~EngraverManager();

	void addEngraver();
	std::optional<EngraverSettings> selectEngraver();
	void removeEngraver();

	bool atLeastOneEngraverAvailable() const noexcept;

signals:
	void engraverListChanged();

private:
	QSettings &m_settings;
	QWidget *m_parent;
	Engravers m_engravers;
};
