#include "src/workspace.h"

#include <QFile>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QPainter>

#include <externals/common/qt/raii/raii-painter.hpp>

namespace {
constexpr auto inch = 25.4;
}

Workspace::Workspace(qreal x, qreal y, qreal w, qreal h)
		: QGraphicsScene(x, y, w, h)
		, m_model(std::make_unique<graphical::model>()) {
	connect(this, &QGraphicsScene::selectionChanged, [this]() {
		auto list = selectedItems();
		const auto enabled = !list.isEmpty();

		if (enabled)
			m_selected_object = list.first();

		emit objectSelectionChanged(enabled);
		emit objectSelectionChanged(m_selected_object);
	});
}

void Workspace::setDisableBackground(bool value) noexcept {
	m_disableBackground = value;
}

void Workspace::setGridSize(double size) noexcept {
	m_gridSize = size;
	update();
}

void Workspace::drawSheetAreas(std::vector<inverter<sheet::metrics>> &&papers) {
	m_papers = std::move(papers);
	update();
}

void Workspace::updateDpi(double dpi) {
	m_dpi = dpi;
	update();
}

bool Workspace::insertPixmapObject(const QString &path) noexcept {
	if (!QFile::exists(path))
		return false;

	auto item = addPixmap({path});
	item->setTransformationMode(Qt::SmoothTransformation);
	setCommonObjectParameters(item);

	graphical::object::properties properties;
	properties.item = item;
	properties.name = path;
	properties.order = item->zValue();
	properties.flavor = graphical::object::type::image;

	m_model->insertObject(std::move(properties));

	return true;
}

void Workspace::insertTextObject(const TextWithFont &property) noexcept {
	auto item = addText(property.text, property.font);
	item->setDefaultTextColor(Qt::black);
	setCommonObjectParameters(item);

	graphical::object::properties properties;
	properties.item = item;
	properties.name = property.text;
	properties.order = item->zValue();
	properties.flavor = graphical::object::type::font;

	m_model->insertObject(std::move(properties));
}

void Workspace::remove(QGraphicsItem *item) noexcept {
	if (m_selected_object == item) {
		m_selected_object = nullptr;
	}

	m_model->removeObject(item);
	delete item;
}

void Workspace::selected_object_move_up() {
	selected_object()->setZValue(selected_object()->topLevelItem()->zValue() + 1.0);
}

void Workspace::selected_object_move_down() {
	selected_object()->setZValue(selected_object()->topLevelItem()->zValue() - 1.0);
}

void Workspace::selected_object_center() {
	selected_object()->setPos(-selected_object()->boundingRect().width() / 2.0, -selected_object()->boundingRect().height() / 2.0);
}

void Workspace::selected_object_remove() {
	removeItem(selected_object());
}

graphical::model *Workspace::model() {
	return m_model.get();
}

QGraphicsItem *Workspace::selected_object() {
	return m_selected_object;
}

void Workspace::drawBackground(QPainter *painter, const QRectF &rect) {
	raii_painter _(painter);
	painter->setRenderHint(QPainter::Antialiasing);
	painter->setRenderHint(QPainter::TextAntialiasing);
	painter->setRenderHint(QPainter::SmoothPixmapTransform);
	painter->setRenderHint(QPainter::HighQualityAntialiasing);

	if (m_disableBackground)
		return;

	if (m_gridSize)
		drawGrid(painter, rect);

	if (m_xAxisEnabled)
		drawXAxis(painter, sceneRect().toRect());

	if (m_yAxisEnabled)
		drawYAxis(painter, sceneRect().toRect());

	for (auto &&paper : m_papers)
		drawSheet(painter, paper);
}

void Workspace::drawSheet(QPainter *painter, const inverter<sheet::metrics> &sheet) noexcept {
	raii_painter _(painter);
	const auto iw = sheet.inverted ? sheet.value.h : sheet.value.w;
	const auto ih = sheet.inverted ? sheet.value.w : sheet.value.h;
	const auto w = (m_dpi * iw) / inch;
	const auto h = (m_dpi * ih) / inch;
	auto pen = painter->pen();
	pen.setStyle(Qt::PenStyle::DashLine);
	pen.setColor(Qt::white);
	painter->setPen(pen);

	painter->drawRect(-w / 2, -h / 2, w, h);
	painter->drawText(-w / 2, h / 2 + painter->fontMetrics().ascent(), QString("%1: %2mm x %3mm (%4px x %5px)").arg(QString::fromStdString(sheet.value.name), QString::number(iw), QString::number(ih), QString::number(w), QString::number(h)));
}

void Workspace::setCommonObjectParameters(QGraphicsItem *item) {
	item->setFlag(QGraphicsItem::ItemIsMovable);
	item->setFlag(QGraphicsItem::ItemIsSelectable);
	item->setTransformOriginPoint(item->boundingRect().width() / 2, item->boundingRect().height() / 2);
	item->setX(item->boundingRect().width() / -2);
	item->setY(item->boundingRect().height() / -2);
	item->setZValue(item->topLevelItem()->zValue() + 1.0);
}

void Workspace::drawGrid(QPainter *painter, const QRectF &rect) noexcept {
	raii_painter _(painter);
	const auto grid = static_cast<int>((m_dpi * m_gridSize) / inch);

	qreal left = int(rect.left()) - (int(rect.left()) % grid);
	qreal top = int(rect.top()) - (int(rect.top()) % grid);

	QVarLengthArray<QLineF, 1024> lines;

	QPen pen = painter->pen();
	auto color = pen.color();
	color.setAlphaF(0.10);
	pen.setColor(color);
	painter->setPen(pen);

	for (auto x = left; x < rect.right(); x += grid)
		lines.append(QLineF(x, rect.top(), x, rect.bottom()));
	for (auto y = top; y < rect.bottom(); y += grid)
		lines.append(QLineF(rect.left(), y, rect.right(), y));

	painter->drawLines(lines.data(), lines.size());
}

void Workspace::drawXAxis(QPainter *painter, QRect &&scene) noexcept {
	raii_painter _(painter);
	painter->setPen(m_xAxisColor);
	painter->drawLine(scene.left(), 0, scene.right(), 0);
}

void Workspace::drawYAxis(QPainter *painter, QRect &&scene) noexcept {
	raii_painter _(painter);
	painter->setPen(m_yAxisColor);
	painter->drawLine(0, scene.top(), 0, scene.bottom());
}
