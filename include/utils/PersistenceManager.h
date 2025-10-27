/**
 * @file PersistenceManager.h
 * @brief Central persistence manager with modular architecture for schematic data management
 * 
 * This module provides a comprehensive persistence system for the schematic editor,
 * managing the storage and retrieval of all schematic data including components,
 * connections, RTL modules, and text annotations. It implements a modular architecture
 * with specialized persistence components for different data types.
 * 
 * Architecture:
 * - SchematicPersistence: Manages text items and schematic metadata
 * - ComponentPersistence: Handles ready component storage and metadata
 * - RTLModulePersistence: Manages RTL module placement and information
 * - ConnectionPersistence: Handles wire connections and routing data
 * 
 * Key Features:
 * - JSON-based data storage
 * - Data persistence
 * - Component metadata management
 * - Connection persistence with control points
 * - RTL module integration
 * - Text annotation support
 * - Performance optimization with caching
 * - Data validation and repair
 * 
 * Data Storage:
 * - Components: Individual JSON files with metadata
 * - Connections: Centralized JSON file with routing information
 * - RTL Modules: Placement and file path information
 * - Text Items: Annotation data with formatting
 * 
 * @author SCV Project Team
 * @version 1.0
 * @date 2024
 */

#ifndef PERSISTENCEMANAGER_H
#define PERSISTENCEMANAGER_H

#include <QString>
#include <QJsonObject>
#include <QGraphicsScene>
#include <QPointF>
#include <QSizeF>
#include <QColor>
#include <QVariant>
#include <memory>
#include "parsers/SvParser.h"

// Forward declarations for persistence components
class SchematicPersistence;
class ComponentPersistence;
class RTLModulePersistence;
class ConnectionPersistence;

// Graphics item forward declarations
class ReadyComponentGraphicsItem;
class ModuleGraphicsItem;
class WireGraphicsItem;

/**
 * @class PersistenceManager
 * @brief Central persistence manager with modular architecture for schematic data management
 * 
 * This singleton class coordinates all persistence operations for the schematic editor.
 * It delegates specific persistence tasks to specialized components while providing
 * a unified interface for the rest of the application.
 */
class PersistenceManager
{
public:
    /**
     * @brief Get the singleton instance of PersistenceManager
     * @return Reference to the PersistenceManager instance
     * 
     * Returns the singleton instance of the persistence manager.
     * Creates the instance if it doesn't exist.
     */
    static PersistenceManager& instance();
    
    // Set the current working directory
    void setWorkingDirectory(const QString& directory);
    QString getWorkingDirectory() const { return m_workingDirectory; }
    
    // Component persistence
    QString createComponentFile(const QString& componentType, const QPointF& position, const QSizeF& size);
    bool loadComponentsFromDirectory(QGraphicsScene* scene);
    void updateComponentPosition(const QString& componentId, const QPointF& position);
    void updateComponentSize(const QString& componentId, const QSizeF& size);
    void deleteComponentFile(const QString& componentId, bool actuallyDelete = true);
    void removeComponentFromConnections(const QString& componentId);
    void removeComponentOnlyFromConnections(const QString& componentId);
    
    // RTL Module persistence
    void saveRTLModulePlacement(const QString& moduleName, const QString& filePath, const QPointF& position);
    void updateRTLModulePosition(const QString& moduleName, const QPointF& position);
    void updateRTLModulePorts(const QString& moduleName, const QList<Port>& inputs, const QList<Port>& outputs);
    void removeRTLModulePlacement(const QString& moduleName);
    bool loadRTLModules(QGraphicsScene* scene);
    
    // RTL Component persistence (RTL is now a ready component type)
    void updateRTLComponentInConnections(const QString& componentId, const QPointF& position);
    
    // Connection persistence
    void saveConnection(const QString& sourceId, const QPointF& sourcePort,
                       const QString& targetId, const QPointF& targetPort,
                       bool sourceIsRTL = false, bool targetIsRTL = false,
                       const QList<QPointF>& controlPoints = QList<QPointF>(), qreal orthogonalOffset = 0.0);
    void removeConnection(const QString& sourceId, const QPointF& sourcePort,
                         const QString& targetId, const QPointF& targetPort);
    void updateConnectionControlPoints(const QString& sourceId, const QPointF& sourcePort,
                                       const QString& targetId, const QPointF& targetPort,
                                       const QList<QPointF>& controlPoints);
    void updateConnectionOrthogonalOffset(const QString& sourceId, const QPointF& sourcePort,
                                          const QString& targetId, const QPointF& targetPort,
                                          qreal orthogonalOffset);
    bool loadConnections(QGraphicsScene* scene);
    
    // Helper to get component ID from graphics item
    QString getComponentId(ReadyComponentGraphicsItem* component) const;
    void setComponentId(ReadyComponentGraphicsItem* component, const QString& id);
    void unregisterComponent(ReadyComponentGraphicsItem* component);
    
    // Component color persistence
    void updateComponentColor(const QString& componentId, const QColor& color);
    QColor getComponentColor(const QString& componentId);
    
    // Enhanced metadata management
    void updateComponentMetadata(const QString& componentId, const QJsonObject& metadata);
    QJsonObject getComponentMetadata(const QString& componentId);
    void updateComponentProperty(const QString& componentId, const QString& property, const QVariant& value);
    QVariant getComponentProperty(const QString& componentId, const QString& property);
    
    // Component appearance properties
    void updateComponentOpacity(const QString& componentId, qreal opacity);
    void updateComponentVisibility(const QString& componentId, bool visible);
    void updateComponentRotation(const QString& componentId, qreal rotation);
    
    // Port-specific persistence
    void updatePortConfiguration(const QString& componentId, const QString& portName, const QJsonObject& portConfig);
    QJsonObject getPortConfiguration(const QString& componentId, const QString& portName);
    void updateAllPortConfigurations(const QString& componentId, const QJsonObject& portsConfig);
    
    // Component metadata queries
    QJsonObject getComponentGeometry(const QString& componentId);
    QJsonObject getComponentAppearance(const QString& componentId);
    QJsonObject getComponentProperties(const QString& componentId);
    
    // Metadata validation and repair
    bool validateComponentMetadata(const QString& componentId);
    void repairComponentMetadata(const QString& componentId);
    bool migrateLegacyMetadata(const QString& componentId);
    
    // Performance optimization
    void clearMetadataCache();
    
    // RTL connection persistence
    void updateComponentRTLConnection(const QString& componentId, const QString& rtlFilePath);
    QString getComponentRTLConnection(const QString& componentId);
    void generateTopSvIntegration(const QString& componentId, const QString& rtlFilePath);
    
    // Component ID generation
    QString createComponentId(const QString& componentType); // Public wrapper
    
    // RTL component placement persistence
    void saveRTLComponentPlacement(const QString& componentId, const QString& type, 
                                  const QPointF& position, const QSizeF& size);
    
    // Helper to get RTL module name
    QString getRTLModuleName(ModuleGraphicsItem* module) const;
    void setRTLModuleName(ModuleGraphicsItem* module, const QString& name);
    void unregisterRTLModule(ModuleGraphicsItem* module);
    
    QString getRTLModuleFilePath(const QString& moduleName);
    
    // Expose JSON loader for checking existing placements
    QJsonObject loadRTLPlacementsJson();
    
    // New methods for creating component files with actual port information
    QString createRTLModuleFile(const ModuleInfo& moduleInfo, const QString& filePath, const QPointF& position, const QSizeF& size);
    QString getSystemCContentFromModuleInfo(const ModuleInfo& moduleInfo, const QString& componentId);
    QString getSystemCPortType(const Port& port);
    
    // Text item persistence
    void saveTextItem(const QString& text, const QPointF& position, const QColor& color, const QFont& font);
    void updateTextItem(const QString& oldText, const QPointF& oldPosition, const QString& newText, const QPointF& newPosition, const QColor& color, const QFont& font);
    void removeTextItem(const QString& text, const QPointF& position);
    bool loadTextItems(QGraphicsScene* scene);
    
    // Schematic file management
    void initializeSchematicFile();
    QJsonObject loadSchematicJson();
    void saveSchematicJson(const QJsonObject& json);
    
    // Public accessors for component/module lookup (needed by ConnectionPersistence)
    ReadyComponentGraphicsItem* getComponentById(const QString& id) const;
    ReadyComponentGraphicsItem* getRTLModuleByName(const QString& name) const;
    
    // Accessors for persistence components
    SchematicPersistence* getSchematicPersistence() const;
    
private:
    PersistenceManager();
    ~PersistenceManager();
    PersistenceManager(const PersistenceManager&) = delete;
    PersistenceManager& operator=(const PersistenceManager&) = delete;
    
    QString m_workingDirectory;
    QMap<ReadyComponentGraphicsItem*, QString> m_componentIdMap;
    QMap<QString, ReadyComponentGraphicsItem*> m_idToComponentMap;
    QMap<ModuleGraphicsItem*, QString> m_rtlModuleNameMap;
    QMap<QString, ModuleGraphicsItem*> m_nameToRTLModuleMap;
    
    // Persistence component modules
    std::unique_ptr<SchematicPersistence> m_schematicPersistence;
    std::unique_ptr<ComponentPersistence> m_componentPersistence;
    std::unique_ptr<RTLModulePersistence> m_rtlModulePersistence;
    std::unique_ptr<ConnectionPersistence> m_connectionPersistence;
};

#endif // PERSISTENCEMANAGER_H

