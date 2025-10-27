// WireGraphicsItem.cpp - Refactored with composition
#include "graphics/wire/WireGraphicsItem.h"
#include "graphics/ReadyComponentGraphicsItem.h"
#include "graphics/ModuleGraphicsItem.h"
#include "utils/PersistenceManager.h"
#include <QPen>
#include <QCursor>
#include <QGraphicsScene>
#include <QMenu>
#include <QAction>
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QInputDialog>
#include <QColorDialog>
#include <QCoreApplication>

WireGraphicsItem::WireGraphicsItem(ReadyComponentGraphicsItem* source, const QPointF& sourcePort,
                                   ReadyComponentGraphicsItem* target, const QPointF& targetPort,
                                   QGraphicsItem* parent)
    : QObject(), QGraphicsItem(parent), m_source(source), m_target(target),
      m_sourcePort(sourcePort), m_targetPort(targetPort)
{
    setFlag(ItemIsSelectable);
    setFlag(ItemIsFocusable);
    setAcceptHoverEvents(true);
    setZValue(-1); // Draw wires behind components
    
    // Animation timer disabled
    m_animationTimer = nullptr;
    
    // Create label (hidden by default)
    m_label = new QGraphicsTextItem(this);
    m_label->setDefaultTextColor(Qt::white);
    m_label->setFlag(QGraphicsItem::ItemIsSelectable, false);
    m_label->setVisible(false);
    
    updatePath();
}

WireGraphicsItem::~WireGraphicsItem()
{
    if (m_animationTimer) {
        m_animationTimer->stop();
    }
}

QRectF WireGraphicsItem::boundingRect() const
{
    // Larger bounding rect for neon glow and interaction
    return m_path.boundingRect().adjusted(-35, -35, 35, 35);
}

void WireGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // Delegate rendering to WireRenderer
    m_renderer.paint(painter, m_path, isSelected(), m_isTemporary);
    
    // Draw arrow if not temporary
    if (!m_isTemporary && m_target) {
        m_renderer.drawArrow(painter, m_path, m_isInverted);
    }
    
    // Draw locked indicator
    m_renderer.drawLockedIndicator(painter, m_path);
    
    // Draw control points (delegated)
    m_controlPointsManager.drawControlPoints(painter, isSelected(), m_hoveredControlPointIndex);
    
    // Draw segment adjustment arrows if a segment is selected
    if (isSelected() && m_selectedSegmentIndex >= 0 && m_controlPointsManager.isEmpty()) {
        m_segmentsManager.drawSegmentArrows(painter, m_selectedSegmentIndex);
    }
}

QPainterPath WireGraphicsItem::shape() const
{
    QPainterPathStroker stroker;
    stroker.setWidth(15); // Larger selection area
    return stroker.createStroke(m_path);
}

QPointF WireGraphicsItem::getSourceScenePos() const
{
    QPointF portOffset(PORT_RADIUS, PORT_RADIUS);
    return m_source ? m_source->pos() + m_sourcePort + portOffset : m_sourcePort;
}

QPointF WireGraphicsItem::getTargetScenePos() const
{
    QPointF portOffset(PORT_RADIUS, PORT_RADIUS);
    if (m_isTemporary) {
        return m_temporaryEnd;
    } else if (m_target) {
        return m_target->pos() + m_targetPort + portOffset;
    } else {
        return m_targetPort;
    }
}

void WireGraphicsItem::updatePortPositions(const QPointF& newSourcePort, const QPointF& newTargetPort)
{
    // Update stored port positions (called when component is resized)
    m_sourcePort = newSourcePort;
    m_targetPort = newTargetPort;
    
    // Immediately update the wire path with new port positions
    updatePath();
    
    qDebug() << "🔗 Wire port positions updated:"
             << "Source:" << newSourcePort << "Target:" << newTargetPort;
}

void WireGraphicsItem::saveConnectionToPersistence()
{
    // Call the overload with current port positions (used when creating new connections)
    saveConnectionToPersistence(m_sourcePort, m_targetPort);
}

void WireGraphicsItem::saveConnectionToPersistence(const QPointF& oldSourcePort, const QPointF& oldTargetPort)
{
    if (!m_source || !m_target) {
        return;
    }
    
    PersistenceManager& pm = PersistenceManager::instance();
    
    QString sourceId;
    QString targetId;
    bool sourceIsRTL = false;
    bool targetIsRTL = false;
    
    // Check if source is RTL module
    ModuleGraphicsItem* sourceModule = dynamic_cast<ModuleGraphicsItem*>(m_source);
    if (sourceModule && sourceModule->isRTLView()) {
        sourceId = pm.getRTLModuleName(sourceModule);
        sourceIsRTL = true;
    } else {
        sourceId = pm.getComponentId(m_source);
    }
    
    // Check if target is RTL module
    ModuleGraphicsItem* targetModule = dynamic_cast<ModuleGraphicsItem*>(m_target);
    if (targetModule && targetModule->isRTLView()) {
        targetId = pm.getRTLModuleName(targetModule);
        targetIsRTL = true;
    } else {
        targetId = pm.getComponentId(m_target);
    }
    
    if (sourceId.isEmpty() || targetId.isEmpty()) {
        qWarning() << "⚠️ Cannot save connection - missing component IDs";
        return;
    }
    
    // Remove old connection using OLD port positions
    pm.removeConnection(sourceId, oldSourcePort, targetId, oldTargetPort);
    
    // Save new connection with CURRENT port positions
    pm.saveConnection(sourceId, m_sourcePort, targetId, m_targetPort,
                     sourceIsRTL, targetIsRTL, getControlPoints());
    
    qDebug() << "💾 Saved wire connection to persistence:"
             << "Removed old: (" << oldSourcePort << "→" << oldTargetPort << ")"
             << "Added new: (" << m_sourcePort << "→" << m_targetPort << ")";
}

void WireGraphicsItem::updatePath()
{
    prepareGeometryChange();
    
    // Calculate start and end points with port offset
    QPointF start = getSourceScenePos();
    QPointF end = getTargetScenePos();
    
    // Use control points if available, otherwise use routing mode (delegated to WirePathBuilder)
    if (!m_controlPointsManager.isEmpty() && !m_isTemporary) {
        m_path = WirePathBuilder::createPathWithControlPoints(start, end, 
                                                              m_controlPointsManager.getControlPoints());
    } else {
        m_path = WirePathBuilder::createPath(start, end, m_routingMode, m_orthogonalOffset);
    }
    
    // Update label position
    if (m_label && m_labelVisible) {
        QPointF center = m_path.pointAtPercent(0.5);
        m_label->setPos(center - QPointF(m_label->boundingRect().width() / 2, 
                                         m_label->boundingRect().height() / 2));
    }
    
    // Update segments for adjustment (delegated)
    if (m_routingMode == WirePathBuilder::Orthogonal && m_controlPointsManager.isEmpty() && !m_isTemporary) {
        m_segmentsManager.updateFromPath(m_path);
    } else {
        m_segmentsManager.clear();
    }
    
    update();
}

// Control points management (delegated)
QList<QPointF> WireGraphicsItem::getControlPoints() const
{
    return m_controlPointsManager.getControlPoints().toList();
}

void WireGraphicsItem::setControlPoints(const QList<QPointF>& points)
{
    m_controlPointsManager.setControlPoints(points.toVector());
    updatePath();
}

void WireGraphicsItem::addControlPoint(const QPointF& point)
{
    m_controlPointsManager.addControlPoint(point);
    updatePath();
}

void WireGraphicsItem::removeControlPoint(int index)
{
    m_controlPointsManager.removeControlPoint(index);
    updatePath();
}

void WireGraphicsItem::setLocked(bool locked)
{
    m_renderer.setLocked(locked);
    if (locked) {
        m_renderer.setWireState(WireRenderer::Locked);
    } else {
        m_renderer.setWireState(WireRenderer::Normal);
    }
    update();
}

void WireGraphicsItem::setLabel(const QString& label)
{
    m_labelText = label;
    if (m_label) {
        m_label->setPlainText(label);
        QPointF center = m_path.pointAtPercent(0.5);
        m_label->setPos(center - QPointF(m_label->boundingRect().width() / 2, 
                                         m_label->boundingRect().height() / 2));
    }
}

void WireGraphicsItem::showLabel(bool show)
{
    m_labelVisible = show;
    if (m_label) {
        m_label->setVisible(show);
    }
}

void WireGraphicsItem::onLabelChanged()
{
    if (m_label) {
        m_labelText = m_label->toPlainText();
    }
}

void WireGraphicsItem::nudge(int dx, int dy)
{
    // Nudge all control points
    if (!m_controlPointsManager.isEmpty()) {
        m_controlPointsManager.nudgeAll(QPointF(dx * 10, dy * 10));
        updatePath();
    }
}

void WireGraphicsItem::animateParticles()
{
    // Animation disabled
}

// Mouse event handlers
void WireGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_renderer.isLocked()) {
        event->ignore();
        return;
    }
    
    if (event->button() == Qt::LeftButton && isSelected()) {
        int controlPointIndex = m_controlPointsManager.findControlPointAt(event->scenePos());
        
        if (controlPointIndex >= 0) {
            // Start dragging control point
            m_isDraggingControlPoint = true;
            m_draggedControlPointIndex = controlPointIndex;
            event->accept();
            return;
        } else if (event->modifiers() & Qt::ControlModifier) {
            // Ctrl+Click: Add new control point
            QPointF nearestPoint = m_controlPointsManager.findNearestPointOnPath(event->scenePos(), m_path);
            addControlPoint(nearestPoint);
            event->accept();
            return;
        } else if (event->modifiers() & Qt::ShiftModifier) {
            // Shift+Click: Remove nearest control point
            int nearestIndex = m_controlPointsManager.findControlPointAt(event->scenePos());
            if (nearestIndex >= 0) {
                removeControlPoint(nearestIndex);
                event->accept();
                return;
            }
        } else if (m_controlPointsManager.isEmpty() && !m_segmentsManager.isEmpty()) {
            // Check if clicking on any segment (only when in orthogonal mode without control points)
            int segmentIndex = m_segmentsManager.findSegmentAt(event->scenePos());
            if (segmentIndex >= 0) {
                // Allow dragging any segment, not just middle segments
                m_isDraggingSegment = true;
                m_selectedSegmentIndex = segmentIndex;
                m_segmentDragStart = event->scenePos();
                m_segmentOriginalOffset = m_orthogonalOffset;
                event->accept();
                update();
                return;
            }
        }
    }
    
    QGraphicsItem::mousePressEvent(event);
}

void WireGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_isDraggingControlPoint && m_draggedControlPointIndex >= 0) {
        // Update control point position (delegated)
        m_controlPointsManager.updateControlPoint(m_draggedControlPointIndex, event->scenePos());
        updatePath();
        event->accept();
        return;
    }
    
    if (m_isDraggingSegment && m_selectedSegmentIndex >= 0) {
        // Calculate offset based on drag distance
        const WireSegment& segment = m_segmentsManager.getSegment(m_selectedSegmentIndex);
        QPointF dragDelta = event->scenePos() - m_segmentDragStart;
        
        qreal offsetDelta = 0.0;
        if (segment.isVertical) {
            // For vertical segments, drag horizontally changes the offset
            // This matches the horizontal arrows shown on hover
            offsetDelta = dragDelta.x();
        } else if (segment.isHorizontal) {
            // For horizontal segments, drag vertically changes the offset
            // This matches the vertical arrows shown on hover
            offsetDelta = dragDelta.y();
        }
        
        // Apply the offset - this works for middle segments and provides
        // intuitive sliding behavior that matches the arrow directions
        setOrthogonalOffset(m_segmentOriginalOffset + offsetDelta);
        event->accept();
        return;
    }
    
    QGraphicsItem::mouseMoveEvent(event);
}

void WireGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_isDraggingControlPoint) {
        m_isDraggingControlPoint = false;
        m_draggedControlPointIndex = -1;
        
        // Save control points to persistence
        if (m_source && m_target) {
            PersistenceManager& pm = PersistenceManager::instance();
            
            QString sourceId;
            QString targetId;
            
            // Check if source is RTL module
            ModuleGraphicsItem* sourceModule = dynamic_cast<ModuleGraphicsItem*>(m_source);
            if (sourceModule && sourceModule->isRTLView()) {
                sourceId = pm.getRTLModuleName(sourceModule);
            } else {
                sourceId = pm.getComponentId(m_source);
            }
            
            // Check if target is RTL module
            ModuleGraphicsItem* targetModule = dynamic_cast<ModuleGraphicsItem*>(m_target);
            if (targetModule && targetModule->isRTLView()) {
                targetId = pm.getRTLModuleName(targetModule);
            } else {
                targetId = pm.getComponentId(m_target);
            }
            
            if (!sourceId.isEmpty() && !targetId.isEmpty()) {
                pm.updateConnectionControlPoints(sourceId, m_sourcePort, targetId, m_targetPort, 
                                                m_controlPointsManager.getControlPoints());
            }
        }
        
        event->accept();
        return;
    }
    
    if (event->button() == Qt::LeftButton && m_isDraggingSegment) {
        m_isDraggingSegment = false;
        m_selectedSegmentIndex = -1;
        
        // Save offset to persistence
        if (m_source && m_target) {
            PersistenceManager& pm = PersistenceManager::instance();
            
            QString sourceId;
            QString targetId;
            
            // Check if source is RTL module
            ModuleGraphicsItem* sourceModule = dynamic_cast<ModuleGraphicsItem*>(m_source);
            if (sourceModule && sourceModule->isRTLView()) {
                sourceId = pm.getRTLModuleName(sourceModule);
            } else {
                sourceId = pm.getComponentId(m_source);
            }
            
            // Check if target is RTL module
            ModuleGraphicsItem* targetModule = dynamic_cast<ModuleGraphicsItem*>(m_target);
            if (targetModule && targetModule->isRTLView()) {
                targetId = pm.getRTLModuleName(targetModule);
            } else {
                targetId = pm.getComponentId(m_target);
            }
            
            if (!sourceId.isEmpty() && !targetId.isEmpty()) {
                pm.updateConnectionOrthogonalOffset(sourceId, m_sourcePort, targetId, m_targetPort, m_orthogonalOffset);
            }
        }
        
        event->accept();
        update();
        return;
    }
    
    QGraphicsItem::mouseReleaseEvent(event);
}

void WireGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    if (isSelected()) {
        setCursor(QCursor(Qt::PointingHandCursor));
    }
    QGraphicsItem::hoverEnterEvent(event);
}

void WireGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    // Check for control points (delegated)
    int oldHovered = m_hoveredControlPointIndex;
    m_hoveredControlPointIndex = m_controlPointsManager.findControlPointAt(event->scenePos());
    
    int oldSelectedSegment = m_selectedSegmentIndex;
    
    if (isSelected() && m_controlPointsManager.isEmpty() && !m_segmentsManager.isEmpty() && !m_isDraggingSegment) {
        // Check for segments if no control points exist
        int segmentIndex = m_segmentsManager.findSegmentAt(event->scenePos());
        
        // Only allow selecting middle segment
        if (m_segmentsManager.count() == 3 && segmentIndex == 1) {
            m_selectedSegmentIndex = segmentIndex;
        } else if (m_segmentsManager.count() == 5 && segmentIndex == 2) {
            m_selectedSegmentIndex = segmentIndex;
        } else {
            m_selectedSegmentIndex = -1;
        }
    } else if (!m_isDraggingSegment) {
        m_selectedSegmentIndex = -1;
    }
    
    if (oldHovered != m_hoveredControlPointIndex || oldSelectedSegment != m_selectedSegmentIndex) {
        update();
    }
    
    if (isSelected()) {
        if (m_hoveredControlPointIndex >= 0) {
            setCursor(QCursor(Qt::SizeAllCursor));
            setToolTip("Drag to move control point\nShift+Click to remove");
        } else if (m_selectedSegmentIndex >= 0) {
            const WireSegment& segment = m_segmentsManager.getSegment(m_selectedSegmentIndex);
            if (segment.isVertical) {
                setCursor(QCursor(Qt::SizeHorCursor));
            } else {
                setCursor(QCursor(Qt::SizeVerCursor));
            }
            setToolTip("Drag to slide middle segment");
        } else if (event->modifiers() & Qt::ControlModifier) {
            setCursor(QCursor(Qt::CrossCursor));
            setToolTip("Ctrl+Click to add control point");
        } else {
            setCursor(QCursor(Qt::PointingHandCursor));
            setToolTip("Ctrl+Click to add control point\nRight-click for options");
        }
    } else {
        if (m_hoveredControlPointIndex >= 0) {
            setToolTip("Control point - Click wire first to select");
        } else {
            setToolTip("Click to select wire\nRight-click for options");
        }
    }
    
    QGraphicsItem::hoverMoveEvent(event);
}

void WireGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    m_hoveredControlPointIndex = -1;
    if (!m_isDraggingSegment) {
        m_selectedSegmentIndex = -1;
    }
    setCursor(QCursor(Qt::ArrowCursor));
    setToolTip("");
    update();
    QGraphicsItem::hoverLeaveEvent(event);
}

void WireGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    
    // Routing mode submenu
    QMenu* routingMenu = menu.addMenu("Routing Mode");
    QAction* straightAction = routingMenu->addAction("Straight Line");
    straightAction->setCheckable(true);
    straightAction->setChecked(m_routingMode == WirePathBuilder::Straight);
    QAction* orthogonalAction = routingMenu->addAction("Orthogonal (Manhattan)");
    orthogonalAction->setCheckable(true);
    orthogonalAction->setChecked(m_routingMode == WirePathBuilder::Orthogonal);
    QAction* bezierAction = routingMenu->addAction("Bezier Curve");
    bezierAction->setCheckable(true);
    bezierAction->setChecked(m_routingMode == WirePathBuilder::Bezier);
    
    // Line style submenu
    QMenu* styleMenu = menu.addMenu("Line Style");
    QAction* solidAction = styleMenu->addAction("Solid");
    solidAction->setCheckable(true);
    solidAction->setChecked(m_renderer.getLineStyle() == WireRenderer::Solid);
    QAction* dashedAction = styleMenu->addAction("Dashed");
    dashedAction->setCheckable(true);
    dashedAction->setChecked(m_renderer.getLineStyle() == WireRenderer::Dashed);
    QAction* dottedAction = styleMenu->addAction("Dotted");
    dottedAction->setCheckable(true);
    dottedAction->setChecked(m_renderer.getLineStyle() == WireRenderer::Dotted);
    
    menu.addSeparator();
    
    // Label actions
    QAction* addLabelAction = menu.addAction("Add/Edit Label");
    QAction* toggleLabelAction = menu.addAction(m_labelVisible ? "Hide Label" : "Show Label");
    toggleLabelAction->setEnabled(!m_labelText.isEmpty());
    
    menu.addSeparator();
    
    // Wire state submenu
    QMenu* stateMenu = menu.addMenu("Wire State");
    QAction* normalStateAction = stateMenu->addAction("Normal");
    normalStateAction->setCheckable(true);
    normalStateAction->setChecked(m_renderer.getWireState() == WireRenderer::Normal);
    QAction* activeStateAction = stateMenu->addAction("Active");
    activeStateAction->setCheckable(true);
    activeStateAction->setChecked(m_renderer.getWireState() == WireRenderer::Active);
    QAction* errorStateAction = stateMenu->addAction("Error");
    errorStateAction->setCheckable(true);
    errorStateAction->setChecked(m_renderer.getWireState() == WireRenderer::Error);
    
    menu.addSeparator();
    
    // Other actions
    QAction* invertAction = menu.addAction(m_isInverted ? "Reset Direction" : "Invert Direction");
    QAction* lockAction = menu.addAction(m_renderer.isLocked() ? "Unlock Wire" : "Lock Wire");
    QAction* colorAction = menu.addAction("Custom Color...");
    QAction* resetColorAction = menu.addAction("Reset Color");
    resetColorAction->setEnabled(m_renderer.hasCustomColor());
    QAction* resetOffsetAction = menu.addAction("Reset Offset");
    resetOffsetAction->setEnabled(m_orthogonalOffset != 0.0);
    
    menu.addSeparator();
    QAction* deleteAction = menu.addAction("Delete Wire");
    
    QAction* selected = menu.exec(event->screenPos());
    
    if (selected == straightAction) {
        setRoutingMode(WirePathBuilder::Straight);
    } else if (selected == orthogonalAction) {
        setRoutingMode(WirePathBuilder::Orthogonal);
    } else if (selected == bezierAction) {
        setRoutingMode(WirePathBuilder::Bezier);
    } else if (selected == solidAction) {
        setLineStyle(WireRenderer::Solid);
    } else if (selected == dashedAction) {
        setLineStyle(WireRenderer::Dashed);
    } else if (selected == dottedAction) {
        setLineStyle(WireRenderer::Dotted);
    } else if (selected == addLabelAction) {
        bool ok;
        QString text = QInputDialog::getText(nullptr, "Wire Label", "Enter label:", QLineEdit::Normal, m_labelText, &ok);
        if (ok) {
            setLabel(text);
            showLabel(!text.isEmpty());
        }
    } else if (selected == toggleLabelAction) {
        showLabel(!m_labelVisible);
    } else if (selected == normalStateAction) {
        setWireState(WireRenderer::Normal);
    } else if (selected == activeStateAction) {
        setWireState(WireRenderer::Active);
    } else if (selected == errorStateAction) {
        setWireState(WireRenderer::Error);
    } else if (selected == invertAction) {
        setInverted(!m_isInverted);
    } else if (selected == lockAction) {
        setLocked(!m_renderer.isLocked());
    } else if (selected == colorAction) {
        QColor color = QColorDialog::getColor(m_renderer.getWireColor(), nullptr, "Select Wire Color");
        if (color.isValid()) {
            setCustomColor(color);
        }
    } else if (selected == resetColorAction) {
        clearCustomColor();
    } else if (selected == resetOffsetAction) {
        setOrthogonalOffset(0);
    } else if (selected == deleteAction) {
        scene()->clearSelection();
        setSelected(true);
        QKeyEvent* deleteEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
        QCoreApplication::postEvent(scene(), deleteEvent);
    }
    
    event->accept();
}

void WireGraphicsItem::keyPressEvent(QKeyEvent* event)
{
    if (m_renderer.isLocked()) {
        event->ignore();
        return;
    }
    
    switch (event->key()) {
        case Qt::Key_Up:
            nudge(0, -1);
            event->accept();
            break;
        case Qt::Key_Down:
            nudge(0, 1);
            event->accept();
            break;
        case Qt::Key_Left:
            nudge(-1, 0);
            event->accept();
            break;
        case Qt::Key_Right:
            nudge(1, 0);
            event->accept();
            break;
        case Qt::Key_Escape:
            if (isSelected()) {
                setSelected(false);
                event->accept();
            } else {
                event->ignore();
            }
            break;
        default:
            QGraphicsItem::keyPressEvent(event);
            break;
    }
}
