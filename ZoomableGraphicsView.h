#pragma once
#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>

class ZoomableGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ZoomableGraphicsView(QWidget* parent = nullptr);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    double m_totalScale = 1.0;
    const double m_scaleStep = 1.15;       // 每次放大/缩小比例
    const double m_maxScale = 8.0;
    const double m_minScale = 0.15;
    QPoint m_lastMousePos;
    bool m_dragging = false;
};