#include "graphics-view.h"

GraphicsView::GraphicsView(QWidget *parent)
		: QGraphicsView(parent) {
}

void GraphicsView::showEvent(QShowEvent *) {
	//setViewport(new QGLWidget(QGLFormat(QGL::SingleBuffer))); //moved to showEvent because of strange random artefacts
	setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::MinimalViewportUpdate);
	centerOn(0.0, 0.0);
}

void GraphicsView::wheelEvent(QWheelEvent *) {
}
