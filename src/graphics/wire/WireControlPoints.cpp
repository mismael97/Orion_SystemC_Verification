// WireControlPoints.cpp
#include "graphics/wire/WireControlPoints.h"
#include <QLineF>
#include <QtMath>

WireControlPoints::WireControlPoints()
{
}

void WireControlPoints::addControlPoint(const QPointF& point)
{
    m_controlPoints.append(point);
}

void WireControlPoints::removeControlPoint(int index)
{
    if (index >= 0 && index < m_controlPoints.size()) {
        m_controlPoints.removeAt(index);
    }
}

int WireControlPoints::findControlPointAt(const QPointF& scenePos) const
{
    for (int i = 0; i < m_controlPoints.size(); ++i) {
        qreal distance = QLineF(m_controlPoints[i], scenePos).length();
        if (distance < CONTROL_POINT_DETECTION_RADIUS) {
            return i;
        }
    }
    return -1;
}

QPointF WireControlPoints::findNearestPointOnPath(const QPointF& pos, const QPainterPath& path) const
{
    QPointF nearest = path.pointAtPercent(0);
    qreal minDist = QLineF(nearest, pos).length();
    
    for (qreal t = 0.0; t <= 1.0; t += 0.01) {
        QPointF point = path.pointAtPercent(t);
        qreal dist = QLineF(point, pos).length();
        if (dist < minDist) {
            minDist = dist;
            nearest = point;
        }
    }
    
    return nearest;
}

void WireControlPoints::updateControlPoint(int index, const QPointF& newPos)
{
    if (index >= 0 && index < m_controlPoints.size()) {
        m_controlPoints[index] = newPos;
    }
}

void WireControlPoints::drawControlPoints(QPainter* painter, bool isSelected, int hoveredIndex) const
{
    if (m_controlPoints.isEmpty()) {
        return;
    }
    
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    for (int i = 0; i < m_controlPoints.size(); ++i) {
        const QPointF& point = m_controlPoints[i];
        bool isHovered = (i == hoveredIndex);
        
        if (isSelected) {
            // Draw outer glow for selected wire
            if (isHovered) {
                painter->setPen(Qt::NoPen);
                painter->setBrush(QColor(255, 255, 0, 150));
                painter->drawEllipse(point, CONTROL_POINT_RADIUS + 4, CONTROL_POINT_RADIUS + 4);
            }
            
            // Draw control point circle
            painter->setPen(QPen(Qt::white, 2));
            painter->setBrush(isHovered ? QColor(255, 200, 0) : QColor(100, 150, 255));
            painter->drawEllipse(point, CONTROL_POINT_RADIUS, CONTROL_POINT_RADIUS);
            
            // Draw inner highlight
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::white);
            painter->drawEllipse(point, CONTROL_POINT_RADIUS / 2, CONTROL_POINT_RADIUS / 2);
        } else {
            // Draw subtle control points for unselected wire
            painter->setPen(QPen(QColor(150, 150, 150, 100), 1));
            painter->setBrush(QColor(100, 150, 255, 50));
            painter->drawEllipse(point, CONTROL_POINT_RADIUS / 2, CONTROL_POINT_RADIUS / 2);
        }
    }
}

void WireControlPoints::nudgeAll(const QPointF& offset)
{
    for (QPointF& point : m_controlPoints) {
        point += offset;
    }
}
