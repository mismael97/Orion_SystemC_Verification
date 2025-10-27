// WireManager.cpp
#include "scene/WireManager.h"
#include "graphics/wire/WireGraphicsItem.h"
#include <QGraphicsScene>
#include <QtMath>
#include <QDebug>
#include <QPainterPath>
#include <algorithm>

WireManager::WireManager(QGraphicsScene* scene, QObject* parent)
    : QObject(parent)
    , m_scene(scene)
    , m_autoRoutingEnabled(true)
    , m_bundlingEnabled(true)
    , m_wireSpacing(DEFAULT_WIRE_SPACING)
{
}

WireManager::~WireManager()
{
}

void WireManager::registerWire(WireGraphicsItem* wire)
{
    if (!wire || m_wires.contains(wire)) {
        return;
    }
    
    m_wires.append(wire);
    qDebug() << "WireManager: Registered wire, total wires:" << m_wires.size();
    
    if (m_autoRoutingEnabled) {
        optimizeWireRoute(wire);
    }
    
    emit wireRoutesOptimized();
}

void WireManager::unregisterWire(WireGraphicsItem* wire)
{
    m_wires.removeAll(wire);
    qDebug() << "WireManager: Unregistered wire, remaining wires:" << m_wires.size();
}

void WireManager::optimizeAllWireRoutes()
{
    if (!m_autoRoutingEnabled) {
        return;
    }
    
    qDebug() << "WireManager: Optimizing all wire routes...";
    
    // Apply wire spacing to prevent overlaps
    applyWireSpacing();
    
    // Bundle parallel wires if enabled
    if (m_bundlingEnabled) {
        bundleParallelWires();
    }
    
    // Update z-order for better visibility
    updateWireZOrder();
    
    emit wireRoutesOptimized();
}

void WireManager::optimizeWireRoute(WireGraphicsItem* wire)
{
    if (!wire || !m_autoRoutingEnabled) {
        return;
    }
    
    // Calculate optimal route avoiding other wires
    QPointF start = wire->getSourceScenePos();
    QPointF end = wire->getTargetScenePos();
    
    QList<QPointF> optimalRoute = calculateOptimalRoute(start, end, wire);
    
    // Apply the optimal route if it's better than current
    if (!optimalRoute.isEmpty()) {
        // The route includes intermediate points for better routing
        // This would be applied through control points if needed
        qDebug() << "WireManager: Optimized route for wire with" << optimalRoute.size() << "points";
    }
}

QList<QPointF> WireManager::calculateOptimalRoute(const QPointF& start, const QPointF& end, 
                                                   WireGraphicsItem* excludeWire)
{
    QList<QPointF> route;
    route.append(start);
    
    // Calculate direct path
    qreal dx = end.x() - start.x();
    qreal dy = end.y() - start.y();
    
    // Check if direct path has collisions
    bool hasCollisions = false;
    QPointF midPoint = QPointF(start.x() + dx / 2, start.y() + dy / 2);
    
    if (checkWireCollision(midPoint, COLLISION_THRESHOLD, excludeWire)) {
        hasCollisions = true;
    }
    
    // If collisions detected, add offset points
    if (hasCollisions) {
        // Determine routing direction based on wire orientation
        if (qAbs(dx) > qAbs(dy)) {
            // Horizontal-dominant: add vertical offset
            qreal offsetY = (dy > 0 ? m_wireSpacing : -m_wireSpacing);
            route.append(QPointF(start.x() + dx * 0.3, start.y()));
            route.append(QPointF(start.x() + dx * 0.3, start.y() + offsetY));
            route.append(QPointF(start.x() + dx * 0.7, start.y() + offsetY));
            route.append(QPointF(start.x() + dx * 0.7, end.y()));
        } else {
            // Vertical-dominant: add horizontal offset
            qreal offsetX = (dx > 0 ? m_wireSpacing : -m_wireSpacing);
            route.append(QPointF(start.x(), start.y() + dy * 0.3));
            route.append(QPointF(start.x() + offsetX, start.y() + dy * 0.3));
            route.append(QPointF(start.x() + offsetX, start.y() + dy * 0.7));
            route.append(QPointF(end.x(), start.y() + dy * 0.7));
        }
    } else {
        // No collisions: use simple orthogonal route
        if (qAbs(dx) > qAbs(dy)) {
            route.append(QPointF(start.x() + dx / 2, start.y()));
            route.append(QPointF(start.x() + dx / 2, end.y()));
        } else {
            route.append(QPointF(start.x(), start.y() + dy / 2));
            route.append(QPointF(end.x(), start.y() + dy / 2));
        }
    }
    
    route.append(end);
    return route;
}

bool WireManager::checkWireCollision(const QPointF& point, qreal radius, WireGraphicsItem* excludeWire) const
{
    for (WireGraphicsItem* wire : m_wires) {
        if (wire == excludeWire) {
            continue;
        }
        
        qreal distance = distanceToWire(point, wire);
        if (distance < radius) {
            return true;
        }
    }
    return false;
}

QList<WireGraphicsItem*> WireManager::getWiresNearPoint(const QPointF& point, qreal radius) const
{
    QList<WireGraphicsItem*> nearWires;
    
    for (WireGraphicsItem* wire : m_wires) {
        qreal distance = distanceToWire(point, wire);
        if (distance < radius) {
            nearWires.append(wire);
        }
    }
    
    return nearWires;
}

bool WireManager::areWiresOverlapping(WireGraphicsItem* wire1, WireGraphicsItem* wire2) const
{
    if (!wire1 || !wire2) {
        return false;
    }
    
    QRectF bbox1 = getWireBoundingBox(wire1);
    QRectF bbox2 = getWireBoundingBox(wire2);
    
    // Check if bounding boxes intersect
    if (!bbox1.intersects(bbox2)) {
        return false;
    }
    
    // Check actual path intersection
    QList<RouteSegment> segments1 = getWireSegments(wire1);
    QList<RouteSegment> segments2 = getWireSegments(wire2);
    
    for (const RouteSegment& seg1 : segments1) {
        for (const RouteSegment& seg2 : segments2) {
            if (seg1.intersects(seg2)) {
                return true;
            }
        }
    }
    
    return false;
}

void WireManager::applyWireSpacing()
{
    qDebug() << "WireManager: Applying wire spacing...";
    
    // Group wires by their general routing paths
    QMap<int, QList<WireGraphicsItem*>> routeGroups;
    
    for (WireGraphicsItem* wire : m_wires) {
        QPointF start = wire->getSourceScenePos();
        QPointF end = wire->getTargetScenePos();
        
        // Create a routing key based on general direction
        int routeKey = qRound(start.x() / 50) * 1000 + qRound(start.y() / 50);
        routeGroups[routeKey].append(wire);
    }
    
    // Apply spacing within each group
    for (const QList<WireGraphicsItem*>& group : routeGroups) {
        if (group.size() < 2) {
            continue;
        }
        
        // Apply vertical offset to wires in the same group
        for (int i = 0; i < group.size(); ++i) {
            qreal offset = (i - group.size() / 2.0) * m_wireSpacing;
            // Store offset for use during rendering
            // This would be applied through wire's orthogonal offset
            if (group[i]) {
                group[i]->setOrthogonalOffset(offset);
            }
        }
    }
}

void WireManager::bundleParallelWires()
{
    qDebug() << "WireManager: Bundling parallel wires...";
    
    QList<WireBundle> bundles = identifyBundles();
    
    for (const WireBundle& bundle : bundles) {
        if (bundle.wires.size() > 1) {
            applyBundleSpacing(bundle);
        }
    }
}

QList<WireManager::WireBundle> WireManager::identifyBundles() const
{
    QList<WireBundle> bundles;
    QSet<WireGraphicsItem*> processedWires;
    
    for (WireGraphicsItem* wire : m_wires) {
        if (processedWires.contains(wire)) {
            continue;
        }
        
        WireBundle bundle;
        bundle.wires.append(wire);
        bundle.startPoint = wire->getSourceScenePos();
        bundle.endPoint = wire->getTargetScenePos();
        bundle.isParallel = false;
        
        // Find parallel wires
        for (WireGraphicsItem* other : m_wires) {
            if (other == wire || processedWires.contains(other)) {
                continue;
            }
            
            QPointF otherStart = other->getSourceScenePos();
            QPointF otherEnd = other->getTargetScenePos();
            
            // Check if wires are roughly parallel
            QPointF dir1 = bundle.endPoint - bundle.startPoint;
            QPointF dir2 = otherEnd - otherStart;
            
            qreal angle = qAbs(qAtan2(dir1.y(), dir1.x()) - qAtan2(dir2.y(), dir2.x()));
            if (angle < 0.2 || angle > M_PI - 0.2) {  // Roughly parallel (within ~11 degrees)
                // Check if wires are close to each other
                qreal distance = QLineF(bundle.startPoint, otherStart).length();
                if (distance < 100.0) {  // Within 100 pixels
                    bundle.wires.append(other);
                    bundle.isParallel = true;
                    processedWires.insert(other);
                }
            }
        }
        
        processedWires.insert(wire);
        bundles.append(bundle);
    }
    
    return bundles;
}

void WireManager::applyBundleSpacing(const WireBundle& bundle)
{
    if (bundle.wires.size() < 2) {
        return;
    }
    
    // Apply perpendicular offset to each wire in the bundle
    QPointF direction = bundle.endPoint - bundle.startPoint;
    qreal length = qSqrt(direction.x() * direction.x() + direction.y() * direction.y());
    if (length < 0.1) {
        return;
    }
    
    // Normalize direction
    direction /= length;
    
    // Perpendicular vector
    QPointF perpendicular(-direction.y(), direction.x());
    
    // Apply offset to each wire
    int numWires = bundle.wires.size();
    for (int i = 0; i < numWires; ++i) {
        qreal offset = (i - (numWires - 1) / 2.0) * m_wireSpacing;
        QPointF offsetVector = perpendicular * offset;
        
        // Apply offset through orthogonal offset property
        if (bundle.wires[i]) {
            bundle.wires[i]->setOrthogonalOffset(offset);
        }
    }
}

void WireManager::updateWireZOrder()
{
    // Sort wires by length (shorter wires on top for better visibility)
    QList<WireGraphicsItem*> sortedWires = m_wires;
    
    std::sort(sortedWires.begin(), sortedWires.end(), 
              [](WireGraphicsItem* a, WireGraphicsItem* b) {
        QPointF aStart = a->getSourceScenePos();
        QPointF aEnd = a->getTargetScenePos();
        QPointF bStart = b->getSourceScenePos();
        QPointF bEnd = b->getTargetScenePos();
        
        qreal aLength = QLineF(aStart, aEnd).length();
        qreal bLength = QLineF(bStart, bEnd).length();
        
        return aLength < bLength;  // Shorter wires have higher z-order
    });
    
    // Apply z-order
    for (int i = 0; i < sortedWires.size(); ++i) {
        sortedWires[i]->setZValue(100 + i);  // Base z-value of 100 for wires
    }
    
    qDebug() << "WireManager: Updated z-order for" << sortedWires.size() << "wires";
}

void WireManager::bringWireToFront(WireGraphicsItem* wire)
{
    if (!wire) return;
    
    qreal maxZ = 100;
    for (WireGraphicsItem* w : m_wires) {
        if (w->zValue() > maxZ) {
            maxZ = w->zValue();
        }
    }
    
    wire->setZValue(maxZ + 1);
}

void WireManager::sendWireToBack(WireGraphicsItem* wire)
{
    if (!wire) return;
    
    qreal minZ = 100;
    for (WireGraphicsItem* w : m_wires) {
        if (w->zValue() < minZ) {
            minZ = w->zValue();
        }
    }
    
    wire->setZValue(minZ - 1);
}

QList<WireManager::RouteSegment> WireManager::getWireSegments(WireGraphicsItem* wire) const
{
    QList<RouteSegment> segments;
    
    if (!wire) {
        return segments;
    }
    
    QList<QPointF> controlPoints = wire->getControlPoints();
    QPointF start = wire->getSourceScenePos();
    QPointF end = wire->getTargetScenePos();
    
    if (controlPoints.isEmpty()) {
        // Simple wire: single segment
        RouteSegment seg;
        seg.start = start;
        seg.end = end;
        seg.isHorizontal = qAbs(end.x() - start.x()) > qAbs(end.y() - start.y());
        segments.append(seg);
    } else {
        // Wire with control points: multiple segments
        QPointF prev = start;
        for (const QPointF& cp : controlPoints) {
            RouteSegment seg;
            seg.start = prev;
            seg.end = cp;
            seg.isHorizontal = qAbs(cp.x() - prev.x()) > qAbs(cp.y() - prev.y());
            segments.append(seg);
            prev = cp;
        }
        
        // Last segment
        RouteSegment seg;
        seg.start = prev;
        seg.end = end;
        seg.isHorizontal = qAbs(end.x() - prev.x()) > qAbs(end.y() - prev.y());
        segments.append(seg);
    }
    
    return segments;
}

qreal WireManager::RouteSegment::getLength() const
{
    return QLineF(start, end).length();
}

bool WireManager::RouteSegment::intersects(const RouteSegment& other) const
{
    QLineF line1(start, end);
    QLineF line2(other.start, other.end);
    
    QPointF intersection;
    QLineF::IntersectionType type = line1.intersects(line2, &intersection);
    
    if (type == QLineF::BoundedIntersection) {
        return true;
    }
    
    // Check for parallel overlap
    if (isHorizontal == other.isHorizontal) {
        qreal dist = QLineF(start, other.start).length();
        if (dist < 10.0) {  // Wires are close and parallel
            return true;
        }
    }
    
    return false;
}

bool WireManager::segmentIntersectsWires(const RouteSegment& segment, WireGraphicsItem* excludeWire) const
{
    for (WireGraphicsItem* wire : m_wires) {
        if (wire == excludeWire) {
            continue;
        }
        
        QList<RouteSegment> wireSegments = getWireSegments(wire);
        for (const RouteSegment& wireSeg : wireSegments) {
            if (segment.intersects(wireSeg)) {
                return true;
            }
        }
    }
    
    return false;
}

QPointF WireManager::offsetPoint(const QPointF& point, qreal offset, bool horizontal) const
{
    if (horizontal) {
        return QPointF(point.x(), point.y() + offset);
    } else {
        return QPointF(point.x() + offset, point.y());
    }
}

QRectF WireManager::getWireBoundingBox(WireGraphicsItem* wire) const
{
    if (!wire) {
        return QRectF();
    }
    
    return wire->sceneBoundingRect();
}

qreal WireManager::distanceToWire(const QPointF& point, WireGraphicsItem* wire) const
{
    if (!wire) {
        return 999999.0;
    }
    
    QList<RouteSegment> segments = getWireSegments(wire);
    qreal minDistance = 999999.0;
    
    for (const RouteSegment& segment : segments) {
        QLineF line(segment.start, segment.end);
        
        // Calculate distance from point to line segment
        QPointF v = segment.end - segment.start;
        QPointF w = point - segment.start;
        
        qreal c1 = QPointF::dotProduct(w, v);
        if (c1 <= 0) {
            qreal dist = QLineF(point, segment.start).length();
            minDistance = qMin(minDistance, dist);
            continue;
        }
        
        qreal c2 = QPointF::dotProduct(v, v);
        if (c1 >= c2) {
            qreal dist = QLineF(point, segment.end).length();
            minDistance = qMin(minDistance, dist);
            continue;
        }
        
        qreal b = c1 / c2;
        QPointF pb = segment.start + v * b;
        qreal dist = QLineF(point, pb).length();
        minDistance = qMin(minDistance, dist);
    }
    
    return minDistance;
}

qreal WireManager::calculateWireOffset(WireGraphicsItem* wire, const QPointF& point) const
{
    QList<WireGraphicsItem*> nearWires = getWiresNearPoint(point, 30.0);
    
    if (nearWires.isEmpty()) {
        return 0.0;
    }
    
    // Calculate offset to avoid nearby wires
    qreal totalOffset = 0.0;
    for (WireGraphicsItem* nearWire : nearWires) {
        if (nearWire != wire) {
            totalOffset += m_wireSpacing;
        }
    }
    
    return totalOffset / nearWires.size();
}

void WireManager::onWirePathChanged(WireGraphicsItem* wire)
{
    if (m_autoRoutingEnabled) {
        // Check for collisions and optimize if needed
        for (WireGraphicsItem* other : m_wires) {
            if (other != wire && areWiresOverlapping(wire, other)) {
                emit wireCollisionDetected(wire, other);
            }
        }
    }
}

void WireManager::onWireAdded(WireGraphicsItem* wire)
{
    registerWire(wire);
}

void WireManager::onWireRemoved(WireGraphicsItem* wire)
{
    unregisterWire(wire);
}

