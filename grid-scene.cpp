#include "grid-scene.h"

#include <QPainter>
#include <externals/common/qt/raii/raii-painter.hpp>

GridScene::GridScene(qreal x, qreal y, qreal w, qreal h)
		: QGraphicsScene(x, y, w, h) {
}

void GridScene::setDisableBackground(bool value) noexcept {
	m_disableBackground = value;
}

void GridScene::setGridSize(int size) noexcept {
	m_gridSize = size;
	update();
}

void GridScene::drawSheetAreas(std::vector<sheet_metrics> &&papers) {
	m_papers = std::move(papers);
	update();
}

void GridScene::updateDpi(double dpi) {
	m_dpi = dpi;
	update();
}

void GridScene::drawBackground(QPainter *painter, const QRectF &rect) {
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

void GridScene::drawSheet(QPainter *painter, const sheet_metrics &sheet) noexcept {
	raii_painter _(painter);
	constexpr auto inch = 25.4;
	const auto iw = sheet.invert ? sheet.h : sheet.w;
	const auto ih = sheet.invert ? sheet.w : sheet.h;
	const auto w = (m_dpi * iw) / inch;
	const auto h = (m_dpi * ih) / inch;
	auto pen = painter->pen();
	pen.setStyle(Qt::PenStyle::DashLine);
	pen.setColor(Qt::white);
	painter->setPen(pen);

	painter->drawRect(-w / 2, -h / 2, w, h);
	painter->drawText(-w / 2, h / 2 + painter->fontMetrics().ascent(), QString("%1: %2mm x %3mm (%4px x %5px)").arg(QString::fromStdString(sheet.name), QString::number(iw), QString::number(ih), QString::number(w), QString::number(h)));
}

void GridScene::drawGrid(QPainter *painter, const QRectF &rect) noexcept {
	raii_painter _(painter);
	qreal left = int(rect.left()) - (int(rect.left()) % m_gridSize);
	qreal top = int(rect.top()) - (int(rect.top()) % m_gridSize);

	QVarLengthArray<QLineF, 1024> lines;

	QPen pen = painter->pen();
	auto color = pen.color();
	color.setAlphaF(0.10);
	pen.setColor(color);
	painter->setPen(pen);

	for (auto x = left; x < rect.right(); x += m_gridSize)
		lines.append(QLineF(x, rect.top(), x, rect.bottom()));
	for (auto y = top; y < rect.bottom(); y += m_gridSize)
		lines.append(QLineF(rect.left(), y, rect.right(), y));

	painter->drawLines(lines.data(), lines.size());
}

void GridScene::drawXAxis(QPainter *painter, QRect &&scene) noexcept {
	raii_painter _(painter);
	painter->setPen(m_xAxisColor);
	painter->drawLine(scene.left(), 0, scene.right(), 0);
}

void GridScene::drawYAxis(QPainter *painter, QRect &&scene) noexcept {
	raii_painter _(painter);
	painter->setPen(m_yAxisColor);
	painter->drawLine(0, scene.top(), 0, scene.bottom());
}
