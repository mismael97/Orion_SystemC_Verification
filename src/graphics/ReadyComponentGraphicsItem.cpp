// ReadyComponentGraphicsItem.cpp
#include "graphics/ReadyComponentGraphicsItem.h"
#include "graphics/ready/ComponentPortManager.h"
#include "graphics/ready/ComponentWireManager.h"
#include "graphics/ready/ComponentResizeHandler.h"
#include "graphics/ready/ComponentRenderer.h"
#include "graphics/wire/WireGraphicsItem.h"
#include "utils/PersistenceManager.h"
#include "ui/MainWindow.h"
#include "ui/mainwindow/WidgetManager.h"
#include "ui/widgets/EditComponentWidget.h"
#include "parsers/ComponentPortParser.h"
#include "parsers/SvParser.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QColorDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QCursor>
#include <QtMath>
#include <QDir>
#include <QDebug>
#include <QGraphicsView>

ReadyComponentGraphicsItem::ReadyComponentGraphicsItem(const QString& name, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_name(name)
{
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setCacheMode(DeviceCoordinateCache);
    
    // Set default size based on component type
    if (name == "Transactor") {
        // Transactor is taller - much more height than width
        m_width = 100;
        m_height = 200;
    } else {
        // Default size for other components
        m_width = 120;
        m_height = 80;
    }
    
    // Initialize modular components
    m_portManager = std::make_unique<ComponentPortManager>(m_name, m_width, m_height);
    m_wireManager = std::make_unique<ComponentWireManager>();
    m_resizeHandler = std::make_unique<ComponentResizeHandler>();
    m_renderer = std::make_unique<ComponentRenderer>();
}

ReadyComponentGraphicsItem::~ReadyComponentGraphicsItem()
{
}

qreal ReadyComponentGraphicsItem::getPortRadius() const
{
    return ComponentPortManager::PORT_RADIUS;
}

QRectF ReadyComponentGraphicsItem::boundingRect() const
{
    qreal portRadius = getPortRadius();
    return QRectF(-portRadius, -portRadius, 
                  m_width + portRadius * 3 + 4, 
                  m_height + portRadius * 2);
}

void ReadyComponentGraphicsItem::setSize(qreal width, qreal height)
{
    prepareGeometryChange();
    m_width = qMax(50.0, width);  // Minimum width
    m_height = qMax(40.0, height); // Minimum height
    
    // Update port manager with new dimensions (recalculates port positions)
    m_portManager->updateDimensions(m_width, m_height);
    
    // Update wire port positions to match new port locations
    m_wireManager->updateWirePortPositions(this);
    
    // Update connected wires to follow new port positions
    updateWires();
    
    // Emit signal for real-time synchronization
    emit sizeChanged(QSizeF(m_width, m_height));
}

void ReadyComponentGraphicsItem::setRotation(qreal angle)
{
    if (rotation() != angle) {
        QGraphicsItem::setRotation(angle);
        
        // Update rotation in persistence
        try {
            PersistenceManager& pm = PersistenceManager::instance();
            QString componentId = pm.getComponentId(this);
            if (!componentId.isEmpty()) {
                pm.updateComponentRotation(componentId, angle);
            }
        } catch (const std::exception& e) {
            qWarning() << "⚠️ Exception during rotation update:" << e.what();
        } catch (...) {
            qWarning() << "⚠️ Unknown exception during rotation update";
        }
        
        emit rotationChanged(angle);
    }
}

void ReadyComponentGraphicsItem::setOpacity(qreal opacity)
{
    if (this->opacity() != opacity) {
        QGraphicsItem::setOpacity(opacity);
        emit opacityChanged(opacity);
    }
}

void ReadyComponentGraphicsItem::setVisible(bool visible)
{
    if (isVisible() != visible) {
        QGraphicsItem::setVisible(visible);
        emit visibilityChanged(visible);
    }
}

void ReadyComponentGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    qreal portRadius = getPortRadius();
    
    // Use renderer to paint the component body and name
    m_renderer->paint(painter, option, widget, m_name, m_width, m_height, 
                     isSelected(), m_hasCustomColor, m_customColor, portRadius);
    
    // Draw connection ports
    m_renderer->drawPorts(painter, m_portManager.get(), m_wireManager->getWires(), portRadius);
    
    // Draw resize handle when selected
    if (isSelected()) {
        m_resizeHandler->drawResizeHandle(painter, m_width, m_height, portRadius);
    }
}

void ReadyComponentGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        qreal portRadius = getPortRadius();
        QPointF adjustedPos = event->pos() - QPointF(portRadius, portRadius);
        
        // Check if clicking on resize handle
        if (isSelected() && m_resizeHandler->isInResizeHandle(adjustedPos, m_width, m_height)) {
            m_resizeHandler->startResize(adjustedPos, m_width, m_height);
            event->accept();
            return;
        }
        
        // Check if clicking on a port - let the scene handle it
        if (isNearPort(adjustedPos)) {
            event->accept();
            return;
        }
    }
    QGraphicsItem::mousePressEvent(event);
}

void ReadyComponentGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_resizeHandler->isResizing()) {
        qreal portRadius = getPortRadius();
        QPointF adjustedPos = event->pos() - QPointF(portRadius, portRadius);
        
        // Update component dimensions
        m_resizeHandler->updateResize(adjustedPos, m_width, m_height);
        
        // CRITICAL: Update port positions based on new dimensions
        m_portManager->updateDimensions(m_width, m_height);
        
        // Update geometry for the component itself
        prepareGeometryChange();
        
        // Update wire port positions to match new port locations
        m_wireManager->updateWirePortPositions(this);
        
        // Dynamically update wire paths with new port positions
        updateWires();
        
        // Force scene update to ensure smooth visual feedback
        if (scene()) {
            scene()->update();
        }
        
        update();
        event->accept();
        return;
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void ReadyComponentGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_resizeHandler->isResizing()) {
        m_resizeHandler->endResize();
        
        // Final update of wire port positions after resize
        m_wireManager->updateWirePortPositions(this);
        
        // Final wire path update after resize completes
        updateWires();
        
        // Save the new size to persistence
        try {
            PersistenceManager& pm = PersistenceManager::instance();
            QString componentId = pm.getComponentId(this);
            if (!componentId.isEmpty()) {
                pm.updateComponentSize(componentId, QSizeF(m_width, m_height));
                qDebug() << "💾 Component resized:" << m_name 
                         << "| New size:" << m_width << "x" << m_height
                         << "| Wires with updated port positions:" << m_wireManager->getWires().size();
            }
        } catch (const std::exception& e) {
            qWarning() << "⚠️ Exception during size update:" << e.what();
        } catch (...) {
            qWarning() << "⚠️ Unknown exception during size update";
        }
        
        // Emit signal for real-time synchronization
        emit sizeChanged(QSizeF(m_width, m_height));
        
        // NOTE: Wire port positions are saved to persistence via saveConnectionToPersistence()
        // called from updateWirePortPositions() above
        
        // Force final scene update
        if (scene()) {
            scene()->update();
        }
        
        event->accept();
        return;
    }
    QGraphicsItem::mouseReleaseEvent(event);
}

void ReadyComponentGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        openCodeEditor();
        event->accept();
        return;
    }
    
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void ReadyComponentGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    
    // Style the menu
    menu.setStyleSheet("QMenu { font-family: Tajawal; font-size: 10pt; }"
                      "QMenu::item:selected { background-color: #637AB9; }");
    
    // Add actions
    QAction* editAction = menu.addAction("Edit");
    editAction->setIcon(QIcon());  // You can add icons if you have them
    
    QAction* changeColorAction = menu.addAction("Change Color");
    
    // Show menu at cursor position
    QAction* selectedAction = menu.exec(event->screenPos());
    
    if (selectedAction == editAction) {
        openCodeEditor();
    } else if (selectedAction == changeColorAction) {
        changeComponentColor();
    }
    
    event->accept();
}

void ReadyComponentGraphicsItem::openCodeEditor()
{
    qDebug() << "📂 ReadyComponentGraphicsItem::openCodeEditor() called for component:" << getName();
    // Get component ID and file path
    PersistenceManager& pm = PersistenceManager::instance();
    QString componentId = pm.getComponentId(this);
    
    // Check if working directory is set
    QString workingDir = pm.getWorkingDirectory();
    if (workingDir.isEmpty()) {
        QMessageBox::warning(nullptr, "No Project Loaded",
            QString("Please load or create a project first.\n\n"
                    "Use: File → Open RTL Directory"));
        qWarning() << "No working directory set - cannot edit component";
        return;
    }
    
    // If component doesn't have an ID, create one now
    if (componentId.isEmpty()) {
        qWarning() << "No component ID found for" << m_name << "- creating one now";
        
        // Create the component file and get the ID
        componentId = pm.createComponentFile(m_name, pos(), getSize());
        
        if (!componentId.isEmpty()) {
            pm.setComponentId(this, componentId);
            qDebug() << "✅ Created component ID:" << componentId;
        } else {
            QMessageBox::critical(nullptr, "Error Creating Component",
                QString("Failed to create component file for: %1\n\n"
                        "Working directory: %2\n\n"
                        "Please check directory permissions.").arg(m_name).arg(workingDir));
            qWarning() << "Failed to create component ID for" << m_name;
            return;
        }
    }
    
    // Build file path
    QString filePath = QDir(workingDir).filePath(componentId + ".cpp");
    
    qDebug() << "Opening editor for component:" << componentId << "at" << filePath;
    
    // Find MainWindow and open in edit component widget
    QWidget* widget = scene()->views().first()->window();
    MainWindow* mainWindow = qobject_cast<MainWindow*>(widget);
    
    if (mainWindow && mainWindow->widgetManager()) {
        auto editWidget = mainWindow->widgetManager()->editComponentWidget();
        if (editWidget) {
            // Show the editor panel using the splitter
            QSplitter* splitter = mainWindow->findChild<QSplitter*>("mainHorizontalSplitter");
            QWidget* parentWidget = mainWindow->findChild<QWidget*>("widgetEditComponent");
            
            if (splitter && parentWidget) {
                // Get current sizes
                QList<int> sizes = splitter->sizes();
                if (sizes.size() >= 3) {
                    int totalWidth = sizes[0] + sizes[1] + sizes[2];
                    int desiredEditorWidth = 350;  // Default editor width
                    
                    // If editor is currently hidden (size 0), show it with default width
                    if (sizes[2] == 0) {
                        sizes[1] = totalWidth - sizes[0] - desiredEditorWidth;  // Adjust schematic
                        sizes[2] = desiredEditorWidth;  // Set editor width
                        splitter->setSizes(sizes);
                    }
                    // If editor already has a size, keep it (user may have resized it)
                }
                
                parentWidget->show();
            }
            
            // Load the component file in the editor
            editWidget->loadComponentFile(filePath, m_name);
            
            qDebug() << "✅ Edit component widget opened for:" << m_name;
        } else {
            qWarning() << "Edit component widget not initialized";
        }
    } else {
        qWarning() << "Could not find MainWindow or WidgetManager";
    }
}

void ReadyComponentGraphicsItem::refreshPortsFromFile(const QString& filePath)
{
    qDebug() << "🔄 Refreshing ports from file:" << filePath;
    
    // Parse the component file to get port information
    ModuleInfo moduleInfo = ComponentPortParser::parseComponentFile(filePath);
    
    if (moduleInfo.name.isEmpty()) {
        qWarning() << "Failed to parse component file for port refresh:" << filePath;
        return;
    }
    
    qDebug() << "📊 Parsed ports - Inputs:" << moduleInfo.inputs.size() 
             << "| Outputs:" << moduleInfo.outputs.size();
    
    // Update the port manager with the new port information
    m_portManager->updatePortsFromModuleInfo(moduleInfo);
    
    // Force visual update
    prepareGeometryChange();
    update();
    
    // Update all connected wires to adjust to new port positions
    updateWires();
    
    qDebug() << "✅ Ports refreshed successfully for" << m_name;
}

void ReadyComponentGraphicsItem::changeComponentColor()
{
    // Get current color
    QColor currentColor = m_hasCustomColor ? m_customColor : QColor("#637AB9");
    
    // Open color picker dialog
    QColor newColor = QColorDialog::getColor(currentColor, nullptr, 
                                             "Choose Component Color",
                                             QColorDialog::ShowAlphaChannel);
    
    if (newColor.isValid()) {
        setCustomColor(newColor);
        
        // Save color to persistence
        PersistenceManager& pm = PersistenceManager::instance();
        QString componentId = pm.getComponentId(this);
        if (!componentId.isEmpty()) {
            pm.updateComponentColor(componentId, newColor);
            qDebug() << "Changed color for" << componentId << "to" << newColor.name();
        }
    }
}

void ReadyComponentGraphicsItem::setCustomColor(const QColor& color)
{
    m_customColor = color;
    m_hasCustomColor = true;
    update();  // Repaint the component
    
    // Emit signal for real-time synchronization
    emit colorChanged(color);
}

void ReadyComponentGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    qreal portRadius = getPortRadius();
    QPointF adjustedPos = event->pos() - QPointF(portRadius, portRadius);
    
    QCursor cursor = m_resizeHandler->getCursorForPosition(adjustedPos, isSelected(), m_width, m_height);
    if (cursor.shape() == Qt::SizeFDiagCursor) {
        setCursor(cursor);
    } else if (isNearPort(adjustedPos)) {
        setCursor(Qt::PointingHandCursor);
        setToolTip("Click and drag to connect");
    } else {
        setCursor(Qt::ArrowCursor);
        setToolTip("");
    }
    QGraphicsItem::hoverMoveEvent(event);
}

QVariant ReadyComponentGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged) {
        // Safety check - ensure component is still in a scene and valid
        if (!scene()) {
            qWarning() << "⚠️ Component not in scene during position change";
            return QGraphicsItem::itemChange(change, value);
        }
        
        // Update wires with safety check
        if (m_wireManager) {
            updateWires();
        }
        
        // Update position in persistence
        try {
            PersistenceManager& pm = PersistenceManager::instance();
            QString componentId = pm.getComponentId(this);
            if (!componentId.isEmpty()) {
                qDebug() << "🔄 Component movement detected:" << componentId << "moved to" << pos();
                qDebug() << "📝 Updating meta.json with new position...";
                pm.updateComponentPosition(componentId, pos());
                qDebug() << "✅ Component movement reflected in meta.json successfully";
            } else {
                qWarning() << "⚠️ Component ID not found for position update";
            }
        } catch (const std::exception& e) {
            qWarning() << "⚠️ Exception during position update:" << e.what();
        } catch (...) {
            qWarning() << "⚠️ Unknown exception during position update";
        }
        
        // Emit signal for real-time synchronization
        emit positionChanged(pos());
    }
    return QGraphicsItem::itemChange(change, value);
}

// Port management methods (delegate to ComponentPortManager)
QList<QPointF> ReadyComponentGraphicsItem::getInputPorts() const
{
    return m_portManager->getInputPorts();
}

QList<QPointF> ReadyComponentGraphicsItem::getOutputPorts() const
{
    return m_portManager->getOutputPorts();
}

QPointF ReadyComponentGraphicsItem::getPortAt(const QPointF& pos, bool& isInput) const
{
    return m_portManager->getPortAt(pos, isInput);
}

bool ReadyComponentGraphicsItem::isNearPort(const QPointF& pos) const
{
    return m_portManager->isNearPort(pos);
}

void ReadyComponentGraphicsItem::setHighlightedPort(const QPointF& port)
{
    m_portManager->setHighlightedPort(port);
    update();
}

void ReadyComponentGraphicsItem::clearHighlightedPort()
{
    m_portManager->clearHighlightedPort();
    update();
}

QColor ReadyComponentGraphicsItem::getPortColor(const QPointF& port, bool isInput) const
{
    return m_portManager->getPortColor(port, isInput, m_wireManager->getWires());
}

bool ReadyComponentGraphicsItem::isPortConnected(const QPointF& port, bool isInput) const
{
    return m_portManager->isPortConnected(port, isInput, m_wireManager->getWires());
}

WireGraphicsItem* ReadyComponentGraphicsItem::getWireAtPort(const QPointF& port, bool isInput) const
{
    return m_portManager->getWireAtPort(port, isInput, m_wireManager->getWires());
}

// Wire management methods (delegate to ComponentWireManager)
void ReadyComponentGraphicsItem::addWire(WireGraphicsItem* wire)
{
    m_wireManager->addWire(wire);
}

void ReadyComponentGraphicsItem::removeWire(WireGraphicsItem* wire)
{
    m_wireManager->removeWire(wire);
}

QList<WireGraphicsItem*> ReadyComponentGraphicsItem::getWires() const
{
    return m_wireManager->getWires();
}

void ReadyComponentGraphicsItem::updateWires()
{
    m_wireManager->updateWires();
}

