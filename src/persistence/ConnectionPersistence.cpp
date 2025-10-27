// ConnectionPersistence.cpp
#include "persistence/ConnectionPersistence.h"
#include "graphics/ReadyComponentGraphicsItem.h"
#include "graphics/wire/WireGraphicsItem.h"
#include "utils/PersistenceManager.h"
#include "scene/SchematicScene.h"
#include "scene/WireManager.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QGraphicsScene>

ConnectionPersistence::ConnectionPersistence(const QString& workingDirectory)
    : m_workingDirectory(workingDirectory)
{
}

void ConnectionPersistence::setWorkingDirectory(const QString& directory)
{
    m_workingDirectory = directory;
}

QJsonObject ConnectionPersistence::loadConnectionsJson()
{
    qDebug() << "📂 ConnectionPersistence::loadConnectionsJson() called for directory:" << m_workingDirectory;
    if (m_workingDirectory.isEmpty()) {
        return QJsonObject();
    }
    
    // Load from centralized meta.json instead of separate connections.json
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QString metaFilePath = QDir(metaDir).filePath("meta.json");
    
    QFile file(metaFilePath);
    if (!file.exists()) {
        return QJsonObject();
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QJsonObject();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        return QJsonObject();
    }
    
    QJsonObject rootObj = doc.object();
    QJsonObject connectionsObj;
    connectionsObj["version"] = "1.0";
    connectionsObj["connections"] = rootObj["connections"].toArray();
    
    return connectionsObj;
}

void ConnectionPersistence::saveConnectionsJson(const QJsonObject& json)
{
    qDebug() << "💾 ConnectionPersistence::saveConnectionsJson() called for directory:" << m_workingDirectory;
    if (m_workingDirectory.isEmpty()) {
        return;
    }
    
    // Save to centralized meta.json instead of separate connections.json
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QDir dir(metaDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString metaFilePath = dir.filePath("meta.json");
    
    // Load existing meta.json or create new one
    QJsonObject rootObj;
    QFile metaFile(metaFilePath);
    if (metaFile.exists() && metaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(metaFile.readAll());
        metaFile.close();
        if (doc.isObject()) {
            rootObj = doc.object();
        }
    }
    
    // Update connections section
    rootObj["connections"] = json["connections"].toArray();
    
    // Save updated meta.json
    if (!metaFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to save meta.json";
        return;
    }
    
    QJsonDocument doc(rootObj);
    metaFile.write(doc.toJson(QJsonDocument::Indented));
    metaFile.close();
    
    qDebug() << "Saved connections to meta.json";
}

QList<ConnectionData> ConnectionPersistence::parseConnections(const QJsonObject& json)
{
    QList<ConnectionData> result;
    QJsonArray connections = json["connections"].toArray();
    
    for (int i = 0; i < connections.size(); ++i) {
        QJsonObject conn = connections[i].toObject();
        
        ConnectionData data;
        data.sourceId = conn["sourceId"].toString();
        data.targetId = conn["targetId"].toString();
        
        QJsonObject srcPort = conn["sourcePort"].toObject();
        QJsonObject tgtPort = conn["targetPort"].toObject();
        
        data.sourcePort = QPointF(srcPort["x"].toDouble(), srcPort["y"].toDouble());
        data.targetPort = QPointF(tgtPort["x"].toDouble(), tgtPort["y"].toDouble());
        data.sourceIsRTL = conn["sourceIsRTL"].toBool(false);
        data.targetIsRTL = conn["targetIsRTL"].toBool(false);
        
        // Parse control points
        QJsonArray controlPointsArray = conn["controlPoints"].toArray();
        for (int j = 0; j < controlPointsArray.size(); ++j) {
            QJsonObject cpObj = controlPointsArray[j].toObject();
            QPointF cp(cpObj["x"].toDouble(), cpObj["y"].toDouble());
            data.controlPoints.append(cp);
        }
        
        // Parse orthogonal offset
        data.orthogonalOffset = conn["orthogonalOffset"].toDouble(0.0);
        
        result.append(data);
    }
    
    return result;
}

void ConnectionPersistence::saveConnection(const QString& sourceId, const QPointF& sourcePort,
                                          const QString& targetId, const QPointF& targetPort,
                                          bool sourceIsRTL, bool targetIsRTL,
                                          const QList<QPointF>& controlPoints, qreal orthogonalOffset)
{
    QJsonObject json = loadConnectionsJson();
    QJsonArray connections = json["connections"].toArray();
    
    QJsonObject connection;
    connection["sourceId"] = sourceId;
    connection["sourcePort"] = QJsonObject{{"x", sourcePort.x()}, {"y", sourcePort.y()}};
    connection["targetId"] = targetId;
    connection["targetPort"] = QJsonObject{{"x", targetPort.x()}, {"y", targetPort.y()}};
    connection["sourceIsRTL"] = sourceIsRTL;
    connection["targetIsRTL"] = targetIsRTL;
    
    // Save control points
    QJsonArray controlPointsArray;
    for (const QPointF& point : controlPoints) {
        controlPointsArray.append(QJsonObject{{"x", point.x()}, {"y", point.y()}});
    }
    connection["controlPoints"] = controlPointsArray;
    
    // Save orthogonal offset
    connection["orthogonalOffset"] = orthogonalOffset;
    
    connections.append(connection);
    json["connections"] = connections;
    
    saveConnectionsJson(json);
    
    qDebug() << "💾 Saved connection with orthogonal offset:" << orthogonalOffset;
}

void ConnectionPersistence::removeConnection(const QString& sourceId, const QPointF& sourcePort,
                                            const QString& targetId, const QPointF& targetPort)
{
    QJsonObject json = loadConnectionsJson();
    QJsonArray connections = json["connections"].toArray();
    QJsonArray newConnections;
    
    for (int i = 0; i < connections.size(); ++i) {
        QJsonObject conn = connections[i].toObject();
        QString src = conn["sourceId"].toString();
        QString tgt = conn["targetId"].toString();
        QJsonObject srcPort = conn["sourcePort"].toObject();
        QJsonObject tgtPort = conn["targetPort"].toObject();
        
        QPointF srcPos(srcPort["x"].toDouble(), srcPort["y"].toDouble());
        QPointF tgtPos(tgtPort["x"].toDouble(), tgtPort["y"].toDouble());
        
        // Keep connection if it doesn't match
        if (src != sourceId || tgt != targetId ||
            qAbs(srcPos.x() - sourcePort.x()) > 1 || qAbs(srcPos.y() - sourcePort.y()) > 1 ||
            qAbs(tgtPos.x() - targetPort.x()) > 1 || qAbs(tgtPos.y() - targetPort.y()) > 1) {
            newConnections.append(conn);
        }
    }
    
    json["connections"] = newConnections;
    saveConnectionsJson(json);
}

void ConnectionPersistence::updateConnectionControlPoints(const QString& sourceId, const QPointF& sourcePort,
                                                          const QString& targetId, const QPointF& targetPort,
                                                          const QList<QPointF>& controlPoints)
{
    QJsonObject json = loadConnectionsJson();
    QJsonArray connections = json["connections"].toArray();
    
    for (int i = 0; i < connections.size(); ++i) {
        QJsonObject conn = connections[i].toObject();
        QString src = conn["sourceId"].toString();
        QString tgt = conn["targetId"].toString();
        
        if (src == sourceId && tgt == targetId) {
            QJsonObject srcPort = conn["sourcePort"].toObject();
            QJsonObject tgtPort = conn["targetPort"].toObject();
            
            QPointF srcPos(srcPort["x"].toDouble(), srcPort["y"].toDouble());
            QPointF tgtPos(tgtPort["x"].toDouble(), tgtPort["y"].toDouble());
            
            if (qAbs(srcPos.x() - sourcePort.x()) < 1 && qAbs(srcPos.y() - sourcePort.y()) < 1 &&
                qAbs(tgtPos.x() - targetPort.x()) < 1 && qAbs(tgtPos.y() - targetPort.y()) < 1) {
                
                // Update control points
                QJsonArray controlPointsArray;
                for (const QPointF& point : controlPoints) {
                    controlPointsArray.append(QJsonObject{{"x", point.x()}, {"y", point.y()}});
                }
                conn["controlPoints"] = controlPointsArray;
                connections[i] = conn;
                
                json["connections"] = connections;
                saveConnectionsJson(json);
                return;
            }
        }
    }
}

void ConnectionPersistence::updateConnectionOrthogonalOffset(const QString& sourceId, const QPointF& sourcePort,
                                                             const QString& targetId, const QPointF& targetPort,
                                                             qreal orthogonalOffset)
{
    QJsonObject json = loadConnectionsJson();
    QJsonArray connections = json["connections"].toArray();
    
    for (int i = 0; i < connections.size(); ++i) {
        QJsonObject conn = connections[i].toObject();
        QString src = conn["sourceId"].toString();
        QString tgt = conn["targetId"].toString();
        
        if (src == sourceId && tgt == targetId) {
            QJsonObject srcPort = conn["sourcePort"].toObject();
            QJsonObject tgtPort = conn["targetPort"].toObject();
            
            QPointF srcPos(srcPort["x"].toDouble(), srcPort["y"].toDouble());
            QPointF tgtPos(tgtPort["x"].toDouble(), tgtPort["y"].toDouble());
            
            if (qAbs(srcPos.x() - sourcePort.x()) < 1 && qAbs(srcPos.y() - sourcePort.y()) < 1 &&
                qAbs(tgtPos.x() - targetPort.x()) < 1 && qAbs(tgtPos.y() - targetPort.y()) < 1) {
                
                // Update orthogonal offset
                conn["orthogonalOffset"] = orthogonalOffset;
                connections[i] = conn;
                
                json["connections"] = connections;
                saveConnectionsJson(json);
                return;
            }
        }
    }
}


void ConnectionPersistence::updateRTLComponentInConnections(const QString& componentId, const QPointF& position)
{
    if (m_workingDirectory.isEmpty()) {
        return;
    }
    
    QJsonObject json = loadConnectionsJson();
    QJsonArray components = json["components"].toArray();
    
    // Find and update the RTL component
    for (int i = 0; i < components.size(); ++i) {
        QJsonObject comp = components[i].toObject();
        if (comp["id"].toString() == componentId) {
            comp["position"] = QJsonObject{{"x", position.x()}, {"y", position.y()}};
            components[i] = comp;
            json["components"] = components;
            saveConnectionsJson(json);
            qDebug() << "Updated RTL component position for" << componentId;
            return;
        }
    }
}

void ConnectionPersistence::removeComponentFromConnections(const QString& componentId)
{
    if (m_workingDirectory.isEmpty()) {
        return;
    }
    
    QJsonObject json = loadConnectionsJson();
    QJsonArray components = json["components"].toArray();
    QJsonArray newComponents;
    
    // Filter out the component to delete
    for (int i = 0; i < components.size(); ++i) {
        QJsonObject comp = components[i].toObject();
        if (comp["id"].toString() != componentId) {
            newComponents.append(comp);
        }
    }
    
    // Also remove any connections involving this component
    QJsonArray connections = json["connections"].toArray();
    QJsonArray newConnections;
    
    for (int i = 0; i < connections.size(); ++i) {
        QJsonObject conn = connections[i].toObject();
        QString sourceId = conn["sourceId"].toString();
        QString targetId = conn["targetId"].toString();
        
        // Keep connection only if it doesn't involve the deleted component
        if (sourceId != componentId && targetId != componentId) {
            newConnections.append(conn);
        }
    }
    
    json["components"] = newComponents;
    json["connections"] = newConnections;
    saveConnectionsJson(json);
    
    qDebug() << "Removed component from connections.json:" << componentId;
}

void ConnectionPersistence::removeComponentOnlyFromConnections(const QString& componentId)
{
    if (m_workingDirectory.isEmpty()) {
        return;
    }
    
    QJsonObject json = loadConnectionsJson();
    QJsonArray components = json["components"].toArray();
    QJsonArray newComponents;
    
    // Filter out the component to delete from components list only
    for (int i = 0; i < components.size(); ++i) {
        QJsonObject comp = components[i].toObject();
        if (comp["id"].toString() != componentId) {
            newComponents.append(comp);
        }
    }
    
    // Keep all connections intact - do NOT remove connections involving this component
    json["components"] = newComponents;
    // connections array remains unchanged
    
    saveConnectionsJson(json);
    
    qDebug() << "Removed component from components list (connections preserved):" << componentId;
}

bool ConnectionPersistence::loadConnections(QGraphicsScene* scene, PersistenceManager* pm)
{
    if (m_workingDirectory.isEmpty() || !scene || !pm) {
        qWarning() << "⚠️ ConnectionPersistence::loadConnections - Invalid parameters";
        return false;
    }
    
    QJsonObject json = loadConnectionsJson();
    QList<ConnectionData> connections = parseConnections(json);
    
    qDebug() << "🔗 Loading" << connections.size() << "connections from persistence...";
    
    int restoredCount = 0;
    int failedCount = 0;
    
    for (const ConnectionData& conn : connections) {
        ReadyComponentGraphicsItem* source = nullptr;
        ReadyComponentGraphicsItem* target = nullptr;
        
        // Get source (either ready component or RTL module)
        if (conn.sourceIsRTL) {
            source = pm->getRTLModuleByName(conn.sourceId);
            qDebug() << "🔍 Looking for RTL source component:" << conn.sourceId << (source ? "✅ Found" : "❌ Not found");
        } else {
            source = pm->getComponentById(conn.sourceId);
            qDebug() << "🔍 Looking for ready source component:" << conn.sourceId << (source ? "✅ Found" : "❌ Not found");
        }
        
        // Get target (either ready component or RTL module)
        if (conn.targetIsRTL) {
            target = pm->getRTLModuleByName(conn.targetId);
            qDebug() << "🔍 Looking for RTL target component:" << conn.targetId << (target ? "✅ Found" : "❌ Not found");
        } else {
            target = pm->getComponentById(conn.targetId);
            qDebug() << "🔍 Looking for ready target component:" << conn.targetId << (target ? "✅ Found" : "❌ Not found");
        }
        
        if (source && target) {
            WireGraphicsItem* wire = new WireGraphicsItem(source, conn.sourcePort, target, conn.targetPort);
            
            // Restore control points
            if (!conn.controlPoints.isEmpty()) {
                wire->setControlPoints(conn.controlPoints);
            }
            
            // Restore orthogonal offset
            if (conn.orthogonalOffset != 0.0) {
                wire->setOrthogonalOffset(conn.orthogonalOffset);
            }
            
            scene->addItem(wire);
            source->addWire(wire);
            target->addWire(wire);
            
            // Register wire with WireManager for proper routing and management
            // This is critical for wire visibility and functionality
            SchematicScene* schematicScene = qobject_cast<SchematicScene*>(scene);
            if (schematicScene && schematicScene->getWireManager()) {
                schematicScene->getWireManager()->registerWire(wire);
                qDebug() << "🔗 Registered restored wire with WireManager:" << conn.sourceId << "->" << conn.targetId;
            } else {
                qWarning() << "⚠️ Could not register wire with WireManager - SchematicScene or WireManager not available";
            }
            
            qDebug() << "Restored connection:" << conn.sourceId << "->" << conn.targetId;
            restoredCount++;
        } else {
            qWarning() << "⚠️ Failed to restore connection - missing source or target component:"
                      << conn.sourceId << "->" << conn.targetId;
            failedCount++;
        }
    }
    
    qDebug() << "🔗 Connection loading completed - Restored:" << restoredCount << "Failed:" << failedCount;
    
    return true;
}


