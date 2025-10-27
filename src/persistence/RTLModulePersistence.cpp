// RTLModulePersistence.cpp
#include "persistence/RTLModulePersistence.h"
#include "graphics/ModuleGraphicsItem.h"
#include "utils/PersistenceManager.h"
#include "parsers/SvParser.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QGraphicsScene>

RTLModulePersistence::RTLModulePersistence(const QString& workingDirectory)
    : m_workingDirectory(workingDirectory)
{
}

void RTLModulePersistence::setWorkingDirectory(const QString& directory)
{
    m_workingDirectory = directory;
}

QJsonObject RTLModulePersistence::loadRTLPlacementsJson()
{
    qDebug() << "📂 RTLModulePersistence::loadRTLPlacementsJson() called for directory:" << m_workingDirectory;
    if (m_workingDirectory.isEmpty()) {
        return QJsonObject();
    }
    
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QString placementsPath = QDir(metaDir).filePath("rtl_placements.json");
    if (!QFile::exists(placementsPath)) {
        // Fallback to legacy location
        placementsPath = QDir(m_workingDirectory).filePath("rtl_placements.json");
    }
    QFile file(placementsPath);
    
    if (!file.exists()) {
        return QJsonObject();
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QJsonObject();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    return doc.object();
}

void RTLModulePersistence::saveRTLPlacementsJson(const QJsonObject& json)
{
    qDebug() << "💾 RTLModulePersistence::saveRTLPlacementsJson() called for directory:" << m_workingDirectory;
    if (m_workingDirectory.isEmpty()) {
        return;
    }
    
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QDir dir(metaDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString placementsPath = dir.filePath("rtl_placements.json");
    QFile file(placementsPath);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to save rtl_placements.json";
        return;
    }
    
    QJsonDocument doc(json);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    qDebug() << "Saved rtl_placements.json";
}

QList<RTLModuleData> RTLModulePersistence::parseRTLPlacements(const QJsonObject& json)
{
    QList<RTLModuleData> result;
    QJsonArray placements = json["placements"].toArray();
    
    for (int i = 0; i < placements.size(); ++i) {
        QJsonObject placement = placements[i].toObject();
        
        RTLModuleData data;
        data.moduleName = placement["moduleName"].toString();
        data.filePath = placement["filePath"].toString();
        
        QJsonObject pos = placement["position"].toObject();
        data.position = QPointF(pos["x"].toDouble(), pos["y"].toDouble());
        
        result.append(data);
    }
    
    return result;
}

void RTLModulePersistence::saveRTLModulePlacement(const QString& moduleName, const QString& filePath, const QPointF& position)
{
    QJsonObject json = loadRTLPlacementsJson();
    QJsonArray placements = json["placements"].toArray();
    
    QJsonObject placement;
    placement["moduleName"] = moduleName;
    placement["filePath"] = filePath;
    placement["position"] = QJsonObject{{"x", position.x()}, {"y", position.y()}};
    
    placements.append(placement);
    json["placements"] = placements;
    
    saveRTLPlacementsJson(json);
}

void RTLModulePersistence::updateRTLModulePosition(const QString& moduleName, const QPointF& position)
{
    QJsonObject json = loadRTLPlacementsJson();
    QJsonArray placements = json["placements"].toArray();
    
    for (int i = 0; i < placements.size(); ++i) {
        QJsonObject placement = placements[i].toObject();
        if (placement["moduleName"].toString() == moduleName) {
            placement["position"] = QJsonObject{{"x", position.x()}, {"y", position.y()}};
            placements[i] = placement;
            json["placements"] = placements;
            saveRTLPlacementsJson(json);
            return;
        }
    }
}

void RTLModulePersistence::removeRTLModulePlacement(const QString& moduleName)
{
    QJsonObject json = loadRTLPlacementsJson();
    QJsonArray placements = json["placements"].toArray();
    QJsonArray newPlacements;
    
    for (int i = 0; i < placements.size(); ++i) {
        QJsonObject placement = placements[i].toObject();
        if (placement["moduleName"].toString() != moduleName) {
            newPlacements.append(placement);
        }
    }
    
    json["placements"] = newPlacements;
    saveRTLPlacementsJson(json);
}

QString RTLModulePersistence::getRTLModuleFilePath(const QString& moduleName)
{
    QJsonObject json = loadRTLPlacementsJson();
    QJsonArray placements = json["placements"].toArray();
    
    for (const QJsonValue& value : placements) {
        QJsonObject placement = value.toObject();
        if (placement["moduleName"].toString() == moduleName) {
            return placement["filePath"].toString();
        }
    }
    
    return QString();
}

bool RTLModulePersistence::loadRTLModules(QGraphicsScene* scene, PersistenceManager* pm)
{
    if (m_workingDirectory.isEmpty() || !scene || !pm) {
        return false;
    }
    
    QJsonObject json = loadRTLPlacementsJson();
    QList<RTLModuleData> placements = parseRTLPlacements(json);
    
    QStringList modulesToRemove; // Track modules that no longer exist
    
    for (const RTLModuleData& data : placements) {
        // Check if the file still exists
        QFile file(data.filePath);
        if (!file.exists()) {
            qWarning() << "⚠️ RTL file no longer exists, skipping:" << data.filePath;
            modulesToRemove.append(data.moduleName);
            continue;
        }
        
        // Parse the module
        ModuleInfo modInfo = SvParser::parseModule(data.filePath, data.moduleName);
        
        // Verify module was successfully parsed (has a valid name)
        if (modInfo.name.isEmpty()) {
            qWarning() << "⚠️ Failed to parse RTL module:" << data.moduleName << "from" << data.filePath;
            modulesToRemove.append(data.moduleName);
            continue;
        }
        
        // Create and place the module
        ModuleGraphicsItem* module = new ModuleGraphicsItem(modInfo);
        module->setPos(data.position);
        scene->addItem(module);
        
        // Register the module with PersistenceManager
        pm->setRTLModuleName(module, data.moduleName);
        
        qDebug() << "Loaded RTL module:" << data.moduleName << "at" << data.position;
    }
    
    // Clean up placements for modules that no longer exist
    for (const QString& moduleName : modulesToRemove) {
        qDebug() << "🧹 Removing stale RTL placement for:" << moduleName;
        removeRTLModulePlacement(moduleName);
    }
    
    return true;
}

