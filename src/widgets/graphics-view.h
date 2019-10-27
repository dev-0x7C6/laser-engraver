#pragma once

#include <QGraphicsView>

class GraphicsView : public QGraphicsView {
public:
	GraphicsView(QWidget *parent = nullptr);

protected:
	void wheelEvent(QWheelEvent *event) final;
};
