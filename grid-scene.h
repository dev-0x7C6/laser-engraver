#pragma once

#include <QGraphicsScene>

struct sheet_metrics {
	std::string name;
	double w{};
	double h{};
	bool invert{false};
};

class GridScene : public QGraphicsScene {
public:
	GridScene(qreal x, qreal y, qreal w, qreal h);

	void setDisableBackground(bool value) noexcept;
	void setGridSize(int size) noexcept;

	void drawSheetAreas(std::vector<sheet_metrics> &&papers);
	void updateDpi(double dpi);

protected:
	void drawBackground(QPainter *painter, const QRectF &rect) final;

private:
	void drawGrid(QPainter *painter, const QRectF &rect) noexcept;
	void drawXAxis(QPainter *painter, QRect &&scene) noexcept;
	void drawYAxis(QPainter *painter, QRect &&scene) noexcept;
	void drawSheet(QPainter *painter, const sheet_metrics &) noexcept;

private:
	std::vector<sheet_metrics> m_papers;
	double m_dpi{300.0};

	int m_gridSize{10};

	QColor m_xAxisColor{0xff, 0x00, 0x00, 0x9f};
	QColor m_yAxisColor{0x00, 0xff, 0x00, 0x9f};

	bool m_disableBackground{false};
	bool m_xAxisEnabled{true};
	bool m_yAxisEnabled{true};
};
