#include "grid-scene.h"

#include <QPainter>
#include <externals/common/qt/raii/raii-painter.hpp>

GridScene::GridScene(qreal x, qreal y, qreal w, qreal h)
		: QGraphicsScene(x, y, w, h) {}

void GridScene::setDisableBackground(bool value) noexcept {
	m_disableBackground = value;
}

void GridScene::setGridSize(int size) noexcept {
	m_gridSize = size;
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
