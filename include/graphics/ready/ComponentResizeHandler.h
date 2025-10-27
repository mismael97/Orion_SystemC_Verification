// ComponentResizeHandler.h
#ifndef COMPONENTRESIZEHANDLER_H
#define COMPONENTRESIZEHANDLER_H

#include <QPointF>
#include <QSizeF>
#include <QRectF>
#include <QCursor>

class QPainter;

/**
 * @brief Handles resize functionality for ready components
 * 
 * This class handles:
 * - Resize handle detection
 * - Resize operations
 * - Resize handle rendering
 */
class ComponentResizeHandler
{
public:
    ComponentResizeHandler();
    
    // Resize operations
    bool isInResizeHandle(const QPointF& pos, qreal width, qreal height) const;
    void startResize(const QPointF& pos, qreal width, qreal height);
    void updateResize(const QPointF& pos, qreal& width, qreal& height);
    void endResize();
    bool isResizing() const { return m_isResizing; }
    
    // Rendering
    void drawResizeHandle(QPainter* painter, qreal width, qreal height, qreal offset) const;
    QRectF getResizeHandleRect(qreal width, qreal height) const;
    
    // Cursor management
    QCursor getCursorForPosition(const QPointF& pos, bool isSelected, qreal width, qreal height) const;
    
    // Constants
    static constexpr int RESIZE_HANDLE_SIZE = 10;

private:
    bool m_isResizing;
    QPointF m_resizeStartPos;
    QSizeF m_resizeStartSize;
};

#endif // COMPONENTRESIZEHANDLER_H

