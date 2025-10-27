// PersistenceManager.cpp (Refactored)
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
    
    // Initialize persistence modules
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
    
    QString componentId = m_componentPersistence->createComponentFile(componentType, position, size);
    
    // Also save to connections.json
    if (!componentId.isEmpty() && m_connectionPersistence) {
        m_connectionPersistence->saveReadyComponentToConnections(componentId, componentType, position, size);
    }
    
    return componentId;
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
    if (m_componentPersistence) {
        m_componentPersistence->updateComponentPosition(componentId, position);
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

void PersistenceManager::deleteComponentFile(const QString& componentId)
{
    if (m_componentPersistence) {
        m_componentPersistence->deleteComponentFile(componentId);
    }
}

// RTL Module operations (delegated to RTLModulePersistence)
void PersistenceManager::saveRTLModulePlacement(const QString& moduleName, const QString& filePath, const QPointF& position)
{
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
    // Needed for legacy code
    if (!m_rtlModulePersistence) return QJsonObject();
    return m_rtlModulePersistence->loadRTLPlacementsJson();
}

// Connection operations (delegated to ConnectionPersistence)
void PersistenceManager::saveConnection(const QString& sourceId, const QPointF& sourcePort,
                                       const QString& targetId, const QPointF& targetPort,
                                       bool sourceIsRTL, bool targetIsRTL,
                                       const QList<QPointF>& controlPoints)
{
    if (m_connectionPersistence) {
        m_connectionPersistence->saveConnection(sourceId, sourcePort, targetId, targetPort,
                                               sourceIsRTL, targetIsRTL, controlPoints);
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

void PersistenceManager::saveReadyComponentToConnections(const QString& componentId, const QString& type,
                                                         const QPointF& position, const QSizeF& size)
{
    if (m_connectionPersistence) {
        m_connectionPersistence->saveReadyComponentToConnections(componentId, type, position, size);
    }
}

void PersistenceManager::updateRTLComponentInConnections(const QString& componentId, const QPointF& position)
{
    if (m_connectionPersistence) {
        m_connectionPersistence->updateRTLComponentInConnections(componentId, position);
    }
}

// Schematic and text item operations (delegated to SchematicPersistence)
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
    if (m_schematicPersistence) {
        m_schematicPersistence->saveSchematicJson(json);
    }
}

void PersistenceManager::saveTextItem(const QString& text, const QPointF& position,
                                     const QColor& color, const QFont& font)
{
    if (m_schematicPersistence) {
        m_schematicPersistence->saveTextItem(text, position, color, font);
    }
}

void PersistenceManager::updateTextItem(const QString& oldText, const QPointF& oldPosition,
                                       const QString& newText, const QPointF& newPosition,
                                       const QColor& color, const QFont& font)
{
    if (m_schematicPersistence) {
        m_schematicPersistence->updateTextItem(oldText, oldPosition, newText, newPosition, color, font);
    }
}

void PersistenceManager::removeTextItem(const QString& text, const QPointF& position)
{
    if (m_schematicPersistence) {
        m_schematicPersistence->removeTextItem(text, position);
    }
}

bool PersistenceManager::loadTextItems(QGraphicsScene* scene)
{
    if (!m_schematicPersistence) return false;
    return m_schematicPersistence->loadTextItems(scene);
}

// Legacy methods for RTL/top.sv integration
void PersistenceManager::updateComponentRTLConnection(const QString& componentId, const QString& rtlFilePath)
{
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "No working directory set";
        return;
    }
    
    // This method updates connections.json with RTL file associations
    // Implementation remains in main class as it's a cross-cutting concern
    QFile file(QDir(m_workingDirectory).filePath("connections.json"));
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
    
    QFile file(QDir(m_workingDirectory).filePath("connections.json"));
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
    
    // Check if top.sv exists
    QFile topFile(topSvPath);
    if (!topFile.exists()) {
        qDebug() << "top.sv does not exist, skipping integration";
        return;
    }
    
    // Read current content
    if (!topFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open top.sv for reading";
        return;
    }
    
    QTextStream in(&topFile);
    QString content = in.readAll();
    topFile.close();
    
    // Parse the RTL file to get module information
    QFileInfo rtlFileInfo(rtlFilePath);
    QString rtlFileName = rtlFileInfo.fileName();
    QString moduleName = rtlFileInfo.baseName();
    
    // Check if module is already instantiated
    QString instantiationPattern = QString("\\b%1\\s+u_%2\\s*\\(").arg(moduleName).arg(componentId);
    QRegularExpression regex(instantiationPattern);
    
    if (regex.match(content).hasMatch()) {
        qDebug() << "Module" << moduleName << "already instantiated in top.sv";
        return;
    }
    
    // Add include statement if not present
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
    
    // Add module instantiation before endmodule
    QRegularExpression endmoduleRegex(R"(\s*endmodule\s*$)");
    QRegularExpressionMatch endMatch = endmoduleRegex.match(content);
    
    if (endMatch.hasMatch()) {
        QString instantiation = QString("    %1 u_%2 (.*);\n").arg(moduleName).arg(componentId);
        content.insert(endMatch.capturedStart(), instantiation);
        
        // Write the updated content back
        if (topFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&topFile);
            out << content;
            topFile.close();
            
            qDebug() << "Updated top.sv with RTL module instantiation:" << moduleName;
        } else {
            qWarning() << "Failed to write updated top.sv";
        }
    }
}

// Delegate methods
QString PersistenceManager::getRTLModuleFilePath(const QString& moduleName)
{
    if (!m_rtlModulePersistence) return QString();
    return m_rtlModulePersistence->getRTLModuleFilePath(moduleName);
}

void PersistenceManager::saveTextItem(const QString& text, const QPointF& position, const QColor& color, const QFont& font)
{
    if (m_schematicPersistence) {
        m_schematicPersistence->saveTextItem(text, position, color, font);
    }
}

void PersistenceManager::updateTextItem(const QString& oldText, const QPointF& oldPosition,
                                       const QString& newText, const QPointF& newPosition,
                                       const QColor& color, const QFont& font)
{
    if (m_schematicPersistence) {
        m_schematicPersistence->updateTextItem(oldText, oldPosition, newText, newPosition, color, font);
    }
}

void PersistenceManager::removeTextItem(const QString& text, const QPointF& position)
{
    if (m_schematicPersistence) {
        m_schematicPersistence->removeTextItem(text, position);
    }
}

bool PersistenceManager::loadTextItems(QGraphicsScene* scene)
{
    if (!m_schematicPersistence) return false;
    return m_schematicPersistence->loadTextItems(scene);
}

bool PersistenceManager::loadRTLModules(QGraphicsScene* scene)
{
    if (!m_rtlModulePersistence) return false;
    return m_rtlModulePersistence->loadRTLModules(scene, this);
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

void PersistenceManager::saveConnection(const QString& sourceId, const QPointF& sourcePort,
                                       const QString& targetId, const QPointF& targetPort,
                                       bool sourceIsRTL, bool targetIsRTL,
                                       const QList<QPointF>& controlPoints)
{
    if (m_connectionPersistence) {
        m_connectionPersistence->saveConnection(sourceId, sourcePort, targetId, targetPort,
                                               sourceIsRTL, targetIsRTL, controlPoints);
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

void PersistenceManager::saveReadyComponentToConnections(const QString& componentId, const QString& type,
                                                         const QPointF& position, const QSizeF& size)
{
    if (m_connectionPersistence) {
        m_connectionPersistence->saveReadyComponentToConnections(componentId, type, position, size);
    }
}

void PersistenceManager::updateRTLComponentInConnections(const QString& componentId, const QPointF& position)
{
    if (m_connectionPersistence) {
        m_connectionPersistence->updateRTLComponentInConnections(componentId, position);
    }
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
    if (m_schematicPersistence) {
        m_schematicPersistence->saveSchematicJson(json);
    }
}

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

