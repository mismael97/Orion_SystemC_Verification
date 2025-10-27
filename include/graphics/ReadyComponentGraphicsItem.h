/**
 * @file ReadyComponentGraphicsItem.h
 * @brief Base graphics item for ready-made verification components with modular architecture
 * 
 * This module provides the base class for all ready-made verification components in the
 * schematic editor. It implements a modular architecture that delegates specific
 * responsibilities to specialized manager classes for better maintainability and
 * extensibility.
 * 
 * Architecture:
 * - ComponentPortManager: Handles port positioning, detection, and management
 * - ComponentWireManager: Manages wire connections and routing
 * - ComponentResizeHandler: Handles component resizing operations
 * - ComponentRenderer: Manages rendering and painting operations
 * 
 * Key Features:
 * - Modular design with separated concerns
 * - Port management with input/output distinction
 * - Wire connection support
 * - Component resizing with visual handles
 * - Persistence integration
 * - Signal-based property change notifications
 * 
 * @author SCV Project Team
 * @version 1.0
 * @date 2024
 */

#ifndef READYCOMPONENTGRAPHICSITEM_H
#define READYCOMPONENTGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QList>
#include <QObject>
#include <memory>
#include "parsers/SvParser.h"

class WireGraphicsItem;
class ComponentPortManager;
class ComponentWireManager;
class ComponentResizeHandler;
class ComponentRenderer;

// Forward declarations for ready component modules located in graphics/ready/

/**
 * @class ReadyComponentGraphicsItem
 * @brief Base graphics item for ready-made verification components with modular architecture
 * 
 * This is a refactored modular version that delegates responsibilities to specialized
 * manager classes for better code organization and maintainability:
 * - ComponentPortManager: Port positioning and detection
 * - ComponentWireManager: Wire connection management
 * - ComponentResizeHandler: Resizing operations
 * - ComponentRenderer: Rendering/painting operations
 */
class ReadyComponentGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    /**
     * @brief Constructor for ReadyComponentGraphicsItem
     * @param name Component name/type
     * @param parent Parent graphics item (optional)
     * 
     * Creates a new ready component graphics item with the specified name.
     * Initializes all manager classes and sets up the component for interaction.
     */
    ReadyComponentGraphicsItem(const QString& name, QGraphicsItem* parent = nullptr);
    
    /**
     * @brief Destructor
     * 
     * Cleans up resources and manager objects.
     */
    ~ReadyComponentGraphicsItem() override;

    /**
     * @brief Get the bounding rectangle of the component
     * @return QRectF representing the component's bounding rectangle
     * 
     * Returns the bounding rectangle that encompasses the entire component,
     * including ports and resize handles when visible.
     */
    QRectF boundingRect() const override;
    
    /**
     * @brief Paint the component graphics item
     * @param painter QPainter object for drawing
     * @param option Style options for the item
     * @param widget Widget being painted on (optional)
     * 
     * Renders the component using the ComponentRenderer manager.
     * Handles different visual states (selected, hovered, etc.).
     */
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    // Basic properties
    QString getName() const { return m_name; }
    void setName(const QString& name) { m_name = name; }
    void setSize(qreal width, qreal height);
    QSizeF getSize() const { return QSizeF(m_width, m_height); }
    
    // Custom methods to emit signals when properties change
    void setRotation(qreal angle);
    void setOpacity(qreal opacity);
    void setVisible(bool visible);
    
    // Color management
    void setCustomColor(const QColor& color);
    QColor getCustomColor() const { return m_customColor; }
    bool hasCustomColor() const { return m_hasCustomColor; }
    
    // Port management
    void refreshPortsFromFile(const QString& filePath);
    
    // Port management (delegates to ComponentPortManager)
    virtual QList<QPointF> getInputPorts() const;
    virtual QList<QPointF> getOutputPorts() const;
    virtual QPointF getPortAt(const QPointF& pos, bool& isInput) const;
    virtual bool isNearPort(const QPointF& pos) const;
    void setHighlightedPort(const QPointF& port);
    void clearHighlightedPort();
    
    // Wire management (delegates to ComponentWireManager)
    void addWire(WireGraphicsItem* wire);
    void removeWire(WireGraphicsItem* wire);
    QList<WireGraphicsItem*> getWires() const;
    virtual void updateWires();
    virtual QColor getPortColor(const QPointF& port, bool isInput) const;
    virtual bool isPortConnected(const QPointF& port, bool isInput) const;
    virtual WireGraphicsItem* getWireAtPort(const QPointF& port, bool isInput) const;

    // Additional getters for persistence
    QString getComponentType() const { return m_name; }
    QColor getColor() const { return m_hasCustomColor ? m_customColor : QColor(100, 150, 200); }

signals:
    // Signals for persistence updates
    void positionChanged(const QPointF& newPosition);
    void sizeChanged(const QSizeF& newSize);
    void colorChanged(const QColor& newColor);
    void rotationChanged(qreal newRotation);
    void opacityChanged(qreal newOpacity);
    void visibilityChanged(bool newVisibility);
    void componentDeleted();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    
    // Protected members accessible to derived classes
    std::unique_ptr<ComponentPortManager> m_portManager;
    std::unique_ptr<ComponentWireManager> m_wireManager;
    std::unique_ptr<ComponentResizeHandler> m_resizeHandler;
    std::unique_ptr<ComponentRenderer> m_renderer;

private:
    QString m_name;
    qreal m_width;
    qreal m_height;
    QColor m_customColor;
    bool m_hasCustomColor = false;
    
    // Helper methods
    void openCodeEditor();
    void openPortEditor();
    void changeComponentColor();
    void updateComponentPorts(const ModuleInfo& newInfo);
    QList<Port> generatePorts(int count, const QString& prefix) const;
    qreal getPortRadius() const;
};

#endif // READYCOMPONENTGRAPHICSITEM_H

