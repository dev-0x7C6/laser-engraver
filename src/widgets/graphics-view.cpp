#include "graphics-view.h"

#include <QGLWidget>

GraphicsView::GraphicsView(QWidget *parent)
		: QGraphicsView(parent) {
	setViewport(new QGLWidget(QGLFormat(QGL::SingleBuffer)));
}

void GraphicsView::wheelEvent(QWheelEvent *) {
}
