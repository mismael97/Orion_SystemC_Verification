// WireManager.h
#ifndef WIREMANAGER_H
#define WIREMANAGER_H

#include <QObject>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QMap>
#include <QSet>

class WireGraphicsItem;
class QGraphicsScene;

/**
 * @brief Global wire manager for intelligent routing and organization
 * 
 * This class handles:
 * - Wire registration and tracking
 * - Intelligent routing algorithms
 * - Collision detection and avoidance
 * - Wire spacing and offset management
 * - Z-order management for better visibility
 * - Wire bundling for parallel connections
 */
class WireManager : public QObject
{
    Q_OBJECT

public:
    explicit WireManager(QGraphicsScene* scene, QObject* parent = nullptr);
    ~WireManager();
    
    // Wire registration
    void registerWire(WireGraphicsItem* wire);
    void unregisterWire(WireGraphicsItem* wire);
    QList<WireGraphicsItem*> getAllWires() const { return m_wires; }
    
    // Intelligent routing
    void optimizeAllWireRoutes();
    void optimizeWireRoute(WireGraphicsItem* wire);
    QList<QPointF> calculateOptimalRoute(const QPointF& start, const QPointF& end, 
                                         WireGraphicsItem* excludeWire = nullptr);
    
    // Collision detection
    bool checkWireCollision(const QPointF& point, qreal radius = 10.0, 
                           WireGraphicsItem* excludeWire = nullptr) const;
    QList<WireGraphicsItem*> getWiresNearPoint(const QPointF& point, qreal radius = 20.0) const;
    bool areWiresOverlapping(WireGraphicsItem* wire1, WireGraphicsItem* wire2) const;
    
    // Wire spacing and offset
    void applyWireSpacing();
    qreal calculateWireOffset(WireGraphicsItem* wire, const QPointF& point) const;
    void bundleParallelWires();
    
    // Z-order management
    void updateWireZOrder();
    void bringWireToFront(WireGraphicsItem* wire);
    void sendWireToBack(WireGraphicsItem* wire);
    
    // Configuration
    void setAutoRouting(bool enabled) { m_autoRoutingEnabled = enabled; }
    bool isAutoRoutingEnabled() const { return m_autoRoutingEnabled; }
    void setWireSpacing(qreal spacing) { m_wireSpacing = spacing; }
    qreal getWireSpacing() const { return m_wireSpacing; }
    void setBundlingEnabled(bool enabled) { m_bundlingEnabled = enabled; }
    bool isBundlingEnabled() const { return m_bundlingEnabled; }
    
    // Constants
    static constexpr qreal DEFAULT_WIRE_SPACING = 8.0;
    static constexpr qreal MIN_WIRE_SPACING = 4.0;
    static constexpr qreal MAX_WIRE_SPACING = 20.0;
    static constexpr qreal COLLISION_THRESHOLD = 5.0;

public slots:
    void onWirePathChanged(WireGraphicsItem* wire);
    void onWireAdded(WireGraphicsItem* wire);
    void onWireRemoved(WireGraphicsItem* wire);

signals:
    void wireRoutesOptimized();
    void wireCollisionDetected(WireGraphicsItem* wire1, WireGraphicsItem* wire2);

private:
    QGraphicsScene* m_scene;
    QList<WireGraphicsItem*> m_wires;
    
    // Configuration
    bool m_autoRoutingEnabled;
    bool m_bundlingEnabled;
    qreal m_wireSpacing;
    
    // Routing helpers
    struct RouteSegment {
        QPointF start;
        QPointF end;
        bool isHorizontal;
        qreal getLength() const;
        bool intersects(const RouteSegment& other) const;
    };
    
    QList<RouteSegment> getWireSegments(WireGraphicsItem* wire) const;
    bool segmentIntersectsWires(const RouteSegment& segment, WireGraphicsItem* excludeWire) const;
    QPointF offsetPoint(const QPointF& point, qreal offset, bool horizontal) const;
    
    // Bundling helpers
    struct WireBundle {
        QList<WireGraphicsItem*> wires;
        QPointF startPoint;
        QPointF endPoint;
        bool isParallel;
    };
    
    QList<WireBundle> identifyBundles() const;
    void applyBundleSpacing(const WireBundle& bundle);
    
    // Collision detection helpers
    QRectF getWireBoundingBox(WireGraphicsItem* wire) const;
    qreal distanceToWire(const QPointF& point, WireGraphicsItem* wire) const;
};

#endif // WIREMANAGER_H

