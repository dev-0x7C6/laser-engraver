#include "engraver-movement-settings-widget.h"
#include "ui_engraver-movement-settings-widget.h"

EngraverMovementSettingsWidget::EngraverMovementSettingsWidget(QWidget *parent)
		: QWidget(parent)
		, m_ui(std::make_unique<Ui::EngraverMovementSettingsWidget>()) {
	m_ui->setupUi(this);

	for (auto &&widget : {m_ui->x_steps, m_ui->y_steps, m_ui->z_steps, m_ui->x_max_speed, m_ui->y_max_speed, m_ui->z_max_speed, m_ui->x_acceleration, m_ui->y_acceleration, m_ui->z_acceleration})
		connect(widget, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](auto &&) {
			emit settingsChanged();
		});
}

void EngraverMovementSettingsWidget::setParameters(const EngraverParameters &parameters) {
	m_ui->x_max_speed->setValue(parameters.x_max_speed);
	m_ui->y_max_speed->setValue(parameters.y_max_speed);
	m_ui->z_max_speed->setValue(parameters.z_max_speed);
	m_ui->x_steps->setValue(parameters.x_steps_per_mm);
	m_ui->y_steps->setValue(parameters.y_steps_per_mm);
	m_ui->z_steps->setValue(parameters.z_steps_per_mm);
	m_ui->x_acceleration->setValue(parameters.x_acceleration);
	m_ui->y_acceleration->setValue(parameters.y_acceleration);
	m_ui->z_acceleration->setValue(parameters.z_acceleration);
}

EngraverParameters EngraverMovementSettingsWidget::parameters() {
	EngraverParameters ret;
	ret.x_max_speed = m_ui->x_max_speed->value();
	ret.y_max_speed = m_ui->y_max_speed->value();
	ret.z_max_speed = m_ui->z_max_speed->value();
	ret.x_steps_per_mm = m_ui->x_steps->value();
	ret.y_steps_per_mm = m_ui->y_steps->value();
	ret.z_steps_per_mm = m_ui->z_steps->value();
	ret.x_acceleration = m_ui->x_acceleration->value();
	ret.y_acceleration = m_ui->y_acceleration->value();
	ret.z_acceleration = m_ui->z_acceleration->value();
	return ret;
}

EngraverMovementSettingsWidget::~EngraverMovementSettingsWidget() = default;
