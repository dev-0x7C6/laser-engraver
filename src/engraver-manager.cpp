#include "engraver-manager.h"

#include <src/select-engraver-dialog.h>
#include <src/add-engraver-dialog.h>

EngraverManager::EngraverManager(QSettings &settings, QWidget *parent)
		: m_settings(settings)
		, m_parent(parent) {
	m_settings.beginGroup("devices");
	for (auto &&key : m_settings.childGroups()) {
		m_settings.beginGroup(key);
		m_configurations.emplace_back(engraver::settings::load(m_settings));
		m_settings.endGroup();
	}
	m_settings.endGroup();
}

EngraverManager::~EngraverManager() {
	m_settings.beginGroup("devices");
	for (auto &&engraver : m_configurations) {
		m_settings.beginGroup(engraver.name);
		save(m_settings, engraver);
		m_settings.endGroup();
	}
	m_settings.endGroup();
}

void EngraverManager::addEngraver() {
	AddEngraverDialog dialog;
	dialog.exec();

	if (dialog.result()) {
		m_configurations.emplace_back(dialog.result().value());
		emit engraverListChanged();
	}
}

std::optional<engraver::settings::configuration> EngraverManager::selectEngraver() {
	SelectEngraverDialog dialog(m_configurations, m_parent);
	dialog.exec();
	return dialog.result();
}

void EngraverManager::removeEngraver() {
	if (auto engraver = selectEngraver(); engraver) {
		m_configurations.erase(
			std::remove_if(m_configurations.begin(), m_configurations.end(), [&engraver](auto &&match) { return match.name == engraver->name; }),
			m_configurations.end());

		m_settings.remove("devices/" + engraver->name);
		emit engraverListChanged();
	}
}

void EngraverManager::update(const QString &name, const engraver::settings::movement_parameters &parameters) {
	if (auto it = std::find_if(m_configurations.begin(), m_configurations.end(), [&name](auto &&settings) { return settings.name == name; }); it != m_configurations.end())
		it->movement_params = parameters;
}

bool EngraverManager::atLeastOneEngraverAvailable() const noexcept {
	return !m_configurations.empty();
}
