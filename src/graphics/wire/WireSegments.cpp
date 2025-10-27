// WireSegments.cpp
#include "graphics/wire/WireSegments.h"
#include <QLineF>
#include <QtMath>

WireSegments::WireSegments()
{
}

void WireSegments::updateFromPath(const QPainterPath& path)
{
    m_segments.clear();
    
    // Extract segments from the path
    int elementCount = path.elementCount();
    if (elementCount < 2) return;
    
    for (int i = 0; i < elementCount - 1; ++i) {
        QPointF start(path.elementAt(i).x, path.elementAt(i).y);
        QPointF end(path.elementAt(i + 1).x, path.elementAt(i + 1).y);
        
        WireSegment segment;
        segment.start = start;
        segment.end = end;
        segment.segmentIndex = i;
        
        // Determine if vertical or horizontal
        qreal dx = qAbs(end.x() - start.x());
        qreal dy = qAbs(end.y() - start.y());
        
        segment.isVertical = (dx < 5 && dy > 5);
        segment.isHorizontal = (dy < 5 && dx > 5);
        
        if (segment.isVertical || segment.isHorizontal) {
            m_segments.append(segment);
        }
    }
}

int WireSegments::findSegmentAt(const QPointF& scenePos) const
{
    for (int i = 0; i < m_segments.size(); ++i) {
        const WireSegment& segment = m_segments[i];
        
        // Create a line from segment
        QLineF line(segment.start, segment.end);
        
        // Find distance from point to line
        QPointF p1 = segment.start;
        QPointF p2 = segment.end;
        QPointF p = scenePos;
        
        qreal lineLength = line.length();
        if (lineLength < 0.001) continue;
        
        // Calculate perpendicular distance
        qreal t = QPointF::dotProduct(p - p1, p2 - p1) / (lineLength * lineLength);
        
        if (t < 0.0 || t > 1.0) continue; // Point is outside segment
        
        QPointF projection = p1 + t * (p2 - p1);
        qreal distance = QLineF(p, projection).length();
        
        if (distance <= SEGMENT_DETECTION_THRESHOLD) {
            return i;
        }
    }
    
    return -1;
}

void WireSegments::drawSegmentArrows(QPainter* painter, int selectedSegmentIndex) const
{
    if (selectedSegmentIndex < 0 || selectedSegmentIndex >= m_segments.size()) {
        return;
    }
    
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    const WireSegment& segment = m_segments[selectedSegmentIndex];
    QPointF midPoint = (segment.start + segment.end) / 2.0;
    
    qreal arrowSize = ADJUSTMENT_ARROW_SIZE;
    QColor arrowColor(100, 200, 255, 200);
    
    painter->setPen(QPen(Qt::white, 2));
    painter->setBrush(arrowColor);
    
    if (segment.isVertical) {
        // Vertical segment: show horizontal arrows (perpendicular movement)
        qreal offset = 25;
        
        // Left arrow
        QPainterPath leftArrow;
        QPointF leftCenter = midPoint + QPointF(-offset, 0);
        leftArrow.moveTo(leftCenter + QPointF(-arrowSize/2, 0));
        leftArrow.lineTo(leftCenter + QPointF(arrowSize/2, -arrowSize/2));
        leftArrow.lineTo(leftCenter + QPointF(arrowSize/2, arrowSize/2));
        leftArrow.closeSubpath();
        painter->drawPath(leftArrow);
        
        // Right arrow
        QPainterPath rightArrow;
        QPointF rightCenter = midPoint + QPointF(offset, 0);
        rightArrow.moveTo(rightCenter + QPointF(arrowSize/2, 0));
        rightArrow.lineTo(rightCenter + QPointF(-arrowSize/2, -arrowSize/2));
        rightArrow.lineTo(rightCenter + QPointF(-arrowSize/2, arrowSize/2));
        rightArrow.closeSubpath();
        painter->drawPath(rightArrow);
        
        // Draw connecting line
        painter->setPen(QPen(arrowColor, 2, Qt::DotLine));
        painter->drawLine(leftCenter, rightCenter);
        
    } else if (segment.isHorizontal) {
        // Horizontal segment: show vertical arrows (perpendicular movement)
        qreal offset = 25;
        
        // Up arrow
        QPainterPath upArrow;
        QPointF upCenter = midPoint + QPointF(0, -offset);
        upArrow.moveTo(upCenter + QPointF(0, -arrowSize/2));
        upArrow.lineTo(upCenter + QPointF(-arrowSize/2, arrowSize/2));
        upArrow.lineTo(upCenter + QPointF(arrowSize/2, arrowSize/2));
        upArrow.closeSubpath();
        painter->drawPath(upArrow);
        
        // Down arrow
        QPainterPath downArrow;
        QPointF downCenter = midPoint + QPointF(0, offset);
        downArrow.moveTo(downCenter + QPointF(0, arrowSize/2));
        downArrow.lineTo(downCenter + QPointF(-arrowSize/2, -arrowSize/2));
        downArrow.lineTo(downCenter + QPointF(arrowSize/2, -arrowSize/2));
        downArrow.closeSubpath();
        painter->drawPath(downArrow);
        
        // Draw connecting line
        painter->setPen(QPen(arrowColor, 2, Qt::DotLine));
        painter->drawLine(upCenter, downCenter);
    }
}
