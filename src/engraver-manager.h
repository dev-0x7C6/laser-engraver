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
	std::optional<engraver::settings::configuration> selectEngraver();
	void removeEngraver();

	void update(const QString &name, const engraver::settings::movement_parameters &parameters);

	bool atLeastOneEngraverAvailable() const noexcept;

signals:
	void engraverListChanged();

private:
	QSettings &m_settings;
	QWidget *m_parent;
	engraver::settings::configurations m_configurations;
};
