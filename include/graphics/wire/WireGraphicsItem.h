// WireGraphicsItem.h
#ifndef WIREGRAPHICSITEM_H
#define WIREGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include <QObject>
#include <QColor>
#include <QMenu>
#include <QGraphicsTextItem>
#include "graphics/wire/WirePathBuilder.h"
#include "graphics/wire/WireControlPoints.h"
#include "graphics/wire/WireRenderer.h"
#include "graphics/wire/WireSegments.h"

class ReadyComponentGraphicsItem;

/**
 * @brief Main wire graphics item - refactored with composition
 * 
 * This class now delegates responsibilities to specialized components:
 * - WirePathBuilder: Creates wire paths
 * - WireControlPoints: Manages control points
 * - WireRenderer: Handles visual rendering
 * - WireSegments: Manages segment adjustments
 */
class WireGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    // Re-export enums from components for API compatibility
    using RoutingMode = WirePathBuilder::RoutingMode;
    using LineStyle = WireRenderer::LineStyle;
    using WireState = WireRenderer::WireState;

    // Enum values for convenience
    static constexpr RoutingMode Straight = WirePathBuilder::Straight;
    static constexpr RoutingMode Orthogonal = WirePathBuilder::Orthogonal;
    static constexpr RoutingMode Bezier = WirePathBuilder::Bezier;
    
    static constexpr LineStyle Solid = WireRenderer::Solid;
    static constexpr LineStyle Dashed = WireRenderer::Dashed;
    static constexpr LineStyle Dotted = WireRenderer::Dotted;
    
    static constexpr WireState Normal = WireRenderer::Normal;
    static constexpr WireState Active = WireRenderer::Active;
    static constexpr WireState Error = WireRenderer::Error;
    static constexpr WireState Locked = WireRenderer::Locked;

    WireGraphicsItem(ReadyComponentGraphicsItem* source, const QPointF& sourcePort,
                     ReadyComponentGraphicsItem* target = nullptr, const QPointF& targetPort = QPointF(),
                     QGraphicsItem* parent = nullptr);
    ~WireGraphicsItem();

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QPainterPath shape() const override;

    // Port management
    void setSourcePort(const QPointF& port) { m_sourcePort = port; updatePath(); }
    void setTargetPort(const QPointF& port) { m_targetPort = port; m_isTemporary = false; updatePath(); }
    void setTemporaryEnd(const QPointF& pos) { m_temporaryEnd = pos; m_isTemporary = true; updatePath(); }
    void setTarget(ReadyComponentGraphicsItem* target) { m_target = target; }
    
    // Update port positions (called when component is resized)
    void updatePortPositions(const QPointF& newSourcePort, const QPointF& newTargetPort);
    
    // Save connection to persistence
    void saveConnectionToPersistence();
    void saveConnectionToPersistence(const QPointF& oldSourcePort, const QPointF& oldTargetPort);
    
    ReadyComponentGraphicsItem* getSource() const { return m_source; }
    ReadyComponentGraphicsItem* getTarget() const { return m_target; }
    QPointF getSourcePort() const { return m_sourcePort; }
    QPointF getTargetPort() const { return m_targetPort; }
    QPointF getSourceScenePos() const;
    QPointF getTargetScenePos() const;
    QColor getNeonColor() const { return m_renderer.getWireColor(); }
    
    // Control points management (delegated to WireControlPoints)
    QList<QPointF> getControlPoints() const;
    void setControlPoints(const QList<QPointF>& points);
    void addControlPoint(const QPointF& point);
    void removeControlPoint(int index);
    
    // Routing and style configuration (delegated to components)
    void setRoutingMode(RoutingMode mode) { m_routingMode = mode; updatePath(); }
    RoutingMode getRoutingMode() const { return m_routingMode; }
    
    void setLineStyle(LineStyle style) { m_renderer.setLineStyle(style); update(); }
    LineStyle getLineStyle() const { return m_renderer.getLineStyle(); }
    
    void setWireState(WireState state) { m_renderer.setWireState(state); update(); }
    WireState getWireState() const { return m_renderer.getWireState(); }
    
    void setCustomColor(const QColor& color) { m_renderer.setCustomColor(color); update(); }
    void clearCustomColor() { m_renderer.clearCustomColor(); update(); }
    
    void setWireThickness(int thickness) { m_renderer.setWireThickness(thickness); update(); }
    int getWireThickness() const { return m_renderer.getWireThickness(); }
    
    void setLocked(bool locked);
    bool isLocked() const { return m_renderer.isLocked(); }
    
    void setInverted(bool inverted) { m_isInverted = inverted; update(); }
    bool isInverted() const { return m_isInverted; }
    
    // Label management
    void setLabel(const QString& label);
    QString getLabel() const { return m_labelText; }
    void showLabel(bool show);
    bool isLabelVisible() const { return m_labelVisible; }
    
    // Offset adjustment
    void nudge(int dx, int dy);
    void setOrthogonalOffset(qreal offset) { m_orthogonalOffset = offset; updatePath(); }
    qreal getOrthogonalOffset() const { return m_orthogonalOffset; }
    
    void updatePath();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void animateParticles();
    void onLabelChanged();

private:
    // Component instances
    WireControlPoints m_controlPointsManager;
    WireRenderer m_renderer;
    WireSegments m_segmentsManager;
    
    // Wire endpoints
    ReadyComponentGraphicsItem* m_source;
    ReadyComponentGraphicsItem* m_target;
    QPointF m_sourcePort;
    QPointF m_targetPort;
    QPointF m_temporaryEnd;
    bool m_isTemporary = false;
    
    // Path and routing
    QPainterPath m_path;
    RoutingMode m_routingMode = WirePathBuilder::Orthogonal;
    qreal m_orthogonalOffset = 0.0;
    
    // Animation
    QTimer* m_animationTimer;
    qreal m_animationOffset = 0.0;
    
    // Label
    QGraphicsTextItem* m_label = nullptr;
    QString m_labelText;
    bool m_labelVisible = false;
    
    // Interaction state
    int m_draggedControlPointIndex = -1;
    int m_hoveredControlPointIndex = -1;
    bool m_isDraggingControlPoint = false;
    bool m_isInverted = false;
    
    // Segment adjustment
    int m_selectedSegmentIndex = -1;
    bool m_isDraggingSegment = false;
    QPointF m_segmentDragStart;
    qreal m_segmentOriginalOffset = 0.0;
    
    static constexpr int PORT_RADIUS = 6;
};

#endif // WIREGRAPHICSITEM_H
