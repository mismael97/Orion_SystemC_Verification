// PersistenceManager.cpp (Refactored with modular persistence components)
#include "utils/PersistenceManager.h"
#include "persistence/SchematicPersistence.h"
#include "persistence/ComponentPersistence.h"
#include "persistence/RTLModulePersistence.h"
#include "persistence/ConnectionPersistence.h"
#include "graphics/ReadyComponentGraphicsItem.h"
#include "graphics/ModuleGraphicsItem.h"
#include "graphics/wire/WireGraphicsItem.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QRegularExpression>
#include <QTextStream>

PersistenceManager::PersistenceManager()
{
    // Persistence modules will be initialized when working directory is set
}

PersistenceManager::~PersistenceManager()
{
    // Unique pointers will clean up automatically
}

PersistenceManager& PersistenceManager::instance()
{
    static PersistenceManager inst;
    return inst;
}

void PersistenceManager::setWorkingDirectory(const QString& directory)
{
    m_workingDirectory = directory;
    m_componentIdMap.clear();
    m_idToComponentMap.clear();
    m_rtlModuleNameMap.clear();
    m_nameToRTLModuleMap.clear();
    
    // Initialize persistence modules with the working directory
    m_schematicPersistence = std::make_unique<SchematicPersistence>(directory);
    m_componentPersistence = std::make_unique<ComponentPersistence>(directory);
    m_rtlModulePersistence = std::make_unique<RTLModulePersistence>(directory);
    m_connectionPersistence = std::make_unique<ConnectionPersistence>(directory);
    
    qDebug() << "📂 PersistenceManager: Working directory set to" << directory;
}

// Component ID management
QString PersistenceManager::getComponentId(ReadyComponentGraphicsItem* component) const
{
    return m_componentIdMap.value(component, QString());
}

void PersistenceManager::setComponentId(ReadyComponentGraphicsItem* component, const QString& id)
{
    m_componentIdMap[component] = id;
    m_idToComponentMap[id] = component;
}

void PersistenceManager::unregisterComponent(ReadyComponentGraphicsItem* component)
{
    QString id = m_componentIdMap.value(component, QString());
    if (!id.isEmpty()) {
        m_componentIdMap.remove(component);
        m_idToComponentMap.remove(id);
        qDebug() << "🔓 Unregistered component:" << id;
    }
}

ReadyComponentGraphicsItem* PersistenceManager::getComponentById(const QString& id) const
{
    return m_idToComponentMap.value(id, nullptr);
}

// RTL Module name management
QString PersistenceManager::getRTLModuleName(ModuleGraphicsItem* module) const
{
    return m_rtlModuleNameMap.value(module, QString());
}

void PersistenceManager::setRTLModuleName(ModuleGraphicsItem* module, const QString& name)
{
    m_rtlModuleNameMap[module] = name;
    m_nameToRTLModuleMap[name] = module;
}

void PersistenceManager::unregisterRTLModule(ModuleGraphicsItem* module)
{
    QString name = m_rtlModuleNameMap.value(module, QString());
    if (!name.isEmpty()) {
        m_rtlModuleNameMap.remove(module);
        m_nameToRTLModuleMap.remove(name);
        qDebug() << "🔓 Unregistered RTL module:" << name;
    }
}

ReadyComponentGraphicsItem* PersistenceManager::getRTLModuleByName(const QString& name) const
{
    return m_nameToRTLModuleMap.value(name, nullptr);
}


// Component operations (delegated to ComponentPersistence)
QString PersistenceManager::createComponentId(const QString& componentType)
{
    if (!m_componentPersistence) return QString();
    return m_componentPersistence->generateComponentId(componentType);
}

QString PersistenceManager::createComponentFile(const QString& componentType, const QPointF& position, const QSizeF& size)
{
    if (!m_componentPersistence) return QString();
    return m_componentPersistence->createComponentFile(componentType, position, size);
}

QString PersistenceManager::createRTLModuleFile(const ModuleInfo& moduleInfo, const QString& filePath,
                                               const QPointF& position, const QSizeF& size)
{
    if (!m_componentPersistence || !m_rtlModulePersistence) return QString();

    QString componentId = m_componentPersistence->createRTLModuleFile(moduleInfo, filePath, position, size);

    // Save RTL module placement
    if (!componentId.isEmpty()) {
        m_rtlModulePersistence->saveRTLModulePlacement(moduleInfo.name, filePath, position);
    }
    
    return componentId;
}

bool PersistenceManager::loadComponentsFromDirectory(QGraphicsScene* scene)
{
    if (!m_componentPersistence) return false;
    return m_componentPersistence->loadComponentsFromDirectory(scene, this);
}

void PersistenceManager::updateComponentPosition(const QString& componentId, const QPointF& position)
{
    qDebug() << "🔄 PersistenceManager: Updating component position for" << componentId << "to" << position;
    
    if (m_componentPersistence) {
        m_componentPersistence->updateComponentPosition(componentId, position);
        qDebug() << "✅ PersistenceManager: Position update completed for" << componentId;
    } else {
        qWarning() << "⚠️ PersistenceManager: ComponentPersistence not available";
    }
    
    if (m_connectionPersistence) {
        m_connectionPersistence->updateRTLComponentInConnections(componentId, position);
    }
}

void PersistenceManager::updateComponentSize(const QString& componentId, const QSizeF& size)
{
    if (m_componentPersistence) {
        m_componentPersistence->updateComponentSize(componentId, size);
    }
}

void PersistenceManager::updateComponentColor(const QString& componentId, const QColor& color)
{
    if (m_componentPersistence) {
        m_componentPersistence->updateComponentColor(componentId, color);
    }
}

QColor PersistenceManager::getComponentColor(const QString& componentId)
{
    if (!m_componentPersistence) return QColor();
    return m_componentPersistence->getComponentColor(componentId);
}

// Enhanced metadata management
void PersistenceManager::updateComponentMetadata(const QString& componentId, const QJsonObject& metadata)
{
    if (!m_componentPersistence) {
        qWarning() << "ComponentPersistence not initialized";
        return;
    }
    
    // Update the cached metadata with enhanced information
    QJsonObject enhancedMetadata = metadata;
    
    // Update timestamps
    QDateTime currentTime = QDateTime::currentDateTime();
    enhancedMetadata["modified"] = currentTime.toString(Qt::ISODate);
    enhancedMetadata["modifiedTimestamp"] = currentTime.toMSecsSinceEpoch();
    enhancedMetadata["lastAccessed"] = currentTime.toString(Qt::ISODate);
    
    // Update statistics
    QJsonObject statistics = enhancedMetadata["statistics"].toObject();
    statistics["usageCount"] = statistics["usageCount"].toInt() + 1;
    statistics["lastUsed"] = currentTime.toString(Qt::ISODate);
    enhancedMetadata["statistics"] = statistics;
    
    // Add to modification history
    QJsonObject history = enhancedMetadata["history"].toObject();
    QJsonArray modificationHistory = history["modificationHistory"].toArray();
    QJsonObject modificationEvent;
    modificationEvent["action"] = "metadata-update";
    modificationEvent["timestamp"] = currentTime.toString(Qt::ISODate);
    modificationEvent["user"] = "system";
    modificationEvent["details"] = "Component metadata updated";
    modificationHistory.append(modificationEvent);
    history["modificationHistory"] = modificationHistory;
    enhancedMetadata["history"] = history;
    
    m_componentPersistence->updateCachedMetadata(componentId, enhancedMetadata);
    qDebug() << "Updated enhanced metadata for component:" << componentId;
}

QJsonObject PersistenceManager::getComponentMetadata(const QString& componentId)
{
    if (!m_componentPersistence) return QJsonObject();
    return m_componentPersistence->getComponentMetadata(componentId);
}

void PersistenceManager::updateComponentProperty(const QString& componentId, const QString& property, const QVariant& value)
{
    if (m_componentPersistence) {
        m_componentPersistence->updateComponentProperty(componentId, property, value);
    }
}

QVariant PersistenceManager::getComponentProperty(const QString& componentId, const QString& property)
{
    if (!m_componentPersistence) return QVariant();
    return m_componentPersistence->getComponentProperty(componentId, property);
}

// Component appearance properties
void PersistenceManager::updateComponentOpacity(const QString& componentId, qreal opacity)
{
    if (m_componentPersistence) {
        m_componentPersistence->updateComponentOpacity(componentId, opacity);
    }
}

void PersistenceManager::updateComponentVisibility(const QString& componentId, bool visible)
{
    if (m_componentPersistence) {
        m_componentPersistence->updateComponentVisibility(componentId, visible);
    }
}

void PersistenceManager::updateComponentRotation(const QString& componentId, qreal rotation)
{
    if (m_componentPersistence) {
        m_componentPersistence->updateComponentRotation(componentId, rotation);
    }
}

// Port-specific persistence
void PersistenceManager::updatePortConfiguration(const QString& componentId, const QString& portName, const QJsonObject& portConfig)
{
    if (m_componentPersistence) {
        m_componentPersistence->updatePortConfiguration(componentId, portName, portConfig);
    }
}

QJsonObject PersistenceManager::getPortConfiguration(const QString& componentId, const QString& portName)
{
    if (!m_componentPersistence) return QJsonObject();
    return m_componentPersistence->getPortConfiguration(componentId, portName);
}

void PersistenceManager::updateAllPortConfigurations(const QString& componentId, const QJsonObject& portsConfig)
{
    if (m_componentPersistence) {
        m_componentPersistence->updateAllPortConfigurations(componentId, portsConfig);
    }
}

// Component metadata queries
QJsonObject PersistenceManager::getComponentGeometry(const QString& componentId)
{
    if (!m_componentPersistence) return QJsonObject();
    return m_componentPersistence->getComponentGeometry(componentId);
}

QJsonObject PersistenceManager::getComponentAppearance(const QString& componentId)
{
    if (!m_componentPersistence) return QJsonObject();
    return m_componentPersistence->getComponentAppearance(componentId);
}

QJsonObject PersistenceManager::getComponentProperties(const QString& componentId)
{
    if (!m_componentPersistence) return QJsonObject();
    return m_componentPersistence->getComponentProperties(componentId);
}

// Metadata validation and repair
bool PersistenceManager::validateComponentMetadata(const QString& componentId)
{
    if (!m_componentPersistence) return false;
    return m_componentPersistence->validateComponentMetadata(componentId);
}

void PersistenceManager::repairComponentMetadata(const QString& componentId)
{
    if (m_componentPersistence) {
        m_componentPersistence->repairComponentMetadata(componentId);
    }
}

bool PersistenceManager::migrateLegacyMetadata(const QString& componentId)
{
    if (!m_componentPersistence) return false;
    return m_componentPersistence->migrateLegacyMetadata(componentId);
}

// Performance optimization
void PersistenceManager::clearMetadataCache()
{
    if (m_componentPersistence) {
        m_componentPersistence->clearMetadataCache();
    }
}

void PersistenceManager::deleteComponentFile(const QString& componentId, bool actuallyDelete)
{
    if (m_componentPersistence) {
        m_componentPersistence->deleteComponentFile(componentId, actuallyDelete);
    }
    
    if (m_connectionPersistence) {
        if (actuallyDelete) {
            // For user deletion: remove component and all its connections
            m_connectionPersistence->removeComponentFromConnections(componentId);
        } else {
            // For scene clearing: only remove component from list, preserve connections
            m_connectionPersistence->removeComponentOnlyFromConnections(componentId);
        }
    }
}

void PersistenceManager::removeComponentFromConnections(const QString& componentId)
{
    if (m_connectionPersistence) {
        m_connectionPersistence->removeComponentFromConnections(componentId);
    }
}

void PersistenceManager::removeComponentOnlyFromConnections(const QString& componentId)
{
    if (m_connectionPersistence) {
        m_connectionPersistence->removeComponentOnlyFromConnections(componentId);
    }
}

// RTL Module operations (delegated to RTLModulePersistence)
void PersistenceManager::saveRTLModulePlacement(const QString& moduleName, const QString& filePath, const QPointF& position)
{
    qDebug() << "💾 PersistenceManager::saveRTLModulePlacement() called for module:" << moduleName << "at file:" << filePath;
    if (m_rtlModulePersistence) {
        m_rtlModulePersistence->saveRTLModulePlacement(moduleName, filePath, position);
    }
}

void PersistenceManager::updateRTLModulePosition(const QString& moduleName, const QPointF& position)
{
    if (m_rtlModulePersistence) {
        m_rtlModulePersistence->updateRTLModulePosition(moduleName, position);
    }
}

void PersistenceManager::removeRTLModulePlacement(const QString& moduleName)
{
    if (m_rtlModulePersistence) {
        m_rtlModulePersistence->removeRTLModulePlacement(moduleName);
    }
}

bool PersistenceManager::loadRTLModules(QGraphicsScene* scene)
{
    if (!m_rtlModulePersistence) return false;
    return m_rtlModulePersistence->loadRTLModules(scene, this);
}

QString PersistenceManager::getRTLModuleFilePath(const QString& moduleName)
{
    if (!m_rtlModulePersistence) return QString();
    return m_rtlModulePersistence->getRTLModuleFilePath(moduleName);
}

QJsonObject PersistenceManager::loadRTLPlacementsJson()
{
    if (!m_rtlModulePersistence) return QJsonObject();
    return m_rtlModulePersistence->loadRTLPlacementsJson();
}

// Connection operations (delegated to ConnectionPersistence)
void PersistenceManager::saveConnection(const QString& sourceId, const QPointF& sourcePort,
                                       const QString& targetId, const QPointF& targetPort,
                                       bool sourceIsRTL, bool targetIsRTL,
                                       const QList<QPointF>& controlPoints, qreal orthogonalOffset)
{
    qDebug() << "🔗 PersistenceManager::saveConnection() called from:" << sourceId << "to:" << targetId;
    if (m_connectionPersistence) {
        m_connectionPersistence->saveConnection(sourceId, sourcePort, targetId, targetPort,
                                               sourceIsRTL, targetIsRTL, controlPoints, orthogonalOffset);
    }
}

void PersistenceManager::removeConnection(const QString& sourceId, const QPointF& sourcePort,
                                          const QString& targetId, const QPointF& targetPort)
{
    if (m_connectionPersistence) {
        m_connectionPersistence->removeConnection(sourceId, sourcePort, targetId, targetPort);
    }
}

void PersistenceManager::updateConnectionControlPoints(const QString& sourceId, const QPointF& sourcePort,
                                                       const QString& targetId, const QPointF& targetPort,
                                                       const QList<QPointF>& controlPoints)
{
    if (m_connectionPersistence) {
        m_connectionPersistence->updateConnectionControlPoints(sourceId, sourcePort, targetId, targetPort, controlPoints);
    }
}

void PersistenceManager::updateConnectionOrthogonalOffset(const QString& sourceId, const QPointF& sourcePort,
                                                          const QString& targetId, const QPointF& targetPort,
                                                          qreal orthogonalOffset)
{
    if (m_connectionPersistence) {
        m_connectionPersistence->updateConnectionOrthogonalOffset(sourceId, sourcePort, targetId, targetPort, orthogonalOffset);
    }
}

bool PersistenceManager::loadConnections(QGraphicsScene* scene)
{
    if (!m_connectionPersistence) return false;
    return m_connectionPersistence->loadConnections(scene, this);
}


void PersistenceManager::updateRTLComponentInConnections(const QString& componentId, const QPointF& position)
{
    if (m_connectionPersistence) {
        m_connectionPersistence->updateRTLComponentInConnections(componentId, position);
    }
}


// Schematic and text item operations (delegated to SchematicPersistence)
void PersistenceManager::saveTextItem(const QString& text, const QPointF& position,
                                     const QColor& color, const QFont& font)
{
    qDebug() << "📝 PersistenceManager::saveTextItem() called for text:" << text << "at position:" << position;
    if (!m_schematicPersistence) {
        qWarning() << "❌ PersistenceManager::saveTextItem - SchematicPersistence not initialized!";
        qWarning() << "   Working directory:" << m_workingDirectory;
        return;
    }
    
    qDebug() << "💾 PersistenceManager::saveTextItem called for:" << text << "at" << position;
    m_schematicPersistence->saveTextItem(text, position, color, font);
}

void PersistenceManager::updateTextItem(const QString& oldText, const QPointF& oldPosition,
                                       const QString& newText, const QPointF& newPosition,
                                       const QColor& color, const QFont& font)
{
    if (!m_schematicPersistence) {
        qWarning() << "❌ PersistenceManager::updateTextItem - SchematicPersistence not initialized!";
        return;
    }
    
    m_schematicPersistence->updateTextItem(oldText, oldPosition, newText, newPosition, color, font);
}

void PersistenceManager::removeTextItem(const QString& text, const QPointF& position)
{
    if (!m_schematicPersistence) {
        qWarning() << "❌ PersistenceManager::removeTextItem - SchematicPersistence not initialized!";
        return;
    }
    
    qDebug() << "🗑️ PersistenceManager::removeTextItem called for:" << text << "at" << position;
    m_schematicPersistence->removeTextItem(text, position);
}

bool PersistenceManager::loadTextItems(QGraphicsScene* scene)
{
    if (!m_schematicPersistence) return false;
    return m_schematicPersistence->loadTextItems(scene);
}

void PersistenceManager::initializeSchematicFile()
{
    if (m_schematicPersistence) {
        m_schematicPersistence->initializeSchematicFile();
    }
}

QJsonObject PersistenceManager::loadSchematicJson()
{
    if (!m_schematicPersistence) return QJsonObject();
    return m_schematicPersistence->loadSchematicJson();
}

void PersistenceManager::saveSchematicJson(const QJsonObject& json)
{
    qDebug() << "💾 PersistenceManager::saveSchematicJson() called";
    if (m_schematicPersistence) {
        m_schematicPersistence->saveSchematicJson(json);
    }
}

// SystemC content generation (delegated to ComponentPersistence)
QString PersistenceManager::getSystemCContentFromModuleInfo(const ModuleInfo& moduleInfo, const QString& componentId)
{
    if (!m_componentPersistence) return QString();
    return m_componentPersistence->getSystemCContentFromModuleInfo(moduleInfo, componentId);
}

QString PersistenceManager::getSystemCPortType(const Port& port)
{
    if (!m_componentPersistence) return QString();
    return m_componentPersistence->getSystemCPortType(port);
}

// Accessors for persistence components
SchematicPersistence* PersistenceManager::getSchematicPersistence() const
{
    return m_schematicPersistence.get();
}

// Legacy methods for RTL/top.sv integration
void PersistenceManager::updateComponentRTLConnection(const QString& componentId, const QString& rtlFilePath)
{
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "No working directory set";
        return;
    }
    
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QString connectionsPath = QDir(metaDir).filePath("connections.json");
    if (!QFile::exists(connectionsPath)) {
        connectionsPath = QDir(m_workingDirectory).filePath("connections.json");
    }
    QFile file(connectionsPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    QJsonObject json = doc.object();
    QJsonArray components = json["components"].toArray();
    
    for (int i = 0; i < components.size(); ++i) {
        QJsonObject component = components[i].toObject();
        if (component["id"].toString() == componentId) {
            component["rtlConnection"] = rtlFilePath;
            components[i] = component;
            json["components"] = components;
            
            if (file.open(QIODevice::WriteOnly)) {
                file.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
                file.close();
                qDebug() << "Updated RTL connection for" << componentId;
            }
            return;
        }
    }
}

QString PersistenceManager::getComponentRTLConnection(const QString& componentId)
{
    if (m_workingDirectory.isEmpty()) {
        return QString();
    }
    
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QString connectionsPath = QDir(metaDir).filePath("connections.json");
    if (!QFile::exists(connectionsPath)) {
        connectionsPath = QDir(m_workingDirectory).filePath("connections.json");
    }
    QFile file(connectionsPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    QJsonArray components = doc.object()["components"].toArray();
    for (const QJsonValue& value : components) {
        QJsonObject component = value.toObject();
        if (component["id"].toString() == componentId) {
            return component["rtlConnection"].toString();
        }
    }
    
    return QString();
}

void PersistenceManager::generateTopSvIntegration(const QString& componentId, const QString& rtlFilePath)
{
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "No working directory set";
        return;
    }
    
    QString topSvPath = QDir(m_workingDirectory).filePath("top.sv");
    
    QFile topFile(topSvPath);
    if (!topFile.exists()) {
        qDebug() << "top.sv does not exist, skipping integration";
        return;
    }
    
    if (!topFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open top.sv for reading";
        return;
    }
    
    QTextStream in(&topFile);
    QString content = in.readAll();
    topFile.close();
    
    QFileInfo rtlFileInfo(rtlFilePath);
    QString rtlFileName = rtlFileInfo.fileName();
    QString moduleName = rtlFileInfo.baseName();
    
    QString instantiationPattern = QString("\\b%1\\s+u_%2\\s*\\(").arg(moduleName).arg(componentId);
    QRegularExpression regex(instantiationPattern);
    
    if (regex.match(content).hasMatch()) {
        qDebug() << "Module" << moduleName << "already instantiated in top.sv";
        return;
    }
    
    QString includePattern = QString("#include\\s+\"%1\"").arg(rtlFileName);
    QRegularExpression includeRegex(includePattern);
    
    if (!includeRegex.match(content).hasMatch()) {
        QRegularExpression includeSectionRegex(R"(^(\s*//.*\n)*\s*(module\s+top))", QRegularExpression::MultilineOption);
        QRegularExpressionMatch match = includeSectionRegex.match(content);
        
        if (match.hasMatch()) {
            QString includeLine = QString("#include \"%1\"\n\n").arg(rtlFileName);
            content.insert(match.capturedStart(2), includeLine);
        }
    }
    
    QRegularExpression endmoduleRegex(R"(\s*endmodule\s*$)");
    QRegularExpressionMatch endMatch = endmoduleRegex.match(content);
    
    if (endMatch.hasMatch()) {
        QString instantiation = QString("    %1 u_%2 (.*);\n").arg(moduleName).arg(componentId);
        content.insert(endMatch.capturedStart(), instantiation);
        
        if (topFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&topFile);
            out << content;
            topFile.close();
            qDebug() << "Updated top.sv with RTL module instantiation:" << moduleName;
        }
    }
}

