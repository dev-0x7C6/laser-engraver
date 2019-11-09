#pragma once

#include <QGraphicsScene>

#include <src/dialogs/font-dialog.h>
#include <src/sheets.hpp>

class Workspace : public QGraphicsScene {
public:
	Workspace(qreal x, qreal y, qreal w, qreal h);

	void setDisableBackground(bool value) noexcept;
	void setGridSize(double size) noexcept;

	void drawSheetAreas(std::vector<inverter<sheet::metrics>> &&papers);
	void updateDpi(double dpi);

	bool insertPixmapObject(const QString &) noexcept;
	void insertTextObject(const TextWithFont &) noexcept;

protected:
	void drawBackground(QPainter *painter, const QRectF &rect) final;

private:
	void drawGrid(QPainter *painter, const QRectF &rect) noexcept;
	void drawXAxis(QPainter *painter, QRect &&scene) noexcept;
	void drawYAxis(QPainter *painter, QRect &&scene) noexcept;
	void drawSheet(QPainter *painter, const inverter<sheet::metrics> &) noexcept;
	void setCommonObjectParameters(QGraphicsItem *);

private:
	std::vector<inverter<sheet::metrics>> m_papers;
	double m_dpi{300.0};

	double m_gridSize{10.00};

	QColor m_xAxisColor{0xff, 0x00, 0x00, 0x9f};
	QColor m_yAxisColor{0x00, 0xff, 0x00, 0x9f};

	bool m_disableBackground{false};
	bool m_xAxisEnabled{true};
	bool m_yAxisEnabled{true};
};
