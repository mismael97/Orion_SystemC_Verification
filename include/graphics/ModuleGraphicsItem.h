/**
 * @file ModuleGraphicsItem.h
 * @brief Graphics item for SystemVerilog RTL modules with TLM port visualization
 * 
 * This module provides a specialized graphics item for displaying SystemVerilog RTL modules
 * in the schematic editor. It extends ReadyComponentGraphicsItem to provide RTL-specific
 * functionality including TLM (Transaction Level Modeling) port visualization, module
 * resizing, and detailed RTL view modes.
 * 
 * Key Features:
 * - RTL view mode for compact module representation
 * - TLM port visualization with input/output distinction
 * - Interactive port hovering with width information
 * - Module resizing with visual handles
 * - Integration with wire management system
 * - Persistence support for module placement
 * 
 * @author SCV Project Team
 * @version 1.0
 * @date 2024
 */

#ifndef MODULEGRAPHICSITEM_H
#define MODULEGRAPHICSITEM_H

#include <QGraphicsItem>
#include "graphics/ReadyComponentGraphicsItem.h"
#include "parsers/SvParser.h"
#include <QList>
#include <QSizeF>
#include <QCursor>

class WireGraphicsItem;

/**
 * @class ModuleGraphicsItem
 * @brief Graphics item for SystemVerilog RTL modules with TLM port visualization
 * 
 * This class represents a SystemVerilog RTL module in the schematic editor. It provides
 * two view modes: RTL view (compact representation) and detailed view (with port visualization).
 * The class handles TLM port management, wire connections, and module resizing.
 */
class ModuleGraphicsItem : public ReadyComponentGraphicsItem
{
public:
    /**
     * @brief Constructor for ModuleGraphicsItem
     * @param info ModuleInfo structure containing module name, inputs, and outputs
     * @param parent Parent graphics item (optional)
     * 
     * Creates a new module graphics item with the specified module information.
     * The item starts in RTL view mode by default.
     */
    explicit ModuleGraphicsItem(const ModuleInfo& info, QGraphicsItem *parent = nullptr);
    
    /**
     * @brief Set the RTL view mode
     * @param enabled True to enable RTL view (compact), false for detailed view
     * 
     * RTL view shows a compact representation of the module, while detailed view
     * shows individual ports with their names and widths.
     */
    void setRTLView(bool enabled);
    
    /**
     * @brief Check if RTL view is enabled
     * @return True if in RTL view mode, false otherwise
     */
    bool isRTLView() const;
    
    /**
     * @brief Update module information
     * @param newInfo New module information
     * 
     * Updates the module's port configuration and refreshes the display.
     */
    void updateModuleInfo(const ModuleInfo& newInfo);

    /**
     * @brief Get the bounding rectangle of the module
     * @return QRectF representing the module's bounding rectangle
     * 
     * The bounding rectangle includes the module body, ports, and resize handles.
     */
    QRectF boundingRect() const override;
    
    /**
     * @brief Paint the module graphics item
     * @param painter QPainter object for drawing
     * @param option Style options for the item
     * @param widget Widget being painted on (optional)
     * 
     * Renders the module with appropriate styling based on the current view mode,
     * hover state, and selection status.
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // TLM Port management for RTL components (override base class methods)
    /**
     * @brief Get list of input port positions
     * @return QList<QPointF> containing positions of all input ports
     * 
     * Returns the positions of all input ports in the module. In RTL view,
     * this returns TLM-style port positions.
     */
    QList<QPointF> getInputPorts() const override;
    
    /**
     * @brief Get list of output port positions
     * @return QList<QPointF> containing positions of all output ports
     * 
     * Returns the positions of all output ports in the module. In RTL view,
     * this returns TLM-style port positions.
     */
    QList<QPointF> getOutputPorts() const override;
    
    /**
     * @brief Get port at specified position
     * @param pos Position to check for a port
     * @param isInput Reference to store whether the port is an input
     * @return QPointF position of the port, or invalid point if no port found
     * 
     * Checks if there's a port at the specified position and returns its
     * exact position along with whether it's an input or output port.
     */
    QPointF getPortAt(const QPointF& pos, bool& isInput) const override;
    
    /**
     * @brief Check if position is near any port
     * @param pos Position to check
     * @return True if position is within port detection radius
     * 
     * Determines if the specified position is close enough to any port
     * to be considered a port interaction.
     */
    bool isNearPort(const QPointF& pos) const override;
    
    // Wire management (override base class methods)
    /**
     * @brief Update wire connections
     * 
     * Updates all wire connections attached to this module's ports.
     * Called when the module is moved or resized.
     */
    void updateWires() override;
    
    /**
     * @brief Get color for a specific port
     * @param port Port position
     * @param isInput Whether the port is an input
     * @return QColor for the port
     * 
     * Returns the appropriate color for the port based on its type
     * and connection status.
     */
    QColor getPortColor(const QPointF& port, bool isInput) const override;
    
    /**
     * @brief Check if a port is connected to a wire
     * @param port Port position
     * @param isInput Whether the port is an input
     * @return True if port has a wire connection
     */
    bool isPortConnected(const QPointF& port, bool isInput) const override;
    
    /**
     * @brief Get wire connected to a specific port
     * @param port Port position
     * @param isInput Whether the port is an input
     * @return WireGraphicsItem* connected to the port, or nullptr if none
     */
    WireGraphicsItem* getWireAtPort(const QPointF& port, bool isInput) const override;

protected:
    /**
     * @brief Handle mouse hover enter event
     * @param event Hover event details
     * 
     * Called when the mouse enters the module's hover area.
     * Enables hover effects and updates cursor.
     */
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    
    /**
     * @brief Handle mouse hover leave event
     * @param event Hover event details
     * 
     * Called when the mouse leaves the module's hover area.
     * Disables hover effects and resets cursor.
     */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    
    /**
     * @brief Handle mouse hover move event
     * @param event Hover event details
     * 
     * Called when the mouse moves within the module's hover area.
     * Updates port highlighting and cursor based on hover position.
     */
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    
    /**
     * @brief Handle mouse press event
     * @param event Mouse event details
     * 
     * Handles mouse press events for module interaction, including
     * resize handle detection and port selection.
     */
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    
    /**
     * @brief Handle item change events
     * @param change Type of change
     * @param value New value
     * @return QVariant with the new value
     * 
     * Handles changes to the item's properties, such as position
     * or selection state changes.
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    
    /**
     * @brief Handle context menu event
     * @param event Context menu event details
     * 
     * Shows a context menu with options for editing module properties,
     * including port configuration.
     */
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
    ModuleInfo m_info;              ///< Module information (name, ports, etc.)
    int m_hoveredPortIndex;         ///< Index of currently hovered port
    bool m_isInputHovered;          ///< Whether hovered port is an input
    bool m_isHovering;              ///< Whether mouse is hovering over module
    bool m_isRTLView;               ///< Whether in RTL view mode
    bool m_hovered;                 ///< Whether module is being hovered
    
    // Resize functionality
    /**
     * @enum ResizeHandle
     * @brief Enumeration of resize handle positions
     */
    enum ResizeHandle {
        None,           ///< No resize handle
        TopLeft,        ///< Top-left corner handle
        TopRight,       ///< Top-right corner handle
        BottomLeft,     ///< Bottom-left corner handle
        BottomRight,    ///< Bottom-right corner handle
        Top,            ///< Top edge handle
        Bottom,         ///< Bottom edge handle
        Left,           ///< Left edge handle
        Right           ///< Right edge handle
    };
    
    ResizeHandle m_moduleResizeHandle;      ///< Currently active resize handle
    bool m_moduleIsResizing;                ///< Whether module is being resized
    QPointF m_moduleResizeStartPos;         ///< Starting position for resize operation
    QRectF m_moduleResizeStartRect;         ///< Starting rectangle for resize operation
    QSizeF m_moduleSize;                    ///< Current module size

    // Constants for module rendering
    static const int PORT_RADIUS;                    ///< Radius of individual ports in detailed view
    static const int PORT_SPACING;                   ///< Spacing between ports
    static const int LABEL_HEIGHT;                   ///< Height of port labels
    static const int PADDING;                        ///< Internal padding of module
    static const int TLM_PORT_RADIUS = 6;            ///< Radius of TLM ports in RTL view
    static const int TLM_PORT_DETECTION_RADIUS = 15; ///< Detection radius for TLM ports
    static constexpr int RESIZE_HANDLE_SIZE = 8;     ///< Size of resize handles
    
    /**
     * @brief Draw TLM ports in RTL view mode
     * @param painter QPainter object for drawing
     * 
     * Draws the TLM-style ports when the module is in RTL view mode.
     * Shows input and output ports with appropriate colors and positions.
     */
    void drawTLMPorts(QPainter* painter);
    
    /**
     * @brief Draw resize handles around the module
     * @param painter QPainter object for drawing
     * 
     * Draws the resize handles at the corners and edges of the module
     * when the module is selected and in resize mode.
     */
    void drawResizeHandles(QPainter* painter);
    
    /**
     * @brief Get resize handle at specified position
     * @param pos Position to check
     * @return ResizeHandle enum value indicating which handle is at the position
     * 
     * Determines which resize handle (if any) is at the specified position.
     * Returns None if no handle is found.
     */
    ResizeHandle getResizeHandleAt(const QPointF& pos) const;
    
    /**
     * @brief Get appropriate cursor for resize handle
     * @param handle Resize handle type
     * @return QCursor appropriate for the resize operation
     * 
     * Returns the appropriate cursor (e.g., resize arrows) based on
     * the type of resize handle being used.
     */
    QCursor getCursorForHandle(ResizeHandle handle) const;
};

#endif // MODULEGRAPHICSITEM_H
