// WireRenderer.cpp
#include "graphics/wire/WireRenderer.h"
#include <QtMath>

WireRenderer::WireRenderer()
{
    m_neonColor = generateNeonColor();
}

QColor WireRenderer::generateNeonColor()
{
    // Use default black color for all wires
    return QColor(0, 0, 0);
}

QColor WireRenderer::getWireColor() const
{
    // Return color based on wire state
    if (m_useCustomColor) {
        return m_customColor;
    }
    
    switch (m_wireState) {
        case Error:
            return QColor(255, 50, 50); // Red
        case Active:
            return QColor(50, 255, 50); // Green
        case Locked:
            return QColor(150, 150, 150); // Gray
        case Normal:
        default:
            return m_neonColor;
    }
}

Qt::PenStyle WireRenderer::getPenStyle() const
{
    switch (m_lineStyle) {
        case Dashed:
            return Qt::DashLine;
        case Dotted:
            return Qt::DotLine;
        case Solid:
        default:
            return Qt::SolidLine;
    }
}

void WireRenderer::paintNeonEffect(QPainter* painter, const QPainterPath& path, 
                                    const QColor& color, int width)
{
    if (m_wireState == Locked) {
        return; // No glow for locked wires
    }
    
    // Draw outer glow (multiple layers for neon effect)
    for (int i = 1; i > 0; --i) {
        QColor glowColor = color;
        glowColor.setAlpha(30 / i);
        QPen glowPen(glowColor, width + i * 8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(glowPen);
        painter->drawPath(path);
    }
}

void WireRenderer::paint(QPainter* painter, const QPainterPath& path, 
                        bool isSelected, bool isTemporary)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    QColor wireColor = getWireColor();
    int wireWidth = m_wireThickness;
    
    if (isSelected) {
        wireColor = wireColor.lighter(130);
        wireWidth += 3;  // Make selected wire thicker
    }
    
    Qt::PenStyle penStyle = getPenStyle();
    
    // Draw neon glow effect
    paintNeonEffect(painter, path, wireColor, wireWidth);
    
    // Draw the main neon wire
    QPen pen(wireColor, wireWidth, penStyle, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->drawPath(path);
    
    // Draw bright core (not for locked wires)
    if (m_wireState != Locked) {
        QColor coreColor = wireColor.lighter(180);
        QPen corePen(coreColor, wireWidth / 2, penStyle, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(corePen);
        painter->drawPath(path);
    }
}

void WireRenderer::drawArrow(QPainter* painter, const QPainterPath& path, bool isInverted)
{
    if (path.elementCount() < 2) {
        return;
    }
    
    qreal arrowSize = 10;
    
    // Get start and end points to determine wire direction
    QPointF startPoint = QPointF(path.elementAt(0));
    QPointF endPoint = QPointF(path.elementAt(path.elementCount() - 1));
    
    // Determine if wire is going right-to-left
    bool isRightToLeft = startPoint.x() > endPoint.x();
    
    // Calculate arrow position based on wire direction
    int elementIndex;
    int prevElementIndex;
    
    if (isRightToLeft) {
        // Wire goes right to left: draw arrow at target (left side)
        // Arrow should point LEFT (from right to left)
        elementIndex = path.elementCount() - 1;
        prevElementIndex = path.elementCount() - 2;
    } else {
        // Wire goes left to right: draw arrow at target (right side)
        // Arrow should point RIGHT (from left to right)
        elementIndex = isInverted ? 1 : path.elementCount() - 1;
        prevElementIndex = isInverted ? 0 : path.elementCount() - 2;
    }
    
    QPointF arrowEndPoint = QPointF(path.elementAt(elementIndex));
    QPointF lastPoint = QPointF(path.elementAt(prevElementIndex));
    QPointF direction = arrowEndPoint - lastPoint;
    qreal length = qSqrt(direction.x() * direction.x() + direction.y() * direction.y());
    
    if (length > 0) {
        direction /= length;
        QPointF perpendicular(-direction.y(), direction.x());
        
        QPointF arrowTip = arrowEndPoint;
        QPointF p1 = arrowTip - direction * arrowSize + perpendicular * (arrowSize / 2);
        QPointF p2 = arrowTip - direction * arrowSize - perpendicular * (arrowSize / 2);
        
        QPainterPath arrowHead;
        arrowHead.moveTo(arrowTip);
        arrowHead.lineTo(p1);
        arrowHead.lineTo(p2);
        arrowHead.closeSubpath();
        
        QColor wireColor = getWireColor();
        
        // Draw arrow glow
        painter->setPen(Qt::NoPen);
        painter->setBrush(wireColor);
        painter->drawPath(arrowHead);
        
        // Draw bright arrow core
        if (m_wireState != Locked) {
            painter->setBrush(wireColor.lighter(180));
            QPainterPath smallerArrow;
            smallerArrow.moveTo(arrowTip);
            smallerArrow.lineTo((p1 + arrowTip) / 2.0);
            smallerArrow.lineTo((p2 + arrowTip) / 2.0);
            smallerArrow.closeSubpath();
            painter->drawPath(smallerArrow);
        }
    }
}

void WireRenderer::drawLockedIndicator(QPainter* painter, const QPainterPath& path)
{
    if (!m_isLocked) {
        return;
    }
    
    painter->setPen(QPen(Qt::yellow, 2));
    painter->setBrush(Qt::NoBrush);
    QPointF lockPos = path.boundingRect().center() + QPointF(-10, -10);
    QRectF lockRect(lockPos, QSizeF(20, 20));
    painter->drawRect(lockRect);
    painter->drawText(lockRect, Qt::AlignCenter, "L");
}
