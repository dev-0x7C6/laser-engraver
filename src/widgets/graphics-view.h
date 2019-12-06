#pragma once

#include <QGraphicsView>

class GraphicsView : public QGraphicsView {
public:
	GraphicsView(QWidget *parent = nullptr);

protected:
	void resizeEvent(QResizeEvent *) final;
	void showEvent(QShowEvent *) final;
	void wheelEvent(QWheelEvent *) final;

private:
	void center();
};
