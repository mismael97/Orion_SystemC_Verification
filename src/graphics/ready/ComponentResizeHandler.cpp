// ComponentResizeHandler.cpp
#include "graphics/ready/ComponentResizeHandler.h"
#include <QPainter>
#include <QtMath>

ComponentResizeHandler::ComponentResizeHandler()
    : m_isResizing(false)
{
}

QRectF ComponentResizeHandler::getResizeHandleRect(qreal width, qreal height) const
{
    return QRectF(width - RESIZE_HANDLE_SIZE, height - RESIZE_HANDLE_SIZE, 
                  RESIZE_HANDLE_SIZE, RESIZE_HANDLE_SIZE);
}

bool ComponentResizeHandler::isInResizeHandle(const QPointF& pos, qreal width, qreal height) const
{
    return getResizeHandleRect(width, height).contains(pos);
}

void ComponentResizeHandler::startResize(const QPointF& pos, qreal width, qreal height)
{
    m_isResizing = true;
    m_resizeStartPos = pos;
    m_resizeStartSize = QSizeF(width, height);
}

void ComponentResizeHandler::updateResize(const QPointF& pos, qreal& width, qreal& height)
{
    if (!m_isResizing) return;
    
    QPointF delta = pos - m_resizeStartPos;
    qreal newWidth = m_resizeStartSize.width() + delta.x();
    qreal newHeight = m_resizeStartSize.height() + delta.y();
    
    // Apply minimum constraints (increased for better usability)
    width = qMax(60.0, newWidth);   // Minimum width
    height = qMax(50.0, newHeight);  // Minimum height
}

void ComponentResizeHandler::endResize()
{
    m_isResizing = false;
}

void ComponentResizeHandler::drawResizeHandle(QPainter* painter, qreal width, qreal height, qreal offset) const
{
    QRectF handleRect = getResizeHandleRect(width, height);
    handleRect.translate(offset, offset);
    
    painter->setPen(QPen(Qt::darkGray, 1));
    painter->setBrush(Qt::lightGray);
    painter->drawRect(handleRect);
    
    // Draw resize grip lines
    painter->setPen(QPen(Qt::darkGray, 1));
    for (int i = 1; i <= 3; ++i) {
        qreal gripOffset = i * 2;
        painter->drawLine(handleRect.right() - gripOffset, handleRect.bottom(),
                        handleRect.right(), handleRect.bottom() - gripOffset);
    }
}

QCursor ComponentResizeHandler::getCursorForPosition(const QPointF& pos, bool isSelected, 
                                                     qreal width, qreal height) const
{
    if (isSelected && isInResizeHandle(pos, width, height)) {
        return Qt::SizeFDiagCursor;
    }
    return Qt::ArrowCursor;
}

