// DragDropGraphicsView.cpp
#include "ui/widgets/dragdropgraphicsview.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsItemGroup>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QDataStream>
#include <QKeyEvent>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QSysInfo>
#include "parsers/SvParser.h"
#include "graphics/ModuleGraphicsItem.h"
#include "graphics/ReadyComponentGraphicsItem.h"
#include "utils/PersistenceManager.h"
#include "scene/SchematicScene.h"


DragDropGraphicsView::DragDropGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setAcceptDrops(true);
    setRenderHint(QPainter::Antialiasing, false);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate); // ✅ Fixes background rendering during drag
}

void DragDropGraphicsView::setSharedScene(QGraphicsScene* scene)
{
    m_scene = scene;
    setScene(scene);
}

void DragDropGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    // Only accept ready components, reject RTL modules
    if (event->mimeData()->hasFormat("application/x-ready-component")) {
        event->acceptProposedAction();
    } else if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        // RTL module dropping is disabled
        event->ignore();
    }
}

void DragDropGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    // Only accept ready components, reject RTL modules
    if (event->mimeData()->hasFormat("application/x-ready-component")) {   
        event->acceptProposedAction();
    } else if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        // RTL module dropping is disabled
        event->ignore();
    }
}

void DragDropGraphicsView::dropEvent(QDropEvent *event)
{
    if (!m_scene) return;
    
    // Check for ready components first
    if (event->mimeData()->hasFormat("application/x-ready-component")) {
        QString componentName = QString::fromUtf8(event->mimeData()->data("application/x-ready-component"));
        
        QPointF cursorPos = mapToScene(event->position().toPoint());
        
        // All components are treated uniformly now
        
        // Create ready component with standard size
        ReadyComponentGraphicsItem tempItem(componentName);
        QRectF bounds = tempItem.boundingRect();
        QPointF dropPos = cursorPos - QPointF(bounds.width() / 2.0, bounds.height() / 2.0);

        if (event->modifiers() & Qt::ControlModifier) {
            const int gridSize = 20;
            dropPos.setX(qRound(dropPos.x() / gridSize) * gridSize);
            dropPos.setY(qRound(dropPos.y() / gridSize) * gridSize);
        }

        ReadyComponentGraphicsItem* readyComponent = new ReadyComponentGraphicsItem(componentName);
        readyComponent->setPos(dropPos);
        m_scene->addItem(readyComponent);

        // Create SystemC file and register with persistence manager
        PersistenceManager& pm = PersistenceManager::instance();
        QString componentId = pm.createComponentFile(componentName, dropPos, readyComponent->getSize());
        if (!componentId.isEmpty()) {
            pm.setComponentId(readyComponent, componentId);
            qDebug() << "✅ Component dropped:" << componentName << "| ID:" << componentId;
            
            // Enhanced metadata setup for newly dropped component
            QJsonObject initialMetadata = pm.getComponentMetadata(componentId);
            
            // Add comprehensive drag-and-drop specific metadata
            QJsonObject dragDropInfo;
            QDateTime currentTime = QDateTime::currentDateTime();
            dragDropInfo["droppedAt"] = currentTime.toString(Qt::ISODate);
            dragDropInfo["droppedAtTimestamp"] = currentTime.toMSecsSinceEpoch();
            
            QJsonObject dropPosition;
            dropPosition["x"] = dropPos.x();
            dropPosition["y"] = dropPos.y();
            dropPosition["originalX"] = dropPos.x();
            dropPosition["originalY"] = dropPos.y();
            dragDropInfo["dropPosition"] = dropPosition;
            
            QJsonObject dropSize;
            QSizeF componentSize = readyComponent->getSize();
            dropSize["width"] = componentSize.width();
            dropSize["height"] = componentSize.height();
            dropSize["originalWidth"] = componentSize.width();
            dropSize["originalHeight"] = componentSize.height();
            dragDropInfo["dropSize"] = dropSize;
            
            dragDropInfo["snapToGrid"] = bool(event->modifiers() & Qt::ControlModifier);
            dragDropInfo["sourceComponent"] = componentName;
            dragDropInfo["dropMethod"] = "drag-and-drop";
            dragDropInfo["mousePosition"] = QJsonObject{
                {"x", event->pos().x()},
                {"y", event->pos().y()}
            };
            dragDropInfo["keyboardModifiers"] = QJsonArray{
                bool(event->modifiers() & Qt::ControlModifier) ? "Ctrl" : "",
                bool(event->modifiers() & Qt::ShiftModifier) ? "Shift" : "",
                bool(event->modifiers() & Qt::AltModifier) ? "Alt" : ""
            };
            
            // Enhanced component details
            QJsonObject componentDetails;
            componentDetails["description"] = QString("Auto-generated %1 component").arg(componentName);
            componentDetails["author"] = "SCV User";
            componentDetails["category"] = "Ready Component";
            componentDetails["subcategory"] = componentName;
            QJsonArray tags;
            tags.append(componentName);
            tags.append("ready-component");
            tags.append("auto-generated");
            tags.append("drag-drop");
            componentDetails["tags"] = tags;
            componentDetails["editable"] = true;
            componentDetails["version"] = "1.0.0";
            componentDetails["license"] = "MIT";
            componentDetails["documentation"] = QString("Component documentation for %1").arg(componentName);
            componentDetails["keywords"] = QJsonArray{componentName, "logic", "digital", "component"};
            
            // Add session information
            QJsonObject sessionInfo;
            sessionInfo["sessionId"] = QString("session_%1").arg(currentTime.toMSecsSinceEpoch());
            sessionInfo["userAgent"] = "SCV-Editor/1.0";
            sessionInfo["platform"] = QSysInfo::prettyProductName();
            sessionInfo["timestamp"] = currentTime.toString(Qt::ISODate);
            
            initialMetadata["dragDropInfo"] = dragDropInfo;
            initialMetadata["componentDetails"] = componentDetails;
            initialMetadata["sessionInfo"] = sessionInfo;
            
            // Save enhanced metadata
            pm.updateComponentMetadata(componentId, initialMetadata);
            qDebug() << "📝 Enhanced metadata saved for dropped component:" << componentId;
            
            
            // Emit signal for UI updates (if needed)
            emit componentDropped(componentId, componentName, dropPos);
            
        } else {
            qWarning() << "⚠️ Component dropped but no ID created:" << componentName;
            qWarning() << "   Working directory:" << pm.getWorkingDirectory();
            qWarning() << "   The component can still be used, but editing may require re-initialization";
        }

        event->acceptProposedAction();
        return;
    }
    
    // RTL module dropping is disabled - only ready components can be dropped
    // Ignore any RTL module drop attempts
    if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        qDebug() << "🚫 RTL module dropping is disabled - ignoring drop attempt";
        event->ignore();
        return;
    }
}

void DragDropGraphicsView::updateScene()
{
    if (m_scene) {
        m_scene->update();
    }
}

void DragDropGraphicsView::wheelEvent(QWheelEvent *event)
{
    double scaleFactor = event->angleDelta().y() > 0 ? 1.2 : 0.8;
    double newScale = m_currentScale * scaleFactor;

    if (newScale < MIN_SCALE) {
        newScale = MIN_SCALE;
        scaleFactor = newScale / m_currentScale;
    } else if (newScale > MAX_SCALE) {
        newScale = MAX_SCALE;
        scaleFactor = newScale / m_currentScale;
    }

    scale(scaleFactor, scaleFactor);
    m_currentScale = newScale;
    event->accept();
}

void DragDropGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void DragDropGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPanning) {
        QPoint delta = m_lastPanPoint - event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() + delta.y());
        m_lastPanPoint = event->pos();
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void DragDropGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void DragDropGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        scale(1.0 / m_currentScale, 1.0 / m_currentScale);
        m_currentScale = 1.0;
        event->accept();
        return;
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void DragDropGraphicsView::keyPressEvent(QKeyEvent *event)
{
    // Let the scene handle the key event (including Delete key)
    QGraphicsView::keyPressEvent(event);
}

void DragDropGraphicsView::panUp(int amount)
{
    QPointF center = mapToScene(viewport()->rect().center());
    center.setY(center.y() - amount);
    centerOn(center);
}

void DragDropGraphicsView::panDown(int amount)
{
    QPointF center = mapToScene(viewport()->rect().center());
    center.setY(center.y() + amount);
    centerOn(center);
}

void DragDropGraphicsView::panLeft(int amount)
{
    QPointF center = mapToScene(viewport()->rect().center());
    center.setX(center.x() - amount);
    centerOn(center);
}

void DragDropGraphicsView::panRight(int amount)
{
    QPointF center = mapToScene(viewport()->rect().center());
    center.setX(center.x() + amount);
    centerOn(center);
}
