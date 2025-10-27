/**
 * @file SchematicScene.h
 * @brief Custom graphics scene for schematic editing with wire management and persistence
 * 
 * This module provides a specialized QGraphicsScene for the schematic editor that handles
 * component placement, wire connections, and selection management.
 * It integrates with the wire management system and provides persistence updates.
 * 
 * Key Features:
 * - Grid-based background rendering with dark/light mode support
 * - Wire drawing and connection management
 * - Component selection and manipulation
 * - Clipboard operations (copy, cut, paste, duplicate)
 * - Persistence management
 * - Intelligent wire routing and collision detection
 * - Selection rectangle for multi-item selection
 * 
 * Architecture:
 * - WireManager: Handles intelligent wire routing and organization
 * - SchematicPersistenceSync: Manages real-time persistence updates
 * - Component interaction: Supports both ready components and RTL modules
 * 
 * @author SCV Project Team
 * @version 1.0
 * @date 2024
 */

#ifndef SCHEMATICSCENE_H
#define SCHEMATICSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <memory>

class ReadyComponentGraphicsItem;
class ModuleGraphicsItem;
class WireGraphicsItem;
class WireManager;

/**
 * @class SchematicScene
 * @brief Custom graphics scene for schematic editing with advanced wire management
 * 
 * This class extends QGraphicsScene to provide specialized functionality for
 * schematic editing, including wire management and component interaction.
 */
class SchematicScene : public QGraphicsScene
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for SchematicScene
     * @param parent Parent QObject (optional)
     * 
     * Initializes the schematic scene with wire management and sets up
     * the scene rectangle for infinite canvas support.
     */
    explicit SchematicScene(QObject *parent = nullptr);
    
    /**
     * @brief Destructor for SchematicScene
     * 
     * Properly cleans up resources including the selection rectangle
     * and other dynamically allocated objects.
     */
    ~SchematicScene();
    
    /**
     * @brief Set dark mode for the scene
     * @param enabled True to enable dark mode, false for light mode
     * 
     * Toggles between dark and light mode, affecting the background grid
     * and overall scene appearance.
     */
    void setDarkMode(bool enabled);
    
    /**
     * @brief Check if dark mode is enabled
     * @return True if dark mode is enabled, false otherwise
     */
    bool isDarkMode() const { return m_darkMode; }
    
    // Wire manager access
    /**
     * @brief Get the wire manager instance
     * @return Pointer to the WireManager instance
     * 
     * Returns the wire manager that handles intelligent routing,
     * collision detection, and wire organization.
     */
    WireManager* getWireManager() const { return m_wireManager.get(); }
    
    
    // Scene management with persistence cleanup
    /**
     * @brief Clear the scene with proper persistence cleanup
     * 
     * Clears all items from the scene while properly cleaning up
     * persistence data and connections.
     */
    void clearSceneWithPersistenceCleanup();
    void clearSceneWithExplicitDeletion();

signals:
    /**
     * @brief Signal emitted when user requests to add text at a specific position
     * @param position The scene position where text should be added
     */
    void addTextRequested(const QPointF& position);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    bool m_darkMode = false;
    bool m_isDrawingWire = false;
    QGraphicsItem* m_wireSourceItem = nullptr; // Can be either ReadyComponent or Module
    QPointF m_wireSourcePort;
    bool m_wireSourceIsModule = false;
    bool m_wireSourceIsInput = false;  // Whether the source port is an input or output
    WireGraphicsItem* m_temporaryWire = nullptr;
    
    // Selection rectangle
    bool m_isSelecting = false;
    QPointF m_selectionStart;
    QGraphicsRectItem* m_selectionRect = nullptr;
    
    // Clipboard
    QList<QGraphicsItem*> m_clipboard;
    
    // Wire management
    std::unique_ptr<WireManager> m_wireManager;
    
    
    ReadyComponentGraphicsItem* getComponentAt(const QPointF& pos);
    ModuleGraphicsItem* getModuleAt(const QPointF& pos);
    QPointF getPortAt(QGraphicsItem* item, const QPointF& scenePos, bool& isInput, bool& isModule);
    void addWireToItem(QGraphicsItem* item, WireGraphicsItem* wire, bool isModule);
    void removeWireFromItem(QGraphicsItem* item, WireGraphicsItem* wire, bool isModule);
    void selectAllItems();
    void updateSelectionRect(const QPointF& currentPos);
    void cleanupSelectionRectangle();
    void deleteSelectedItems();
    void copySelectedItems();
    void cutSelectedItems();
    void pasteItems();
    void duplicateSelectedItems();
};

#endif // SCHEMATICSCENE_H

