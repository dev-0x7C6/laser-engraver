#pragma once

#include <QGraphicsScene>

class GridScene : public QGraphicsScene {
public:
	GridScene(qreal x, qreal y, qreal w, qreal h);

	void setDisableBackground(bool value) noexcept;
	void setGridSize(int size) noexcept;

protected:
	void drawBackground(QPainter *painter, const QRectF &rect) final;

private:
	void drawGrid(QPainter *painter, const QRectF &rect) noexcept;
	void drawXAxis(QPainter *painter, QRect &&scene) noexcept;
	void drawYAxis(QPainter *painter, QRect &&scene) noexcept;

private:
	bool m_disableBackground{false};

	int m_gridSize{10};

	QColor m_xAxisColor{0xff, 0x00, 0x00, 0x9f};
	QColor m_yAxisColor{0x00, 0xff, 0x00, 0x9f};

	bool m_xAxisEnabled{true};
	bool m_yAxisEnabled{true};
};
