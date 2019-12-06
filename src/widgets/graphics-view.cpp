#include "graphics-view.h"

GraphicsView::GraphicsView(QWidget *parent)
		: QGraphicsView(parent) {
	setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::MinimalViewportUpdate);
}

void GraphicsView::resizeEvent(QResizeEvent *event) {
	center();
	QGraphicsView::resizeEvent(event);
}

void GraphicsView::showEvent(QShowEvent *event) {
	center();
	QGraphicsView::showEvent(event);
}

void GraphicsView::wheelEvent(QWheelEvent *) {} //eat event

void GraphicsView::center() {
	centerOn(0.0, 0.0);
}
