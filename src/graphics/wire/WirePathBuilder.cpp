// WirePathBuilder.cpp
#include "graphics/wire/WirePathBuilder.h"
#include <QtMath>

QPainterPath WirePathBuilder::createPath(const QPointF& start, const QPointF& end, 
                                        RoutingMode mode, qreal offset)
{
    switch (mode) {
        case Straight:
            return createStraightPath(start, end);
        case Bezier:
            return createBezierPath(start, end);
        case Orthogonal:
        default:
            return createOrthogonalPath(start, end, offset);
    }
}

QPainterPath WirePathBuilder::createPathWithControlPoints(const QPointF& start, 
                                                          const QPointF& end,
                                                          const QVector<QPointF>& controlPoints)
{
    QPainterPath path;
    path.moveTo(start);
    
    // Draw path through control points
    for (const QPointF& controlPoint : controlPoints) {
        path.lineTo(controlPoint);
    }
    
    path.lineTo(end);
    return path;
}

QPainterPath WirePathBuilder::createStraightPath(const QPointF& start, const QPointF& end)
{
    QPainterPath path;
    path.moveTo(start);
    path.lineTo(end);
    return path;
}

QPainterPath WirePathBuilder::createOrthogonalPath(const QPointF& start, const QPointF& end, qreal offset)
{
    QPainterPath path;
    path.moveTo(start);

    qreal dx = end.x() - start.x();
    qreal dy = end.y() - start.y();

    // Check if wire goes from RIGHT to LEFT (output on right, input on left)
    if (dx < 0) {
        // Right-to-Left wire
        if (qAbs(dx) > (PORT_SPACING * 2.0)) {
            // ◀ Wire goes to the LEFT with enough space → use 3-segment path
            // Middle segment is vertical, offset adjusts it horizontally
            qreal midX = (start.x() + end.x()) / 2.0 + offset;

            // Segment 1: horizontal from start to midX (going left)
            path.lineTo(midX, start.y());

            // Segment 2: vertical from start.y() to end.y() (MIDDLE SEGMENT - can slide horizontally)
            path.lineTo(midX, end.y());

            // Segment 3: horizontal from midX to end (going left)
            path.lineTo(end.x(), end.y());
        } else {
            // Not enough space, need to go around with 5-segment path
            path.lineTo(start.x() - PORT_SPACING, start.y());

            // Middle segment is horizontal, offset adjusts it vertically
            qreal midY = (start.y() + end.y()) / 2.0 + offset;
            path.lineTo(start.x() - PORT_SPACING, midY);
            
            // Segment 3: horizontal middle segment (MIDDLE SEGMENT - can slide vertically)
            path.lineTo(end.x() + PORT_SPACING, midY);
            
            path.lineTo(end.x() + PORT_SPACING, end.y());
            path.lineTo(end.x(), end.y());
        }
    } else if (dx <= (PORT_SPACING * 2.0)) {
        // Left-to-Right but too close → use 5-segment path
        path.lineTo(start.x() + PORT_SPACING, start.y());

        // Middle segment is horizontal, offset adjusts it vertically
        qreal midY = (start.y() + end.y()) / 2.0 + offset;
        path.lineTo(start.x() + PORT_SPACING, midY);
        
        // Segment 3: horizontal middle segment (MIDDLE SEGMENT - can slide vertically)
        path.lineTo(end.x() - PORT_SPACING, midY);
        
        path.lineTo(end.x() - PORT_SPACING, end.y());
        path.lineTo(end.x(), end.y());
    } else {
        // ▶ Wire goes to the RIGHT with enough space → use 3-segment path
        // Middle segment is vertical, offset adjusts it horizontally
        qreal midX = (start.x() + end.x()) / 2.0 + offset;

        // Segment 1: horizontal from start to midX
        path.lineTo(midX, start.y());

        // Segment 2: vertical from start.y() to end.y() (MIDDLE SEGMENT - can slide horizontally)
        path.lineTo(midX, end.y());

        // Segment 3: horizontal from midX to end
        path.lineTo(end.x(), end.y());
    }

    return path;
}

QPainterPath WirePathBuilder::createBezierPath(const QPointF& start, const QPointF& end)
{
    QPainterPath path;
    path.moveTo(start);
    
    // Calculate control points for smooth Bezier curve
    qreal dx = end.x() - start.x();
    qreal dy = end.y() - start.y();
    
    // Control points offset based on distance
    qreal controlOffset = qMax(qAbs(dx), qAbs(dy)) * 0.5;
    
    QPointF control1 = start + QPointF(controlOffset, 0);
    QPointF control2 = end - QPointF(controlOffset, 0);
    
    path.cubicTo(control1, control2, end);
    
    return path;
}
