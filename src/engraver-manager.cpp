#include "engraver-manager.h"

#include <src/select-engraver-dialog.h>
#include <src/add-engraver-dialog.h>

EngraverManager::EngraverManager(QSettings &settings, QWidget *parent)
		: m_settings(settings)
		, m_parent(parent) {
	m_settings.beginGroup("devices");
	for (auto &&key : m_settings.childGroups()) {
		m_settings.beginGroup(key);
		m_engravers.emplace_back(load(m_settings));
		m_settings.endGroup();
	}
	m_settings.endGroup();
}

EngraverManager::~EngraverManager() {
	m_settings.beginGroup("devices");
	for (auto &&engraver : m_engravers) {
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
		m_engravers.emplace_back(dialog.result().value());
		emit engraverListChanged();
	}
}

std::optional<EngraverSettings> EngraverManager::selectEngraver() {
	SelectEngraverDialog dialog(m_engravers, m_parent);
	dialog.exec();
	return dialog.result();
}

void EngraverManager::removeEngraver() {
	if (auto engraver = selectEngraver(); engraver) {
		m_engravers.erase(
			std::remove_if(m_engravers.begin(), m_engravers.end(), [&engraver](auto &&match) { return match.name == engraver->name; }),
			m_engravers.end());

		m_settings.remove("devices/" + engraver->name);
		emit engraverListChanged();
	}
}

bool EngraverManager::atLeastOneEngraverAvailable() const noexcept {
	return !m_engravers.empty();
}
