#pragma once

#include <QGraphicsView>

class GraphicsView : public QGraphicsView {
public:
	GraphicsView(QWidget *parent = nullptr);

protected:
	void showEvent(QShowEvent *) final;
	void wheelEvent(QWheelEvent *) final;
};
