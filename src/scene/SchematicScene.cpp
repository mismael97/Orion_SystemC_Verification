// SchematicScene.cpp
#include "scene/SchematicScene.h"
#include "scene/WireManager.h"
#include "graphics/ReadyComponentGraphicsItem.h"
#include "graphics/ModuleGraphicsItem.h"
#include "graphics/TextGraphicsItem.h"
#include "graphics/wire/WireGraphicsItem.h"
#include "utils/PersistenceManager.h"
#include "ui/widgets/ComponentPropertiesDialog.h"
#include "persistence/SchematicPersistence.h"
#include <QPainter>
#include <QColor>
#include <QKeyEvent>
#include <QGraphicsItem>
#include <QPainterPath>
#include <QBrush>
#include <cmath>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QMenu>
#include <QAction>

SchematicScene::SchematicScene(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(-1000, -1000, 2000, 2000);
    
    // Initialize wire manager for intelligent routing
    m_wireManager = std::make_unique<WireManager>(this, this);
    qDebug() << "SchematicScene: WireManager initialized";
}

SchematicScene::~SchematicScene()
{
    // Clean up selection rectangle
    cleanupSelectionRectangle();
    
    // Clean up temporary wire
    if (m_temporaryWire) {
        removeItem(m_temporaryWire);
        delete m_temporaryWire;
        m_temporaryWire = nullptr;
    }
    
    // Clear clipboard to avoid dangling pointers
    m_clipboard.clear();
    
    qDebug() << "SchematicScene: Destructor completed - resources cleaned up";
}

void SchematicScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    QColor bgColor = m_darkMode ? QColor(30, 30, 30) : QColor(255, 255, 255); // #1E1E1E / white
    painter->fillRect(rect, bgColor);

    const int gridSize = 20;
    const int majorGridSize = gridSize * 5; // 100px

    QColor normalColor, majorColor;
    if (m_darkMode) {
        normalColor = QColor(45, 45, 45, 80);   // #2D2D2D with reduced opacity
        majorColor = QColor(60, 60, 60, 120);    // #3C3C3C with reduced opacity
    } else {
        normalColor = QColor(240, 240, 240, 100); // Lighter gray with low opacity
        majorColor = QColor(220, 220, 220, 150);  // Lighter gray with low opacity
    }

    QPen normalPen(normalColor);
    normalPen.setWidth(0); // cosmetic pen (1px)
    QPen majorPen(majorColor);
    majorPen.setWidth(0);

    // Use integer coordinates for pixel-perfect alignment
    int left = static_cast<int>(std::floor(rect.left()));
    int top = static_cast<int>(std::floor(rect.top()));
    int right = static_cast<int>(std::ceil(rect.right()));
    int bottom = static_cast<int>(std::ceil(rect.bottom()));

    // Snap to grid
    left = (left / gridSize) * gridSize;
    top = (top / gridSize) * gridSize;

    // Vertical lines
    for (int x = left; x <= right; x += gridSize) {
        painter->setPen((x % majorGridSize) == 0 ? majorPen : normalPen);
        painter->drawLine(x, top, x, bottom);
    }

    // Horizontal lines
    for (int y = top; y <= bottom; y += gridSize) {
        painter->setPen((y % majorGridSize) == 0 ? majorPen : normalPen);
        painter->drawLine(left, y, right, y);
    }
}

void SchematicScene::setDarkMode(bool enabled)
{
    if (m_darkMode != enabled) {
        m_darkMode = enabled;
        invalidate(); // redraw background
    }
}

ReadyComponentGraphicsItem* SchematicScene::getComponentAt(const QPointF& pos)
{
    QList<QGraphicsItem*> itemsAtPos = items(pos);
    for (QGraphicsItem* item : itemsAtPos) {
        ReadyComponentGraphicsItem* component = dynamic_cast<ReadyComponentGraphicsItem*>(item);
        if (component) {
            return component;
        }
    }
    return nullptr;
}

ModuleGraphicsItem* SchematicScene::getModuleAt(const QPointF& pos)
{
    QList<QGraphicsItem*> itemsAtPos = items(pos);
    for (QGraphicsItem* item : itemsAtPos) {
        ModuleGraphicsItem* module = dynamic_cast<ModuleGraphicsItem*>(item);
        if (module && module->isRTLView()) {
            return module;
        }
    }
    return nullptr;
}

QPointF SchematicScene::getPortAt(QGraphicsItem* item, const QPointF& scenePos, bool& isInput, bool& isModule)
{
    ReadyComponentGraphicsItem* readyComp = dynamic_cast<ReadyComponentGraphicsItem*>(item);
    if (readyComp) {
        isModule = false;
        QPointF localPos = readyComp->mapFromScene(scenePos);
        return readyComp->getPortAt(localPos, isInput);
    }
    
    ModuleGraphicsItem* module = dynamic_cast<ModuleGraphicsItem*>(item);
    if (module && module->isRTLView()) {
        isModule = true;
        QPointF localPos = module->mapFromScene(scenePos);
        return module->getPortAt(localPos, isInput);
    }
    
    return QPointF();
}

void SchematicScene::addWireToItem(QGraphicsItem* item, WireGraphicsItem* wire, bool isModule)
{
    if (isModule) {
        ModuleGraphicsItem* module = dynamic_cast<ModuleGraphicsItem*>(item);
        if (module) module->addWire(wire);
    } else {
        ReadyComponentGraphicsItem* comp = dynamic_cast<ReadyComponentGraphicsItem*>(item);
        if (comp) comp->addWire(wire);
    }
}

void SchematicScene::removeWireFromItem(QGraphicsItem* item, WireGraphicsItem* wire, bool isModule)
{
    if (isModule) {
        ModuleGraphicsItem* module = dynamic_cast<ModuleGraphicsItem*>(item);
        if (module) module->removeWire(wire);
    } else {
        ReadyComponentGraphicsItem* comp = dynamic_cast<ReadyComponentGraphicsItem*>(item);
        if (comp) comp->removeWire(wire);
    }
}

void SchematicScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        // Check if clicking on an item first
        QGraphicsItem* clickedItem = itemAt(event->scenePos(), QTransform());
        
        // If clicking on empty space, start selection rectangle
        if (!clickedItem) {
            m_isSelecting = true;
            m_selectionStart = event->scenePos();
            
            // Create selection rectangle
            if (!m_selectionRect) {
                m_selectionRect = new QGraphicsRectItem();
                QPen pen(QColor(0, 120, 215), 1, Qt::DashLine);  // Blue dashed line
                m_selectionRect->setPen(pen);
                QColor fillColor(0, 120, 215, 30);  // Light blue with transparency
                m_selectionRect->setBrush(QBrush(fillColor));
                m_selectionRect->setZValue(1000);  // Draw on top
                addItem(m_selectionRect);
            }
            
            m_selectionRect->setRect(QRectF(m_selectionStart, m_selectionStart));
            m_selectionRect->show();
            
            // Clear selection if not holding Ctrl
            if (!(event->modifiers() & Qt::ControlModifier)) {
                clearSelection();
            }
            
            event->accept();
            return;
        }
        
        // Try ready component first
        ReadyComponentGraphicsItem* readyComp = getComponentAt(event->scenePos());
        ModuleGraphicsItem* module = nullptr;
        QGraphicsItem* sourceItem = nullptr;
        bool isModule = false;
        
        if (readyComp) {
            sourceItem = readyComp;
            isModule = false;
        } else {
            module = getModuleAt(event->scenePos());
            if (module) {
                sourceItem = module;
                isModule = true;
            }
        }
        
        if (sourceItem) {
            bool isInput;
            bool itemIsModule;
            QPointF port = getPortAt(sourceItem, event->scenePos(), isInput, itemIsModule);
            
            if (!port.isNull() && !isInput) {
                // Check if this output port is already connected
                bool portAlreadyConnected = false;
                if (itemIsModule) {
                    ModuleGraphicsItem* mod = dynamic_cast<ModuleGraphicsItem*>(sourceItem);
                    portAlreadyConnected = mod && mod->isPortConnected(port, false);
                } else {
                    ReadyComponentGraphicsItem* comp = dynamic_cast<ReadyComponentGraphicsItem*>(sourceItem);
                    portAlreadyConnected = comp && comp->isPortConnected(port, false);
                }
                
                if (portAlreadyConnected) {
                    // Port already has a connection, don't start new wire
                    event->accept();
                    return;
                }
                
                // Start drawing wire from output port
                m_isDrawingWire = true;
                m_wireSourceItem = sourceItem;
                m_wireSourcePort = port;
                m_wireSourceIsModule = itemIsModule;
                
                // Create temporary wire with source component
                ReadyComponentGraphicsItem* sourceAsReady = itemIsModule ? 
                    reinterpret_cast<ReadyComponentGraphicsItem*>(module) : readyComp;
                m_temporaryWire = new WireGraphicsItem(sourceAsReady, m_wireSourcePort);
                m_temporaryWire->setTemporaryEnd(event->scenePos());
                addItem(m_temporaryWire);
                
                event->accept();
                return;
            }
        }
    }
    
    QGraphicsScene::mousePressEvent(event);
}

void SchematicScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    // Handle selection rectangle
    if (m_isSelecting) {
        updateSelectionRect(event->scenePos());
        event->accept();
        return;
    }
    
    if (m_isDrawingWire && m_temporaryWire) {
        m_temporaryWire->setTemporaryEnd(event->scenePos());
        
        // Clear all highlighted ports first (including RTL modules)
        QList<QGraphicsItem*> allItems = items();
        for (QGraphicsItem* item : allItems) {
            ReadyComponentGraphicsItem* comp = dynamic_cast<ReadyComponentGraphicsItem*>(item);
            if (comp) {
                comp->clearHighlightedPort();
            }
            ModuleGraphicsItem* module = dynamic_cast<ModuleGraphicsItem*>(item);
            if (module) {
                module->clearHighlightedPort();
            }
        }
        
        // Check for target - try both ready component and module
        ReadyComponentGraphicsItem* readyTarget = getComponentAt(event->scenePos());
        ModuleGraphicsItem* moduleTarget = nullptr;
        
        if (!readyTarget) {
            moduleTarget = getModuleAt(event->scenePos());
        }
        
        // Highlight the target port if valid
        if (readyTarget && readyTarget != m_wireSourceItem) {
            bool isInput;
            bool isModule;
            QPointF targetPort = getPortAt(readyTarget, event->scenePos(), isInput, isModule);
            
            if (!targetPort.isNull() && isInput) {
                readyTarget->setHighlightedPort(targetPort);
            }
        } else if (moduleTarget && moduleTarget != m_wireSourceItem) {
            bool isInput;
            bool isModule;
            QPointF targetPort = getPortAt(moduleTarget, event->scenePos(), isInput, isModule);
            
            if (!targetPort.isNull() && isInput) {
                moduleTarget->setHighlightedPort(targetPort);
            }
        }
        
        event->accept();
        return;
    }
    
    QGraphicsScene::mouseMoveEvent(event);
}

void SchematicScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    // Handle selection rectangle completion
    if (m_isSelecting && event->button() == Qt::LeftButton) {
        m_isSelecting = false;
        
        // Hide selection rectangle with safety check
        if (m_selectionRect) {
            // Check if the selection rectangle is still valid
            if (items().contains(m_selectionRect)) {
                m_selectionRect->hide();
            } else {
                qWarning() << "Selection rectangle is no longer in the scene during mouse release";
                m_selectionRect = nullptr;
            }
        }
        
        event->accept();
        
        qDebug() << "Selected" << selectedItems().count() << "items via selection rectangle";
        return;
    }
    
    if (m_isDrawingWire && event->button() == Qt::LeftButton) {
        // Clear all highlighted ports (including RTL modules)
        QList<QGraphicsItem*> allItems = items();
        for (QGraphicsItem* item : allItems) {
            ReadyComponentGraphicsItem* comp = dynamic_cast<ReadyComponentGraphicsItem*>(item);
            if (comp) {
                comp->clearHighlightedPort();
            }
            ModuleGraphicsItem* module = dynamic_cast<ModuleGraphicsItem*>(item);
            if (module) {
                module->clearHighlightedPort();
            }
        }
        
        // Check for target - try both types
        ReadyComponentGraphicsItem* readyTarget = getComponentAt(event->scenePos());
        ModuleGraphicsItem* moduleTarget = nullptr;
        QGraphicsItem* targetItem = nullptr;
        bool targetIsModule = false;
        
        if (readyTarget && readyTarget != m_wireSourceItem) {
            targetItem = readyTarget;
            targetIsModule = false;
        } else if (!readyTarget) {
            moduleTarget = getModuleAt(event->scenePos());
            if (moduleTarget && moduleTarget != m_wireSourceItem) {
                targetItem = moduleTarget;
                targetIsModule = true;
            }
        }
        
        if (targetItem) {
            bool isInput;
            bool itemIsModule;
            QPointF targetPort = getPortAt(targetItem, event->scenePos(), isInput, itemIsModule);
            
            if (!targetPort.isNull() && isInput) {
                // Check if this input port is already connected
                bool portAlreadyConnected = false;
                WireGraphicsItem* existingWire = nullptr;
                
                if (targetIsModule) {
                    ModuleGraphicsItem* mod = dynamic_cast<ModuleGraphicsItem*>(targetItem);
                    if (mod) {
                        portAlreadyConnected = mod->isPortConnected(targetPort, true);
                        if (portAlreadyConnected) {
                            existingWire = mod->getWireAtPort(targetPort, true);
                        }
                    }
                } else {
                    ReadyComponentGraphicsItem* comp = dynamic_cast<ReadyComponentGraphicsItem*>(targetItem);
                    if (comp) {
                        portAlreadyConnected = comp->isPortConnected(targetPort, true);
                        if (portAlreadyConnected) {
                            existingWire = comp->getWireAtPort(targetPort, true);
                        }
                    }
                }
                
                // If port already connected, remove the old wire first
                if (portAlreadyConnected && existingWire) {
                    // Remove wire from both source and target
                    if (existingWire->getSource()) {
                        existingWire->getSource()->removeWire(existingWire);
                    }
                    if (existingWire->getTarget()) {
                        existingWire->getTarget()->removeWire(existingWire);
                    }
                    removeItem(existingWire);
                    delete existingWire;
                }
                
                // Valid connection from output to input
                ReadyComponentGraphicsItem* targetAsReady = targetIsModule ? 
                    reinterpret_cast<ReadyComponentGraphicsItem*>(moduleTarget) : readyTarget;
                    
                m_temporaryWire->setTarget(targetAsReady);
                m_temporaryWire->setTargetPort(targetPort);
                
                // Register wire with both components
                addWireToItem(m_wireSourceItem, m_temporaryWire, m_wireSourceIsModule);
                addWireToItem(targetItem, m_temporaryWire, targetIsModule);
                
                // Register wire with the global wire manager for intelligent routing
                if (m_wireManager) {
                    m_wireManager->registerWire(m_temporaryWire);
                }
                
                // Save connection to JSON file (for ready components and RTL modules)
                PersistenceManager& pm = PersistenceManager::instance();
                QString sourceId;
                QString targetId;
                
                // Get source ID (ready component or RTL module)
                if (m_wireSourceIsModule) {
                    ModuleGraphicsItem* sourceModule = dynamic_cast<ModuleGraphicsItem*>(m_wireSourceItem);
                    sourceId = pm.getRTLModuleName(sourceModule);
                } else {
                    ReadyComponentGraphicsItem* sourceComp = dynamic_cast<ReadyComponentGraphicsItem*>(m_wireSourceItem);
                    sourceId = pm.getComponentId(sourceComp);
                }
                
                // Get target ID (ready component or RTL module)
                if (targetIsModule) {
                    ModuleGraphicsItem* targetModule = dynamic_cast<ModuleGraphicsItem*>(targetItem);
                    targetId = pm.getRTLModuleName(targetModule);
                } else {
                    ReadyComponentGraphicsItem* targetComp = dynamic_cast<ReadyComponentGraphicsItem*>(targetItem);
                    targetId = pm.getComponentId(targetComp);
                }
                
                if (!sourceId.isEmpty() && !targetId.isEmpty()) {
                    // Save connection to connections.json (existing functionality)
                    pm.saveConnection(sourceId, m_wireSourcePort, targetId, targetPort, m_wireSourceIsModule, targetIsModule, QList<QPointF>(), 0.0);
                    
                    // Generate unique wire ID
                    QString wireId = QString("wire_%1_%2_%3").arg(sourceId).arg(targetId).arg(QDateTime::currentMSecsSinceEpoch());
                    
                    // Save wire metadata to meta.json
                    SchematicPersistence* schematicPersistence = pm.getSchematicPersistence();
                    if (schematicPersistence) {
                        // Get wire geometry information
                        QList<QPointF> controlPoints = m_temporaryWire->getControlPoints();
                        qreal orthogonalOffset = m_temporaryWire->getOrthogonalOffset();
                        
                        // Create additional metadata
                        QJsonObject additionalMetadata;
                        additionalMetadata["wireManager"] = "registered";
                        additionalMetadata["routingMode"] = "auto";
                        additionalMetadata["createdInSession"] = true;
                        
                        schematicPersistence->saveWireMetadata(wireId, sourceId, m_wireSourcePort, targetId, targetPort, 
                                                             m_wireSourceIsModule, targetIsModule, 
                                                             controlPoints, orthogonalOffset, additionalMetadata);
                        
                        // Update component connections metadata - Source Component
                        QJsonObject sourceMetadata = pm.getComponentMetadata(sourceId);
                        QJsonObject sourceConnections = sourceMetadata["connections"].toObject();
                        QJsonArray outputConnections = sourceConnections["outputConnections"].toArray();
                        
                        QJsonObject outputConnection;
                        outputConnection["wireId"] = wireId;
                        outputConnection["targetId"] = targetId;
                        QJsonObject targetPortObj;
                        targetPortObj["x"] = targetPort.x();
                        targetPortObj["y"] = targetPort.y();
                        outputConnection["targetPort"] = targetPortObj;
                        outputConnection["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
                        outputConnection["connectionType"] = "output";
                        outputConnections.append(outputConnection);
                        
                        sourceConnections["totalConnections"] = outputConnections.size();
                        sourceConnections["outputConnections"] = outputConnections;
                        sourceConnections["lastConnected"] = QDateTime::currentDateTime().toString(Qt::ISODate);
                        sourceConnections["connectionCount"] = outputConnections.size();
                        
                        sourceMetadata["connections"] = sourceConnections;
                        pm.updateComponentMetadata(sourceId, sourceMetadata);
                        
                        // Update component connections metadata - Target Component
                        QJsonObject targetMetadata = pm.getComponentMetadata(targetId);
                        QJsonObject targetConnections = targetMetadata["connections"].toObject();
                        QJsonArray inputConnections = targetConnections["inputConnections"].toArray();
                        
                        QJsonObject inputConnection;
                        inputConnection["wireId"] = wireId;
                        inputConnection["sourceId"] = sourceId;
                        QJsonObject sourcePortObj;
                        sourcePortObj["x"] = m_wireSourcePort.x();
                        sourcePortObj["y"] = m_wireSourcePort.y();
                        inputConnection["sourcePort"] = sourcePortObj;
                        inputConnection["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
                        inputConnection["connectionType"] = "input";
                        inputConnections.append(inputConnection);
                        
                        targetConnections["totalConnections"] = inputConnections.size();
                        targetConnections["inputConnections"] = inputConnections;
                        targetConnections["lastConnected"] = QDateTime::currentDateTime().toString(Qt::ISODate);
                        targetConnections["connectionCount"] = inputConnections.size();
                        
                        targetMetadata["connections"] = targetConnections;
                        pm.updateComponentMetadata(targetId, targetMetadata);
                        
                        // Also save connection to the connections array in meta.json
                        QJsonObject connectionData;
                        connectionData["id"] = QString("conn_%1").arg(wireId);
                        connectionData["wireId"] = wireId;
                        connectionData["sourceId"] = sourceId;
                        connectionData["targetId"] = targetId;
                        QJsonObject connectionSourcePortObj;
                        connectionSourcePortObj["x"] = m_wireSourcePort.x();
                        connectionSourcePortObj["y"] = m_wireSourcePort.y();
                        connectionData["sourcePort"] = connectionSourcePortObj;
                        
                        QJsonObject connectionTargetPortObj;
                        connectionTargetPortObj["x"] = targetPort.x();
                        connectionTargetPortObj["y"] = targetPort.y();
                        connectionData["targetPort"] = connectionTargetPortObj;
                        connectionData["sourceIsRTL"] = m_wireSourceIsModule;
                        connectionData["targetIsRTL"] = targetIsModule;
                        connectionData["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
                        connectionData["status"] = "active";
                        connectionData["type"] = "wire";
                        
                        // Save connection to connections array
                        pm.saveConnection(sourceId, m_wireSourcePort, targetId, targetPort, m_wireSourceIsModule, targetIsModule, controlPoints, orthogonalOffset);
                        
                        qDebug() << "🔗 Wire metadata saved to meta.json for wire:" << wireId;
                        qDebug() << "🔗 Connection tracking updated for components:" << sourceId << "->" << targetId;
                        qDebug() << "🔗 Connection saved to connections array in meta.json";
                        
                        // Verify wire was saved by checking if it exists in meta.json
                        QJsonObject savedWire = schematicPersistence->getWireMetadata(wireId);
                        if (!savedWire.isEmpty()) {
                            qDebug() << "✅ Wire verification successful - wire found in meta.json";
                        } else {
                            qWarning() << "⚠️ Wire verification failed - wire not found in meta.json";
                        }
                        
                        qDebug() << "📋 Wire recording complete - all data saved to meta.json";
                    }
                    
                    qDebug() << "🔗 Wire created from" << sourceId << "to" << targetId;
                }
                
                m_temporaryWire = nullptr;
                m_isDrawingWire = false;
                m_wireSourceItem = nullptr;
                event->accept();
                return;
            }
        }
        
        // Invalid connection, remove temporary wire
        if (m_temporaryWire) {
            removeItem(m_temporaryWire);
            delete m_temporaryWire;
            m_temporaryWire = nullptr;
        }
        
        m_isDrawingWire = false;
        m_wireSourceItem = nullptr;
        event->accept();
        return;
    }
    
    QGraphicsScene::mouseReleaseEvent(event);
}

void SchematicScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        // Find the item that was double-clicked
        QGraphicsItem* clickedItem = itemAt(event->scenePos(), QTransform());
        
        if (clickedItem) {
            // Check if it's a ready component
            ReadyComponentGraphicsItem* component = dynamic_cast<ReadyComponentGraphicsItem*>(clickedItem);
            if (component) {
                // Get component ID and metadata
                PersistenceManager& pm = PersistenceManager::instance();
                QString componentId = pm.getComponentId(component);
                
                if (!componentId.isEmpty()) {
                    QJsonObject metadata = pm.getComponentMetadata(componentId);
                    
                    if (!metadata.isEmpty()) {
                        // Create and show properties dialog
                        ComponentPropertiesDialog* dialog = ComponentPropertiesDialog::editComponent(
                            componentId, metadata, nullptr);
                        
                        connect(dialog, &ComponentPropertiesDialog::metadataUpdated,
                                this, [this, &pm](const QString& id, const QJsonObject& meta) {
                            // Update the component metadata
                            pm.updateComponentMetadata(id, meta);
                            
                            // Update the component properties in the scene
                            ReadyComponentGraphicsItem* comp = pm.getComponentById(id);
                            if (comp) {
                                // Update geometry
                                QJsonObject geometry = meta["geometry"].toObject();
                                QJsonObject position = geometry["position"].toObject();
                                QJsonObject size = geometry["size"].toObject();
                                
                                comp->setPos(QPointF(position["x"].toDouble(), position["y"].toDouble()));
                                comp->setSize(size["width"].toDouble(), size["height"].toDouble());
                                comp->setRotation(geometry["rotation"].toDouble());
                                
                                // Update appearance
                                QJsonObject appearance = meta["appearance"].toObject();
                                comp->setOpacity(appearance["opacity"].toDouble());
                                comp->setVisible(appearance["visible"].toBool());
                                
                                QString colorStr = appearance["color"].toString();
                                if (!colorStr.isEmpty() && QColor::isValidColorName(colorStr)) {
                                    comp->setCustomColor(QColor(colorStr));
                                }
                                
                                
                                qDebug() << "Component properties updated for:" << id;
                            }
                        });
                        
                        dialog->show();
                        return;
                    } else {
                        qWarning() << "No metadata found for component:" << componentId;
                    }
                } else {
                    qWarning() << "No component ID found for double-clicked component";
                }
            }
            
            // Check if it's a module
            ModuleGraphicsItem* module = dynamic_cast<ModuleGraphicsItem*>(clickedItem);
            if (module) {
                qDebug() << "Double-clicked on module (properties editing not yet implemented)";
                // TODO: Implement module properties editing
            }
        }
    }
    
    QGraphicsScene::mouseDoubleClickEvent(event);
}

void SchematicScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    // Check if right-clicking on empty space (no item at the position)
    QGraphicsItem* itemAtPos = itemAt(event->scenePos(), QTransform());
    
    if (!itemAtPos) {
        // Right-clicking on empty space - show context menu
        QMenu menu;
        
        // Style the menu with Tajawal font for Arabic support
        menu.setStyleSheet("QMenu { font-family: 'Tajawal'; font-size: 10pt; }"
                          "QMenu::item:selected { background-color: #637AB9; }");
        
        // Add "Add Text" action
        QAction* addTextAction = menu.addAction("Add Text");
        addTextAction->setToolTip("Add text at this position");
        
        // Show menu at cursor position
        QAction* selectedAction = menu.exec(event->screenPos());
        
        if (selectedAction == addTextAction) {
            // Emit signal with the scene position where text should be added
            emit addTextRequested(event->scenePos());
        }
        
        event->accept();
        return;
    }
    
    // If clicking on an item, let the item handle its own context menu
    QGraphicsScene::contextMenuEvent(event);
}

void SchematicScene::selectAllItems()
{
    // Select all movable/selectable items (components, modules, text items)
    clearSelection();
    
    QList<QGraphicsItem*> allItems = items();
    int selectedCount = 0;
    
    for (QGraphicsItem* item : allItems) {
        // Select components
        ReadyComponentGraphicsItem* component = dynamic_cast<ReadyComponentGraphicsItem*>(item);
        if (component) {
            item->setSelected(true);
            selectedCount++;
            continue;
        }
        
        // Select RTL modules
        ModuleGraphicsItem* module = dynamic_cast<ModuleGraphicsItem*>(item);
        if (module && module->isRTLView()) {
            item->setSelected(true);
            selectedCount++;
            continue;
        }
        
        // Select text items
        TextGraphicsItem* textItem = dynamic_cast<TextGraphicsItem*>(item);
        if (textItem) {
            item->setSelected(true);
            selectedCount++;
            continue;
        }
        
        // Don't select wires in "select all" to avoid clutter
    }
    
    qDebug() << "Selected" << selectedCount << "items (Ctrl+A)";
}

void SchematicScene::updateSelectionRect(const QPointF& currentPos)
{
    if (!m_selectionRect) {
        return;
    }
    
    // Safety check: ensure the selection rectangle is still valid
    if (!items().contains(m_selectionRect)) {
        qWarning() << "Selection rectangle is no longer in the scene, recreating...";
        m_selectionRect = nullptr;
        return;
    }
    
    // Create rectangle from selection start to current position
    QRectF rect = QRectF(m_selectionStart, currentPos).normalized();
    m_selectionRect->setRect(rect);
    
    // Update selection for items within the rectangle
    QPainterPath selectionPath;
    selectionPath.addRect(rect);
    setSelectionArea(selectionPath);
}

void SchematicScene::cleanupSelectionRectangle()
{
    if (m_selectionRect) {
        if (items().contains(m_selectionRect)) {
            removeItem(m_selectionRect);
        }
        delete m_selectionRect;
        m_selectionRect = nullptr;
        qDebug() << "Selection rectangle cleaned up";
    }
}

void SchematicScene::deleteSelectedItems()
{
    QList<QGraphicsItem*> selected = selectedItems();
    int deletedCount = 0;
    
    for (QGraphicsItem* item : selected) {
        // Check if it's a wire
        WireGraphicsItem* wire = dynamic_cast<WireGraphicsItem*>(item);
        if (wire) {
            // Remove connection from JSON
            PersistenceManager& pm = PersistenceManager::instance();
            ReadyComponentGraphicsItem* source = wire->getSource();
            ReadyComponentGraphicsItem* target = wire->getTarget();
            
            if (source && target) {
                QString sourceId;
                QString targetId;
                
                // Check if source is RTL module
                ModuleGraphicsItem* sourceModule = dynamic_cast<ModuleGraphicsItem*>(source);
                if (sourceModule && sourceModule->isRTLView()) {
                    sourceId = pm.getRTLModuleName(sourceModule);
                } else {
                    sourceId = pm.getComponentId(source);
                }
                
                // Check if target is RTL module
                ModuleGraphicsItem* targetModule = dynamic_cast<ModuleGraphicsItem*>(target);
                if (targetModule && targetModule->isRTLView()) {
                    targetId = pm.getRTLModuleName(targetModule);
                } else {
                    targetId = pm.getComponentId(target);
                }
                
                if (!sourceId.isEmpty() && !targetId.isEmpty()) {
                    pm.removeConnection(sourceId, wire->getSourcePort(), targetId, wire->getTargetPort());
                }
            }
            
            // Notify persistence sync system before deletion
            
            // Remove wire from connected components
            if (wire->getSource()) {
                wire->getSource()->removeWire(wire);
            }
            if (wire->getTarget()) {
                wire->getTarget()->removeWire(wire);
            }
            
            // Unregister from wire manager
            if (m_wireManager) {
                m_wireManager->unregisterWire(wire);
            }
            
            removeItem(wire);
            delete wire;
            deletedCount++;
            continue;
        }
        
        // Check if it's a ready component
        ReadyComponentGraphicsItem* component = dynamic_cast<ReadyComponentGraphicsItem*>(item);
        if (component) {
            PersistenceManager& pm = PersistenceManager::instance();
            
            // Get all connected wires
            QList<WireGraphicsItem*> connectedWires = component->getWires();
            
            // Remove connected wires from scene and from persistence
            for (WireGraphicsItem* connectedWire : connectedWires) {
                // Remove wire from persistence (connections.json)
                
                // Remove wire from connected components
                if (connectedWire->getSource()) {
                    connectedWire->getSource()->removeWire(connectedWire);
                }
                if (connectedWire->getTarget()) {
                    connectedWire->getTarget()->removeWire(connectedWire);
                }
                
                // Unregister from wire manager
                if (m_wireManager) {
                    m_wireManager->unregisterWire(connectedWire);
                }
                
                removeItem(connectedWire);
                delete connectedWire;
            }
            
            // Check if it's an RTL module
            ModuleGraphicsItem* module = dynamic_cast<ModuleGraphicsItem*>(component);
            if (module && module->isRTLView()) {
                // Remove RTL module placement
                QString moduleName = pm.getRTLModuleName(module);
                if (!moduleName.isEmpty()) {
                    // Notify persistence sync system
                    
                    pm.removeRTLModulePlacement(moduleName);
                    pm.unregisterRTLModule(module);  // Unregister from maps BEFORE deletion
                    qDebug() << "🗑️ Deleted RTL module:" << moduleName;
                }
            } else {
                // Remove ready component from persistence
                QString componentId = pm.getComponentId(component);
                if (!componentId.isEmpty()) {
                    
                    pm.deleteComponentFile(componentId, true); // Actually delete files for user deletion
                    pm.unregisterComponent(component);  // Unregister from maps BEFORE deletion
                    qDebug() << "🗑️ Deleted component:" << componentId;
                }
            }
            
            // Remove from scene and delete
            removeItem(component);
            delete component;
            deletedCount++;
            continue;
        }
        
        // Check if it's a text item
        TextGraphicsItem* textItem = dynamic_cast<TextGraphicsItem*>(item);
        if (textItem) {
            // Notify persistence sync system before deletion
            
            // Text items can be deleted
            removeItem(textItem);
            delete textItem;
            deletedCount++;
            continue;
        }
    }
    
    if (deletedCount > 0) {
        qDebug() << "🗑️ Deleted" << deletedCount << "item(s)";
    }
}

void SchematicScene::copySelectedItems()
{
    // Clear clipboard
    m_clipboard.clear();
    
    QList<QGraphicsItem*> selected = selectedItems();
    int copiedCount = 0;
    
    for (QGraphicsItem* item : selected) {
        // Copy text items
        TextGraphicsItem* textItem = dynamic_cast<TextGraphicsItem*>(item);
        if (textItem) {
            m_clipboard.append(item);
            copiedCount++;
        }
        
        // Note: Components and modules are references to actual code,
        // so we don't copy them - they can be dragged from the list instead
    }
    
    qDebug() << "📋 Copied" << copiedCount << "item(s) to clipboard";
}

void SchematicScene::cutSelectedItems()
{
    copySelectedItems();
    deleteSelectedItems();
    qDebug() << "✂️ Cut selected items";
}

void SchematicScene::pasteItems()
{
    if (m_clipboard.isEmpty()) {
        qDebug() << "📋 Clipboard is empty";
        return;
    }
    
    clearSelection();
    int pastedCount = 0;
    QPointF offset(50, 50); // Offset pasted items so they don't overlap originals
    
    for (QGraphicsItem* item : m_clipboard) {
        // Paste text items
        TextGraphicsItem* textItem = dynamic_cast<TextGraphicsItem*>(item);
        if (textItem) {
            TextGraphicsItem* newText = new TextGraphicsItem(textItem->getText());
            newText->setPos(textItem->pos() + offset);
            newText->setTextColor(textItem->getTextColor());
            newText->setTextFont(textItem->getTextFont());
            addItem(newText);
            newText->setSelected(true);
            pastedCount++;
        }
    }
    
    qDebug() << "📄 Pasted" << pastedCount << "item(s)";
}

void SchematicScene::duplicateSelectedItems()
{
    QList<QGraphicsItem*> selected = selectedItems();
    clearSelection();
    int duplicatedCount = 0;
    QPointF offset(50, 50); // Offset duplicated items
    
    for (QGraphicsItem* item : selected) {
        // Duplicate text items
        TextGraphicsItem* textItem = dynamic_cast<TextGraphicsItem*>(item);
        if (textItem) {
            TextGraphicsItem* newText = new TextGraphicsItem(textItem->getText());
            newText->setPos(textItem->pos() + offset);
            newText->setTextColor(textItem->getTextColor());
            newText->setTextFont(textItem->getTextFont());
            addItem(newText);
            newText->setSelected(true);
            duplicatedCount++;
        }
    }
    
    qDebug() << "📑 Duplicated" << duplicatedCount << "item(s)";
}

void SchematicScene::keyPressEvent(QKeyEvent* event)
{
    // Ctrl+A: Select all items
    if (event->matches(QKeySequence::SelectAll)) {
        selectAllItems();
        event->accept();
        return;
    }
    
    // Ctrl+C: Copy
    if (event->matches(QKeySequence::Copy)) {
        copySelectedItems();
        event->accept();
        return;
    }
    
    // Ctrl+X: Cut
    if (event->matches(QKeySequence::Cut)) {
        cutSelectedItems();
        event->accept();
        return;
    }
    
    // Ctrl+V: Paste
    if (event->matches(QKeySequence::Paste)) {
        pasteItems();
        event->accept();
        return;
    }
    
    // Ctrl+D: Duplicate
    if (event->key() == Qt::Key_D && (event->modifiers() & Qt::ControlModifier)) {
        duplicateSelectedItems();
        event->accept();
        return;
    }
    
    // Delete or Backspace: Delete selected items
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        deleteSelectedItems();
        event->accept();
        return;
    }
    
    // Pass other keys to base class
    QGraphicsScene::keyPressEvent(event);
}


void SchematicScene::clearSceneWithPersistenceCleanup()
{
    qDebug() << "🧹 Clearing scene with persistence cleanup...";
    
    // Clean up selection rectangle before clearing scene
    cleanupSelectionRectangle();
    
    // Get all items before clearing
    QList<QGraphicsItem*> allItems = items();
    
    // Safety check: if no items, just clear and return
    if (allItems.isEmpty()) {
        qDebug() << "🧹 No items to clear, scene already empty";
        clear();
        return;
    }
    
    PersistenceManager& pm = PersistenceManager::instance();
    
    // Process each item for proper cleanup
    for (QGraphicsItem* item : allItems) {
        // Handle ready components
        ReadyComponentGraphicsItem* component = dynamic_cast<ReadyComponentGraphicsItem*>(item);
        if (component) {
            QString componentId = pm.getComponentId(component);
            if (!componentId.isEmpty()) {
                // NOTIFY: We do NOT call onComponentDeleted here to prevent clearing persistence files
                // when switching projects. The component cleanup is handled by the new project loading.
                
                // Only unregister from internal maps, don't delete files
                pm.unregisterComponent(component);
                qDebug() << "🔧 Unregistered component:" << componentId << "(files preserved)";
            }
        }
        
        // Handle RTL modules
        ModuleGraphicsItem* module = dynamic_cast<ModuleGraphicsItem*>(item);
        if (module && module->isRTLView()) {
            QString moduleName = pm.getRTLModuleName(module);
            if (!moduleName.isEmpty()) {
                // NOTIFY: We do NOT call onRTLModuleDeleted here to prevent clearing persistence files
                // when switching projects. The module cleanup is handled by the new project loading.
                
                // Only unregister from internal maps, don't delete files
                pm.unregisterRTLModule(module);
                qDebug() << "🔧 Unregistered RTL module:" << moduleName << "(files preserved)";
            }
        }
        
        // Handle wires
        WireGraphicsItem* wire = dynamic_cast<WireGraphicsItem*>(item);
        if (wire) {
            // NOTIFY: We do NOT call onWireDeleted here to prevent clearing persistence files
            // when switching projects. The wire cleanup is handled by the new project loading.
            qDebug() << "🔧 Wire removed from scene (persistence preserved)";
        }
        
        // Handle text items
        TextGraphicsItem* textItem = dynamic_cast<TextGraphicsItem*>(item);
        if (textItem) {
            // NOTIFY: We do NOT call onTextItemDeleted here to prevent clearing persistence files
            // when switching projects. The text item cleanup is handled by the new project loading.
            qDebug() << "🔧 Text item removed from scene (persistence preserved)";
        }
    }
    
    // Clear all items from the scene
    clear();
    
    
    qDebug() << "✅ Scene cleared with persistence cleanup completed (files preserved)";
}

void SchematicScene::clearSceneWithExplicitDeletion()
{
    qDebug() << "🧹 Clearing scene with explicit deletion...";
    
    // Clean up selection rectangle before clearing scene
    cleanupSelectionRectangle();
    
    // Get all items before clearing
    QList<QGraphicsItem*> allItems = items();
    PersistenceManager& pm = PersistenceManager::instance();
    
    // Process each item for proper cleanup with explicit deletion
    for (QGraphicsItem* item : allItems) {
        // Handle ready components
        ReadyComponentGraphicsItem* component = dynamic_cast<ReadyComponentGraphicsItem*>(item);
        if (component) {
            QString componentId = pm.getComponentId(component);
            if (!componentId.isEmpty()) {
                // Notify persistence sync system for explicit deletion
                
                // Delete component file
                pm.deleteComponentFile(componentId, true); // Actually delete files for explicit deletion
                pm.unregisterComponent(component);
                qDebug() << "🗑️ Explicitly deleted component:" << componentId;
            }
        }
        
        // Handle RTL modules
        ModuleGraphicsItem* module = dynamic_cast<ModuleGraphicsItem*>(item);
        if (module && module->isRTLView()) {
            QString moduleName = pm.getRTLModuleName(module);
            if (!moduleName.isEmpty()) {
                
                // Remove RTL module placement
                pm.removeRTLModulePlacement(moduleName);
                pm.unregisterRTLModule(module);
                qDebug() << "🗑️ Explicitly deleted RTL module:" << moduleName;
            }
        }
        
        // Handle wires
        WireGraphicsItem* wire = dynamic_cast<WireGraphicsItem*>(item);
        if (wire) {
            // Notify persistence sync system for explicit deletion
            qDebug() << "🗑️ Explicitly deleted wire";
        }
        
        // Handle text items
        TextGraphicsItem* textItem = dynamic_cast<TextGraphicsItem*>(item);
        if (textItem) {
            // Notify persistence sync system for explicit deletion
            qDebug() << "🗑️ Explicitly deleted text item";
        }
    }
    
    // Clear all items from the scene
    clear();
    
    
    qDebug() << "✅ Scene cleared with explicit deletion completed";
}

