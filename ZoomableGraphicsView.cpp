#include "ZoomableGraphicsView.h"

ZoomableGraphicsView::ZoomableGraphicsView(QWidget* parent)
    : QGraphicsView(parent)
{
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
}

void ZoomableGraphicsView::wheelEvent(QWheelEvent* event)
{
    // 支持滚轮缩放
    double oldScale = m_totalScale;
    if (event->angleDelta().y() > 0)
        m_totalScale *= m_scaleStep;
    else
        m_totalScale /= m_scaleStep;

    m_totalScale = std::max(m_minScale, std::min(m_totalScale, m_maxScale));
    double scaleFac = m_totalScale / oldScale;

    scale(scaleFac, scaleFac);
    event->accept();
}

void ZoomableGraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QGraphicsView::mousePressEvent(event);
}

void ZoomableGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging) {
        QPointF delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
    }
    QGraphicsView::mouseMoveEvent(event);
}