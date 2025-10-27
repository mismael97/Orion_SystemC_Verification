// ComponentPersistence.h
#ifndef COMPONENTPERSISTENCE_H
#define COMPONENTPERSISTENCE_H

#include <QString>
#include <QPointF>
#include <QSizeF>
#include <QColor>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>
#include <QTimer>
#include <QDateTime>
#include <QHash>
#include <QSet>
#include <QObject>
#include "parsers/SvParser.h"

class QGraphicsScene;
class QGraphicsItem;

class ComponentPersistence : public QObject
{
    Q_OBJECT
    
public:
    explicit ComponentPersistence(const QString& workingDirectory);
    
    // Component file creation
    QString createComponentFile(const QString& componentType, const QPointF& position, const QSizeF& size);
    QString createRTLModuleFile(const ModuleInfo& moduleInfo, const QString& filePath, 
                               const QPointF& position, const QSizeF& size);
    
    // Component loading
    bool loadComponentsFromDirectory(QGraphicsScene* scene, class PersistenceManager* pm);
    
    // Component updates
    void updateComponentPosition(const QString& componentId, const QPointF& position);
    void updateComponentSize(const QString& componentId, const QSizeF& size);
    void updateComponentColor(const QString& componentId, const QColor& color);
    void updateComponentOpacity(const QString& componentId, qreal opacity);
    void updateComponentVisibility(const QString& componentId, bool visible);
    void updateComponentRotation(const QString& componentId, qreal rotation);
    void deleteComponentFile(const QString& componentId, bool actuallyDelete = true);
    
    // Enhanced metadata management
    void updateComponentMetadata(const QString& componentId, const QJsonObject& metadata);
    QJsonObject getComponentMetadata(const QString& componentId);
    void updateComponentProperty(const QString& componentId, const QString& property, const QVariant& value);
    QVariant getComponentProperty(const QString& componentId, const QString& property);
    
    // Port-specific persistence
    void updatePortConfiguration(const QString& componentId, const QString& portName, const QJsonObject& portConfig);
    QJsonObject getPortConfiguration(const QString& componentId, const QString& portName);
    void updateAllPortConfigurations(const QString& componentId, const QJsonObject& portsConfig);
    
    // Component queries
    QColor getComponentColor(const QString& componentId);
    QJsonObject getComponentGeometry(const QString& componentId);
    QJsonObject getComponentAppearance(const QString& componentId);
    QJsonObject getComponentProperties(const QString& componentId);
    
    // Validation and integrity
    bool validateComponentMetadata(const QString& componentId);
    void repairComponentMetadata(const QString& componentId);
    bool migrateLegacyMetadata(const QString& componentId);
    
    // Working directory
    void setWorkingDirectory(const QString& directory);
    QString getWorkingDirectory() const { return m_workingDirectory; }
    
    // Component ID generation
    QString generateComponentId(const QString& componentType);
    
    // SystemC content generation (public for PersistenceManager access)
    QString getSystemCContent(const QString& componentType, const QString& componentId);
    QString getSystemCContentFromModuleInfo(const ModuleInfo& moduleInfo, const QString& componentId);
    QString getSystemCPortType(const Port& port);
    
    // Performance optimization
    void scheduleMetadataUpdate(const QString& componentId);
    void clearMetadataCache();
    void restoreComponentCounter();
    
    // Public metadata management
    void updateCachedMetadata(const QString& componentId, const QJsonObject& metadata);
    
private slots:
    void performBatchMetadataUpdate();
    
private:
    // Helper methods for enhanced metadata
    QJsonObject createEnhancedMetadata(const QString& componentId, const QString& componentType, 
                                      const QPointF& position, const QSizeF& size, const QColor& color = QColor());
    QJsonObject migrateLegacyMetadataToEnhanced(const QJsonObject& legacyMetadata);
    void restorePortConfigurations(class ReadyComponentGraphicsItem* component, const QJsonObject& portsConfig);
    void restoreComponentProperties(class ReadyComponentGraphicsItem* component, const QJsonObject& propertiesConfig);
    QJsonObject loadMetadataFromFile(const QString& filePath);
    void saveMetadataToFile(const QString& filePath, const QJsonObject& metadata);
    QString getMetadataFilePath(const QString& componentId);
    
    // Metadata caching
    QJsonObject getCachedMetadata(const QString& componentId);
    void saveAllMetadataToFile();
    
private:
    QString m_workingDirectory;
    int m_componentCounter;
    
    // Performance optimization
    std::unique_ptr<QTimer> m_batchUpdateTimer;
    QSet<QString> m_pendingUpdates;
    
    // Metadata caching
    QHash<QString, QJsonObject> m_metadataCache;
    QHash<QString, QDateTime> m_cacheTimestamps;
};

#endif // COMPONENTPERSISTENCE_H

