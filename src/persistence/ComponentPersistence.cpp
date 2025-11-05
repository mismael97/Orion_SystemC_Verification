// ComponentPersistence.cpp
#include "persistence/ComponentPersistence.h"
#include "graphics/ReadyComponentGraphicsItem.h"
#include "utils/PersistenceManager.h"
#include "parsers/SvParser.h"
#include "scene/SchematicScene.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTextStream>
#include <QDebug>
#include <QGraphicsScene>
#include <QRegularExpression>
#include <QColor>
#include <QObject>
#include <QFileInfo>

ComponentPersistence::ComponentPersistence(const QString& workingDirectory)
    : QObject()
    , m_workingDirectory(workingDirectory)
    , m_componentCounter(0)
    , m_batchUpdateTimer(std::make_unique<QTimer>())
{
    // Setup batch update timer for performance optimization
    m_batchUpdateTimer->setSingleShot(true);
    m_batchUpdateTimer->setInterval(100); // 100ms batch window
    connect(m_batchUpdateTimer.get(), &QTimer::timeout, this, &ComponentPersistence::performBatchMetadataUpdate);
}

void ComponentPersistence::setWorkingDirectory(const QString& directory)
{
    m_workingDirectory = directory;
    m_componentCounter = 0;
    
    // Clear metadata cache when working directory changes
    clearMetadataCache();
    
    // Restore component counter from existing files to avoid ID collisions
    restoreComponentCounter();
}

QString ComponentPersistence::generateComponentId(const QString& componentType)
{
    return QString("%1_%2").arg(componentType).arg(++m_componentCounter);
}

QString ComponentPersistence::getSystemCContent(const QString& componentType, const QString& componentId)
{
    QString content;
    QTextStream stream(&content);
    
    stream << "// SystemC file for " << componentType << "\n";
    stream << "// Component ID: " << componentId << "\n\n";
    stream << "#include <systemc.h>\n\n";
    stream << "SC_MODULE(" << componentId << ") {\n";
    stream << "    // Ports\n";
    
    // Add ports based on component type
    if (componentType == "Transactor") {
        stream << "    sc_in<bool> clk;\n";
        stream << "    sc_in<bool> reset;\n";
        stream << "    sc_out<sc_uint<32>> data_out1;\n";
        stream << "    sc_out<sc_uint<32>> data_out2;\n";
        stream << "    sc_out<sc_uint<32>> data_out3;\n";
    } else if (componentType == "RM") {
        stream << "    sc_in<sc_uint<32>> data_in;\n";
        stream << "    sc_out<sc_uint<32>> data_out;\n";
    } else if (componentType == "Compare") {
        stream << "    sc_in<sc_uint<32>> data_in1;\n";
        stream << "    sc_in<sc_uint<32>> data_in2;\n";
    } else if (componentType == "Driver") {
        stream << "    sc_in<sc_uint<32>> data_in;\n";
        stream << "    sc_out<bool> valid;\n";
        stream << "    sc_out<sc_uint<32>> data_out;\n";
    } else if (componentType == "Stimuler") {
        stream << "    sc_in<bool> clk;\n";
        stream << "    sc_out<sc_uint<32>> data_out;\n";
    } else if (componentType == "Stimuli") {
        stream << "    sc_out<sc_uint<32>> data_out;\n";
    } else if (componentType == "RTL") {
        stream << "    // RTL Component - SystemC wrapper for RTL modules\n";
        stream << "    sc_in<sc_uint<32>> data_in;   // Input port\n";
        stream << "    sc_out<sc_uint<32>> data_out;  // Output port\n";
    }
    
    stream << "\n    SC_CTOR(" << componentId << ") {\n";
    stream << "        // Constructor\n";
    stream << "    }\n";
    stream << "\n    void process() {\n";
    stream << "        // Process logic\n";
    stream << "    }\n";
    stream << "};\n";
    
    return content;
}

QString ComponentPersistence::getSystemCContentFromModuleInfo(const ModuleInfo& moduleInfo, const QString& componentId)
{
    QString content;
    QTextStream stream(&content);
    
    stream << "// SystemC file for RTL module: " << moduleInfo.name << "\n";
    stream << "// Component ID: " << componentId << "\n";
    stream << "// Auto-generated from SystemVerilog module\n\n";
    stream << "#include <systemc.h>\n\n";
    stream << "SC_MODULE(" << componentId << ") {\n";
    stream << "    // Ports from RTL module\n";
    
    // Generate input ports
    if (!moduleInfo.inputs.isEmpty()) {
        stream << "\n    // Input ports\n";
        for (const Port& port : moduleInfo.inputs) {
            QString portType = getSystemCPortType(port);
            stream << "    sc_in<" << portType << "> " << port.name << ";";
            if (!port.width.isEmpty()) {
                stream << "  // " << port.width;
            }
            stream << "\n";
        }
    }
    
    // Generate output ports
    if (!moduleInfo.outputs.isEmpty()) {
        stream << "\n    // Output ports\n";
        for (const Port& port : moduleInfo.outputs) {
            QString portType = getSystemCPortType(port);
            stream << "    sc_out<" << portType << "> " << port.name << ";";
            if (!port.width.isEmpty()) {
                stream << "  // " << port.width;
            }
            stream << "\n";
        }
    }
    
    stream << "\n    SC_CTOR(" << componentId << ") {\n";
    stream << "        // Constructor\n";
    stream << "        // Initialize RTL module wrapper\n";
    stream << "    }\n";
    stream << "\n    void process() {\n";
    stream << "        // Process logic for " << moduleInfo.name << "\n";
    stream << "        // Implement RTL behavior here\n";
    stream << "    }\n";
    stream << "};\n";
    
    return content;
}

QString ComponentPersistence::getSystemCPortType(const Port& port)
{
    // Parse width from format "[MSB:LSB]"
    if (port.width.isEmpty()) {
        return "bool";  // Single bit
    }
    
    // Extract MSB from width string like "[7:0]"
    QRegularExpression re(R"(\[(\d+):(\d+)\])");
    QRegularExpressionMatch match = re.match(port.width);
    
    if (match.hasMatch()) {
        int msb = match.captured(1).toInt();
        int lsb = match.captured(2).toInt();
        int width = msb - lsb + 1;
        
        if (width == 1) {
            return "bool";
        } else if (width <= 64) {
            return "sc_uint<" + QString::number(width) + ">";
        } else {
            return "sc_biguint<" + QString::number(width) + ">";
        }
    }
    
    // Default fallback
    return "sc_uint<32>";
}

QString ComponentPersistence::createComponentFile(const QString& componentType, const QPointF& position, const QSizeF& size)
{
    qDebug() << "📄 ComponentPersistence::createComponentFile() called for type:" << componentType << "at position:" << position;
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "No working directory set";
        return QString();
    }
    
    QString componentId = generateComponentId(componentType);
    QString fileName = componentId + ".cpp";
    QString filePath = QDir(m_workingDirectory).filePath(fileName);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to create component file:" << filePath;
        return QString();
    }
    
    QTextStream out(&file);
    out << getSystemCContent(componentType, componentId);
    file.close();
    
    // Create enhanced metadata
    QJsonObject enhancedMetadata = createEnhancedMetadata(componentId, componentType, position, size);
    
    // Cache the metadata (no individual file creation)
    updateCachedMetadata(componentId, enhancedMetadata);
    
    qDebug() << "Created component file:" << filePath << "with enhanced metadata";
    return componentId;
}

QString ComponentPersistence::createRTLModuleFile(const ModuleInfo& moduleInfo, const QString& filePath,
                                                  const QPointF& position, const QSizeF& size)
{
    qDebug() << "📄 ComponentPersistence::createRTLModuleFile() called for module:" << moduleInfo.name << "at file:" << filePath;
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "No working directory set";
        return QString();
    }
    
    QString componentId = generateComponentId(moduleInfo.name);
    QString fileName = componentId + ".cpp";
    QString cppFilePath = QDir(m_workingDirectory).filePath(fileName);
    
    QFile file(cppFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to create RTL module file:" << cppFilePath;
        return QString();
    }
    
    QTextStream out(&file);
    out << getSystemCContentFromModuleInfo(moduleInfo, componentId);
    file.close();
    
    // Create enhanced metadata for RTL module
    QJsonObject enhancedMetadata = createEnhancedMetadata(componentId, "RTLModule", position, size);
    
    // Add RTL-specific information
    enhancedMetadata["moduleName"] = moduleInfo.name;
    enhancedMetadata["sourceFilePath"] = filePath;
    
    // Store detailed port information in the ports section
    QJsonObject portsConfig;
    
    // Input ports
    QJsonArray inputPorts;
    for (const Port& port : moduleInfo.inputs) {
        QJsonObject portObj;
        portObj["name"] = port.name;
        portObj["type"] = getSystemCPortType(port);
        portObj["width"] = port.width;
        portObj["connected"] = false;
        portObj["customName"] = port.name;
        // Position will be calculated by the component when restored
        inputPorts.append(portObj);
    }
    portsConfig["inputs"] = inputPorts;
    
    // Output ports
    QJsonArray outputPorts;
    for (const Port& port : moduleInfo.outputs) {
        QJsonObject portObj;
        portObj["name"] = port.name;
        portObj["type"] = getSystemCPortType(port);
        portObj["width"] = port.width;
        portObj["connected"] = false;
        portObj["customName"] = port.name;
        // Position will be calculated by the component when restored
        outputPorts.append(portObj);
    }
    portsConfig["outputs"] = outputPorts;
    
    enhancedMetadata["ports"] = portsConfig;
    
    // Cache the metadata (no individual file creation)
    updateCachedMetadata(componentId, enhancedMetadata);
    
    qDebug() << "Created RTL module file:" << cppFilePath << "for module:" << moduleInfo.name;
    return componentId;
}

bool ComponentPersistence::loadComponentsFromDirectory(QGraphicsScene* scene, PersistenceManager* pm)
{
    qDebug() << "📂 ComponentPersistence::loadComponentsFromDirectory() called for directory:" << m_workingDirectory;
    if (m_workingDirectory.isEmpty() || !scene || !pm) {
        return false;
    }
    
    // Load from centralized meta.json instead of individual files
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QString metaFilePath = QDir(metaDir).filePath("meta.json");
    
    QFile file(metaFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "No meta.json found, will create on first save";
        return true; // Not an error, just no existing data
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        qWarning() << "Invalid meta.json format";
        return false;
    }
    
    QJsonObject rootObj = doc.object();
    QJsonObject components = rootObj["components"].toObject();
    
    for (auto it = components.begin(); it != components.end(); ++it) {
        QString componentId = it.key();
        QJsonValue value = it.value();
        
        if (!value.isObject()) continue;
        
        QJsonObject metadata = value.toObject();
        QString id = metadata["id"].toString();
        QString type = metadata["type"].toString();
        
        // Verify that the corresponding .cpp file exists
        QString cppFile = id + ".cpp";
        QString cppFilePath = QDir(m_workingDirectory).filePath(cppFile);
        if (!QFile::exists(cppFilePath)) {
            qWarning() << "⚠️ Skipping component - .cpp file missing:" << cppFile;
            continue;
        }

        // Extract geometry information
        QJsonObject geometry = metadata["geometry"].toObject();
        QJsonObject positionObj = geometry["position"].toObject();
        QJsonObject sizeObj = geometry["size"].toObject();
        
        QPointF position(positionObj["x"].toDouble(), positionObj["y"].toDouble());
        QSizeF size(sizeObj["width"].toDouble(), sizeObj["height"].toDouble());
        qreal rotation = geometry["rotation"].toDouble(0.0);

        // Normalize type name for backward compatibility
        QString normalizedType = type;
        if (type == "RTLComponent") {
            normalizedType = "RTL";
        }
        
        // Create the component
        ReadyComponentGraphicsItem* component = new ReadyComponentGraphicsItem(normalizedType);
        component->setPos(position);
        component->setSize(size.width(), size.height());
        component->setRotation(rotation);
        
        // Restore appearance properties
        if (metadata.contains("appearance")) {
            QJsonObject appearance = metadata["appearance"].toObject();
            
            QString colorStr = appearance["color"].toString();
            if (!colorStr.isEmpty() && QColor::isValidColorName(colorStr)) {
                component->setCustomColor(QColor(colorStr));
            }
            
            if (appearance.contains("opacity")) {
                component->setOpacity(appearance["opacity"].toDouble(1.0));
            }
            
            if (appearance.contains("visible")) {
                component->setVisible(appearance["visible"].toBool(true));
            }
        }
        
        // Restore port configurations
        if (metadata.contains("ports")) {
            restorePortConfigurations(component, metadata["ports"].toObject());
        }
        
        // Restore component properties
        if (metadata.contains("properties")) {
            restoreComponentProperties(component, metadata["properties"].toObject());
        }
        
        // Restore connected file path if present
        if (metadata.contains("connectedFilePath")) {
            QString connectedFilePath = metadata["connectedFilePath"].toString();
            if (!connectedFilePath.isEmpty()) {
                component->setConnectedFilePath(connectedFilePath);
                qDebug() << "🔗 Restored connected file path for component:" << id << "| Path:" << connectedFilePath;
            }
        }
        
        scene->addItem(component);
        
        // Register with PersistenceManager
        pm->setComponentId(component, id);
        
        // Cache the metadata to ensure it's preserved when saving
        updateCachedMetadata(id, metadata);
        
        // Update counter to avoid ID collisions
        QString numberStr = id.mid(id.lastIndexOf('_') + 1);
        int number = numberStr.toInt();
        if (number > m_componentCounter) {
            m_componentCounter = number;
        }
        
        qDebug() << "📦 Loaded and cached component:" << type << "at" << position << "with enhanced metadata";
    }
    
    return true;
}

void ComponentPersistence::updateComponentPosition(const QString& componentId, const QPointF& position)
{
    if (m_workingDirectory.isEmpty()) {
        return;
    }
    
    // Get existing metadata
    QJsonObject metadata = getCachedMetadata(componentId);
    if (metadata.isEmpty()) {
        qWarning() << "⚠️ Component metadata not found for position update:" << componentId;
        return;
    }
    
    // Ensure metadata has required structure
    if (!metadata.contains("geometry")) {
        qWarning() << "⚠️ Component metadata missing geometry section:" << componentId;
        return;
    }
    
    // Update position in geometry section
    QJsonObject geometry = metadata["geometry"].toObject();
    QJsonObject positionObj;
    positionObj["x"] = position.x();
    positionObj["y"] = position.y();
    
    // Safe access to original position values
    QJsonObject existingPosition = geometry["position"].toObject();
    positionObj["originalX"] = existingPosition["originalX"].toDouble(position.x());
    positionObj["originalY"] = existingPosition["originalY"].toDouble(position.y());
    positionObj["snapToGrid"] = true;
    geometry["position"] = positionObj;
    metadata["geometry"] = geometry;
    
    // Update timestamps
    QDateTime currentTime = QDateTime::currentDateTime();
    metadata["modified"] = currentTime.toString(Qt::ISODate);
    metadata["modifiedTimestamp"] = currentTime.toMSecsSinceEpoch();
    
    // Update statistics (with safety check)
    QJsonObject statistics = metadata["statistics"].toObject();
    statistics["moveCount"] = statistics["moveCount"].toInt(0) + 1;
    statistics["lastUsed"] = currentTime.toString(Qt::ISODate);
    metadata["statistics"] = statistics;
    
    // Add to modification history (with safety check)
    QJsonObject history = metadata["history"].toObject();
    QJsonArray modificationHistory = history["modificationHistory"].toArray();
    QJsonObject modificationEvent;
    modificationEvent["action"] = "position-update";
    modificationEvent["timestamp"] = currentTime.toString(Qt::ISODate);
    modificationEvent["user"] = "system";
    modificationEvent["details"] = QString("Component moved to position (%1, %2)").arg(position.x()).arg(position.y());
    modificationHistory.append(modificationEvent);
    history["modificationHistory"] = modificationHistory;
    metadata["history"] = history;
    
    // Update cached metadata and save to meta.json
    updateCachedMetadata(componentId, metadata);
    
    qDebug() << "📍 Component position updated:" << componentId << "to" << position;
    qDebug() << "💾 Position change saved to meta.json geometry section";
    qDebug() << "📋 Meta.json updates: position coordinates, timestamps, move count, history";
    
    // Verify the position was saved by checking the cached metadata
    QJsonObject savedMetadata = getCachedMetadata(componentId);
    if (!savedMetadata.isEmpty() && savedMetadata.contains("geometry")) {
        QJsonObject geometry = savedMetadata["geometry"].toObject();
        QJsonObject savedPosition = geometry["position"].toObject();
        qDebug() << "✅ Verified component movement reflected in meta.json:" << savedPosition["x"].toDouble() << savedPosition["y"].toDouble();
        qDebug() << "📄 Component" << componentId << "movement successfully saved to meta.json geometry section";
    } else {
        qWarning() << "⚠️ Failed to verify component movement in meta.json for:" << componentId;
    }
}

void ComponentPersistence::updateComponentSize(const QString& componentId, const QSizeF& size)
{
    if (m_workingDirectory.isEmpty()) {
        return;
    }
    
    // Get existing metadata
    QJsonObject metadata = getCachedMetadata(componentId);
    if (metadata.isEmpty()) {
        qWarning() << "⚠️ Component metadata not found for size update:" << componentId;
        return;
    }
    
    // Ensure metadata has required structure
    if (!metadata.contains("geometry")) {
        qWarning() << "⚠️ Component metadata missing geometry section:" << componentId;
        return;
    }
    
    // Update size in geometry section
    QJsonObject geometry = metadata["geometry"].toObject();
    QJsonObject sizeObj;
    sizeObj["width"] = size.width();
    sizeObj["height"] = size.height();
    
    // Safe access to original size values
    QJsonObject existingSize = geometry["size"].toObject();
    sizeObj["originalWidth"] = existingSize["originalWidth"].toDouble(size.width());
    sizeObj["originalHeight"] = existingSize["originalHeight"].toDouble(size.height());
    sizeObj["minWidth"] = 50.0;
    sizeObj["minHeight"] = 50.0;
    sizeObj["maxWidth"] = 1000.0;
    sizeObj["maxHeight"] = 1000.0;
    sizeObj["aspectRatio"] = size.width() / size.height();
    geometry["size"] = sizeObj;
    metadata["geometry"] = geometry;
    
    // Update timestamps
    QDateTime currentTime = QDateTime::currentDateTime();
    metadata["modified"] = currentTime.toString(Qt::ISODate);
    metadata["modifiedTimestamp"] = currentTime.toMSecsSinceEpoch();
    
    // Update statistics
    QJsonObject statistics = metadata["statistics"].toObject();
    statistics["resizeCount"] = statistics["resizeCount"].toInt() + 1;
    statistics["lastUsed"] = currentTime.toString(Qt::ISODate);
    metadata["statistics"] = statistics;
    
    // Add to modification history
    QJsonObject history = metadata["history"].toObject();
    QJsonArray modificationHistory = history["modificationHistory"].toArray();
    QJsonObject modificationEvent;
    modificationEvent["action"] = "size-update";
    modificationEvent["timestamp"] = currentTime.toString(Qt::ISODate);
    modificationEvent["user"] = "system";
    modificationEvent["details"] = QString("Component resized to %1x%2").arg(size.width()).arg(size.height());
    modificationHistory.append(modificationEvent);
    history["modificationHistory"] = modificationHistory;
    metadata["history"] = history;
    
    // Update cached metadata and save to meta.json
    updateCachedMetadata(componentId, metadata);
    
    qDebug() << "📏 Component size updated:" << componentId << "to" << size;
}

void ComponentPersistence::updateComponentRotation(const QString& componentId, qreal rotation)
{
    if (m_workingDirectory.isEmpty()) {
        return;
    }
    
    // Get existing metadata
    QJsonObject metadata = getCachedMetadata(componentId);
    if (metadata.isEmpty()) {
        qWarning() << "⚠️ Component metadata not found for rotation update:" << componentId;
        return;
    }
    
    // Ensure metadata has required structure
    if (!metadata.contains("geometry")) {
        qWarning() << "⚠️ Component metadata missing geometry section:" << componentId;
        return;
    }
    
    // Update rotation in geometry section
    QJsonObject geometry = metadata["geometry"].toObject();
    geometry["rotation"] = rotation;
    geometry["originalRotation"] = geometry["originalRotation"].toDouble(0.0);
    metadata["geometry"] = geometry;
    
    // Update timestamps
    QDateTime currentTime = QDateTime::currentDateTime();
    metadata["modified"] = currentTime.toString(Qt::ISODate);
    metadata["modifiedTimestamp"] = currentTime.toMSecsSinceEpoch();
    
    // Update statistics
    QJsonObject statistics = metadata["statistics"].toObject();
    statistics["rotateCount"] = statistics["rotateCount"].toInt() + 1;
    statistics["lastUsed"] = currentTime.toString(Qt::ISODate);
    metadata["statistics"] = statistics;
    
    // Add to modification history
    QJsonObject history = metadata["history"].toObject();
    QJsonArray modificationHistory = history["modificationHistory"].toArray();
    QJsonObject modificationEvent;
    modificationEvent["action"] = "rotation-update";
    modificationEvent["timestamp"] = currentTime.toString(Qt::ISODate);
    modificationEvent["user"] = "system";
    modificationEvent["details"] = QString("Component rotated to %1 degrees").arg(rotation);
    modificationHistory.append(modificationEvent);
    history["modificationHistory"] = modificationHistory;
    metadata["history"] = history;
    
    // Update cached metadata and save to meta.json
    updateCachedMetadata(componentId, metadata);
    
    qDebug() << "🔄 Component rotation updated:" << componentId << "to" << rotation << "degrees";
}

void ComponentPersistence::updateComponentColor(const QString& componentId, const QColor& color)
{
    if (m_workingDirectory.isEmpty()) {
        return;
    }
    
    QString metaFileName = componentId + ".meta.json";
    QString metaFilePath = QDir(m_workingDirectory).filePath(metaFileName);
    
    QFile file(metaFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        return;
    }
    
    QJsonObject obj = doc.object();
    obj["color"] = color.name(QColor::HexArgb);  // Save with alpha
    
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonDocument newDoc(obj);
        file.write(newDoc.toJson());
        file.close();
        qDebug() << "Saved color for" << componentId << ":" << color.name();
    }
}

QColor ComponentPersistence::getComponentColor(const QString& componentId)
{
    if (m_workingDirectory.isEmpty()) {
        return QColor();
    }
    
    QString metaFileName = componentId + ".meta.json";
    QString metaFilePath = QDir(m_workingDirectory).filePath(metaFileName);
    
    QFile file(metaFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QColor();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        return QColor();
    }
    
    QJsonObject obj = doc.object();
    QString colorStr = obj["color"].toString();
    
    if (!colorStr.isEmpty() && QColor::isValidColorName(colorStr)) {
        return QColor(colorStr);
    }
    
    return QColor();
}

void ComponentPersistence::deleteComponentFile(const QString& componentId, bool actuallyDelete)
{
    qDebug() << "🗑️ ComponentPersistence::deleteComponentFile() called for component:" << componentId 
             << "actuallyDelete:" << actuallyDelete;
    
    // Safety validation: Ensure component ID is not empty and contains only safe characters
    if (componentId.isEmpty()) {
        qWarning() << "⚠️ Cannot process component - component ID is empty";
        return;
    }
    
    // Validate component ID contains only safe characters (alphanumeric, underscore, hyphen)
    QRegularExpression safeIdRegex("^[a-zA-Z0-9_-]+$");
    if (!safeIdRegex.match(componentId).hasMatch()) {
        qWarning() << "⚠️ Cannot process component - invalid component ID format:" << componentId;
        return;
    }
    
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "⚠️ Cannot process component - working directory is empty";
        return;
    }
    
    // Validate working directory exists and is accessible
    QDir dir(m_workingDirectory);
    if (!dir.exists()) {
        qWarning() << "⚠️ Cannot process component - working directory does not exist:" << m_workingDirectory;
        return;
    }
    
    // Get file paths
    QString cppFile = componentId + ".cpp";
    QString cppFilePath = dir.filePath(cppFile);
    QString metaFile = componentId + ".meta.json";
    QString metaFilePath = dir.filePath(metaFile);
    
    bool cppExists = QFile::exists(cppFilePath);
    bool metaExists = QFile::exists(metaFilePath);
    
    if (actuallyDelete) {
        // Actually delete the files
        bool cppDeleted = false;
        bool metaDeleted = false;
        
        if (cppExists) {
            cppDeleted = QFile::remove(cppFilePath);
            if (cppDeleted) {
                qDebug() << "✅ Deleted component .cpp file:" << cppFile;
            } else {
                qWarning() << "❌ Failed to delete component .cpp file:" << cppFile;
            }
        } else {
            qDebug() << "ℹ️ Component .cpp file doesn't exist:" << cppFile;
        }
        
        if (metaExists) {
            metaDeleted = QFile::remove(metaFilePath);
            if (metaDeleted) {
                qDebug() << "✅ Deleted component .meta.json file:" << metaFile;
            } else {
                qWarning() << "❌ Failed to delete component .meta.json file:" << metaFile;
            }
        } else {
            qDebug() << "ℹ️ Component .meta.json file doesn't exist:" << metaFile;
        }
        
        qDebug() << "🗑️ Component files deletion completed for:" << componentId;
    } else {
        // Just check for existence without deleting
        if (cppExists) {
            qDebug() << "ℹ️ Component .cpp file exists:" << cppFile;
        } else {
            qDebug() << "ℹ️ Component .cpp file doesn't exist:" << cppFile;
        }
        
        if (metaExists) {
            qDebug() << "ℹ️ Component .meta.json file exists:" << metaFile;
        } else {
            qDebug() << "ℹ️ Component .meta.json file doesn't exist:" << metaFile;
        }
        
        qDebug() << "🔧 Component cleanup completed for:" << componentId << "(files preserved)";
    }
}

// Enhanced metadata management methods
void ComponentPersistence::updateComponentMetadata(const QString& componentId, const QJsonObject& metadata)
{
    if (m_workingDirectory.isEmpty()) {
        return;
    }
    
    // Use centralized metadata approach - just update the cache
    updateCachedMetadata(componentId, metadata);
    
    qDebug() << "Updated enhanced metadata for component:" << componentId;
}

QJsonObject ComponentPersistence::getComponentMetadata(const QString& componentId)
{
    return getCachedMetadata(componentId);
}

void ComponentPersistence::updateComponentProperty(const QString& componentId, const QString& property, const QVariant& value)
{
    QJsonObject metadata = getCachedMetadata(componentId);
    
    // Update the specific property in the appropriate section
    if (property == "position") {
        QJsonObject geometry = metadata["geometry"].toObject();
        QPointF pos = value.value<QPointF>();
        QJsonObject positionObj;
        positionObj["x"] = pos.x();
        positionObj["y"] = pos.y();
        geometry["position"] = positionObj;
        metadata["geometry"] = geometry;
    } else if (property == "size") {
        QJsonObject geometry = metadata["geometry"].toObject();
        QSizeF size = value.value<QSizeF>();
        QJsonObject sizeObj;
        sizeObj["width"] = size.width();
        sizeObj["height"] = size.height();
        geometry["size"] = sizeObj;
        metadata["geometry"] = geometry;
    } else if (property == "color") {
        QJsonObject appearance = metadata["appearance"].toObject();
        appearance["color"] = value.value<QColor>().name(QColor::HexArgb);
        metadata["appearance"] = appearance;
    } else if (property == "opacity") {
        QJsonObject appearance = metadata["appearance"].toObject();
        appearance["opacity"] = value.toDouble();
        metadata["appearance"] = appearance;
    } else if (property == "visible") {
        QJsonObject appearance = metadata["appearance"].toObject();
        appearance["visible"] = value.toBool();
        metadata["appearance"] = appearance;
    } else if (property == "rotation") {
        QJsonObject geometry = metadata["geometry"].toObject();
        geometry["rotation"] = value.toDouble();
        metadata["geometry"] = geometry;
    } else {
        // Store in custom properties
        QJsonObject properties = metadata["properties"].toObject();
        QJsonObject customAttributes = properties["customAttributes"].toObject();
        customAttributes[property] = QJsonValue::fromVariant(value);
        properties["customAttributes"] = customAttributes;
        metadata["properties"] = properties;
    }
    
    // Update modification timestamp
    metadata["modified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Save and cache the updated metadata
    updateCachedMetadata(componentId, metadata);
    
    // For critical properties like position, save immediately to ensure persistence
    if (property == "position" || property == "size" || property == "color") {
        // Use centralized metadata approach - saveAllMetadataToFile() handles immediate persistence
        saveAllMetadataToFile();
        qDebug() << "💾 Immediately saved" << property << "for component:" << componentId;
    } else {
        // For other properties, use batch update for performance
        scheduleMetadataUpdate(componentId);
    }
}

QVariant ComponentPersistence::getComponentProperty(const QString& componentId, const QString& property)
{
    QJsonObject metadata = getCachedMetadata(componentId);
    
    if (property == "position") {
        QJsonObject geometry = metadata["geometry"].toObject();
        QJsonObject positionObj = geometry["position"].toObject();
        return QVariant(QPointF(positionObj["x"].toDouble(), positionObj["y"].toDouble()));
    } else if (property == "size") {
        QJsonObject geometry = metadata["geometry"].toObject();
        QJsonObject sizeObj = geometry["size"].toObject();
        return QVariant(QSizeF(sizeObj["width"].toDouble(), sizeObj["height"].toDouble()));
    } else if (property == "color") {
        QJsonObject appearance = metadata["appearance"].toObject();
        QString colorStr = appearance["color"].toString();
        return QVariant(QColor(colorStr));
    } else if (property == "opacity") {
        QJsonObject appearance = metadata["appearance"].toObject();
        return QVariant(appearance["opacity"].toDouble());
    } else if (property == "visible") {
        QJsonObject appearance = metadata["appearance"].toObject();
        return QVariant(appearance["visible"].toBool());
    } else if (property == "rotation") {
        QJsonObject geometry = metadata["geometry"].toObject();
        return QVariant(geometry["rotation"].toDouble());
    } else {
        // Get from custom properties
        QJsonObject properties = metadata["properties"].toObject();
        QJsonObject customAttributes = properties["customAttributes"].toObject();
        return customAttributes[property].toVariant();
    }
}

// Port-specific persistence methods
void ComponentPersistence::updatePortConfiguration(const QString& componentId, const QString& portName, const QJsonObject& portConfig)
{
    QJsonObject metadata = getCachedMetadata(componentId);
    QJsonObject ports = metadata["ports"].toObject();
    
    // Find and update the specific port
    QJsonArray inputs = ports["inputs"].toArray();
    QJsonArray outputs = ports["outputs"].toArray();
    
    for (int i = 0; i < inputs.size(); ++i) {
        QJsonObject port = inputs[i].toObject();
        if (port["name"].toString() == portName) {
            inputs[i] = portConfig;
            break;
        }
    }
    
    for (int i = 0; i < outputs.size(); ++i) {
        QJsonObject port = outputs[i].toObject();
        if (port["name"].toString() == portName) {
            outputs[i] = portConfig;
            break;
        }
    }
    
    ports["inputs"] = inputs;
    ports["outputs"] = outputs;
    metadata["ports"] = ports;
    metadata["modified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    scheduleMetadataUpdate(componentId);
    updateCachedMetadata(componentId, metadata);
}

QJsonObject ComponentPersistence::getPortConfiguration(const QString& componentId, const QString& portName)
{
    QJsonObject metadata = getCachedMetadata(componentId);
    QJsonObject ports = metadata["ports"].toObject();
    
    // Search in inputs
    QJsonArray inputs = ports["inputs"].toArray();
    for (const QJsonValue& value : inputs) {
        QJsonObject port = value.toObject();
        if (port["name"].toString() == portName) {
            return port;
        }
    }
    
    // Search in outputs
    QJsonArray outputs = ports["outputs"].toArray();
    for (const QJsonValue& value : outputs) {
        QJsonObject port = value.toObject();
        if (port["name"].toString() == portName) {
            return port;
        }
    }
    
    return QJsonObject(); // Port not found
}

void ComponentPersistence::updateAllPortConfigurations(const QString& componentId, const QJsonObject& portsConfig)
{
    QJsonObject metadata = getCachedMetadata(componentId);
    metadata["ports"] = portsConfig;
    metadata["modified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    scheduleMetadataUpdate(componentId);
    updateCachedMetadata(componentId, metadata);
}

// Enhanced query methods
QJsonObject ComponentPersistence::getComponentGeometry(const QString& componentId)
{
    QJsonObject metadata = getCachedMetadata(componentId);
    return metadata["geometry"].toObject();
}

QJsonObject ComponentPersistence::getComponentAppearance(const QString& componentId)
{
    QJsonObject metadata = getCachedMetadata(componentId);
    return metadata["appearance"].toObject();
}

QJsonObject ComponentPersistence::getComponentProperties(const QString& componentId)
{
    QJsonObject metadata = getCachedMetadata(componentId);
    return metadata["properties"].toObject();
}

// Validation and integrity methods
bool ComponentPersistence::validateComponentMetadata(const QString& componentId)
{
    QJsonObject metadata = getCachedMetadata(componentId);
    
    // Check required fields
    if (!metadata.contains("id") || !metadata.contains("type") || 
        !metadata.contains("version") || !metadata.contains("geometry")) {
        return false;
    }
    
    // Validate geometry structure
    QJsonObject geometry = metadata["geometry"].toObject();
    if (!geometry.contains("position") || !geometry.contains("size")) {
        return false;
    }
    
    QJsonObject position = geometry["position"].toObject();
    QJsonObject size = geometry["size"].toObject();
    if (!position.contains("x") || !position.contains("y") ||
        !size.contains("width") || !size.contains("height")) {
        return false;
    }
    
    return true;
}

void ComponentPersistence::repairComponentMetadata(const QString& componentId)
{
    QJsonObject metadata = getCachedMetadata(componentId);
    
    // Add missing required fields with defaults
    if (!metadata.contains("version")) {
        metadata["version"] = "1.0";
    }
    
    if (!metadata.contains("created")) {
        metadata["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    }
    
    if (!metadata.contains("geometry")) {
        QJsonObject geometry;
        geometry["position"] = QJsonObject{{"x", 0}, {"y", 0}};
        geometry["size"] = QJsonObject{{"width", 120}, {"height", 80}};
        geometry["rotation"] = 0;
        geometry["zIndex"] = 1;
        metadata["geometry"] = geometry;
    }
    
    if (!metadata.contains("appearance")) {
        QJsonObject appearance;
        appearance["color"] = "";
        appearance["opacity"] = 1.0;
        appearance["visible"] = true;
        appearance["highlighted"] = false;
        metadata["appearance"] = appearance;
    }
    
    if (!metadata.contains("properties")) {
        QJsonObject properties;
        properties["enabled"] = true;
        properties["locked"] = false;
        properties["customAttributes"] = QJsonObject();
        metadata["properties"] = properties;
    }
    
    metadata["modified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    updateComponentMetadata(componentId, metadata);
    qDebug() << "Repaired metadata for component:" << componentId;
}

bool ComponentPersistence::migrateLegacyMetadata(const QString& componentId)
{
    QJsonObject metadata = getCachedMetadata(componentId);
    
    if (metadata.contains("version")) {
        return false; // Already migrated
    }
    
    QJsonObject migratedMetadata = migrateLegacyMetadataToEnhanced(metadata);
    updateComponentMetadata(componentId, migratedMetadata);
    
    qDebug() << "Migrated legacy metadata for component:" << componentId;
    return true;
}

// Performance optimization methods
void ComponentPersistence::scheduleMetadataUpdate(const QString& componentId)
{
    m_pendingUpdates.insert(componentId);
    if (!m_batchUpdateTimer->isActive()) {
        m_batchUpdateTimer->start();
    }
}

void ComponentPersistence::clearMetadataCache()
{
    m_metadataCache.clear();
    m_cacheTimestamps.clear();
    qDebug() << "Cleared metadata cache";
}

void ComponentPersistence::performBatchMetadataUpdate()
{
    // Use centralized metadata approach - save all cached metadata to meta.json
    saveAllMetadataToFile();
    m_pendingUpdates.clear();
    qDebug() << "Performed batch metadata update for" << m_pendingUpdates.size() << "components";
}

// Helper methods
QJsonObject ComponentPersistence::createEnhancedMetadata(const QString& componentId, const QString& componentType, 
                                                        const QPointF& position, const QSizeF& size, const QColor& color)
{
    QJsonObject metadata;
    QDateTime currentTime = QDateTime::currentDateTime();
    
    // Basic identification
    metadata["id"] = componentId;
    metadata["type"] = componentType;
    metadata["name"] = componentType; // Display name
    metadata["version"] = "1.0";
    
    // Timestamps
    metadata["created"] = currentTime.toString(Qt::ISODate);
    metadata["createdTimestamp"] = currentTime.toMSecsSinceEpoch();
    metadata["modified"] = currentTime.toString(Qt::ISODate);
    metadata["modifiedTimestamp"] = currentTime.toMSecsSinceEpoch();
    metadata["lastAccessed"] = currentTime.toString(Qt::ISODate);
    
    // Component details
    QJsonObject componentDetails;
    componentDetails["description"] = QString("Auto-generated %1 component").arg(componentType);
    componentDetails["author"] = "SCV User";
    componentDetails["category"] = "Ready Component";
    componentDetails["tags"] = QJsonArray{componentType, "ready-component", "auto-generated"};
    componentDetails["editable"] = true;
    componentDetails["version"] = "1.0.0";
    componentDetails["license"] = "MIT";
    componentDetails["documentation"] = QString("Component documentation for %1").arg(componentType);
    metadata["componentDetails"] = componentDetails;
    
    // Enhanced geometry information
    QJsonObject geometry;
    QJsonObject positionObj;
    positionObj["x"] = position.x();
    positionObj["y"] = position.y();
    positionObj["originalX"] = position.x(); // Track original position
    positionObj["originalY"] = position.y();
    positionObj["snapToGrid"] = true;
    geometry["position"] = positionObj;
    
    QJsonObject sizeObj;
    sizeObj["width"] = size.width();
    sizeObj["height"] = size.height();
    sizeObj["originalWidth"] = size.width(); // Track original size
    sizeObj["originalHeight"] = size.height();
    sizeObj["minWidth"] = 50.0;
    sizeObj["minHeight"] = 50.0;
    sizeObj["maxWidth"] = 1000.0;
    sizeObj["maxHeight"] = 1000.0;
    sizeObj["aspectRatio"] = size.width() / size.height();
    geometry["size"] = sizeObj;
    
    geometry["rotation"] = 0.0;
    geometry["originalRotation"] = 0.0;
    geometry["zIndex"] = 1;
    geometry["scale"] = 1.0;
    geometry["transform"] = QJsonObject{
        {"scaleX", 1.0},
        {"scaleY", 1.0},
        {"skewX", 0.0},
        {"skewY", 0.0}
    };
    metadata["geometry"] = geometry;
    
    // Enhanced appearance
    QJsonObject appearance;
    appearance["color"] = color.isValid() ? color.name(QColor::HexArgb) : "#6496C8";
    appearance["originalColor"] = color.isValid() ? color.name(QColor::HexArgb) : "#6496C8";
    appearance["opacity"] = 1.0;
    appearance["originalOpacity"] = 1.0;
    appearance["visible"] = true;
    appearance["highlighted"] = false;
    appearance["selected"] = false;
    appearance["hovered"] = false;
    appearance["borderColor"] = "#000000";
    appearance["borderWidth"] = 1.0;
    appearance["fillStyle"] = "solid";
    appearance["shadowEnabled"] = false;
    appearance["shadowColor"] = "#000000";
    appearance["shadowBlur"] = 4.0;
    appearance["shadowOffset"] = QJsonObject{{"x", 2.0}, {"y", 2.0}};
    metadata["appearance"] = appearance;
    
    // Enhanced properties
    QJsonObject properties;
    properties["enabled"] = true;
    properties["locked"] = false;
    properties["selectable"] = true;
    properties["movable"] = true;
    properties["resizable"] = true;
    properties["rotatable"] = true;
    properties["copyable"] = true;
    properties["deletable"] = true;
    properties["groupable"] = true;
    properties["priority"] = 0;
    properties["weight"] = 1.0;
    properties["customAttributes"] = QJsonObject();
    properties["metadata"] = QJsonObject{
        {"source", "drag-drop"},
        {"creationMethod", "user-action"},
        {"parentGroup", ""},
        {"layer", "default"}
    };
    metadata["properties"] = properties;
    
    // Port information
    QJsonObject ports;
    ports["inputPorts"] = QJsonArray();
    ports["outputPorts"] = QJsonArray();
    ports["bidirectionalPorts"] = QJsonArray();
    ports["totalPorts"] = 0;
    ports["connectedPorts"] = 0;
    ports["portConfiguration"] = "auto";
    metadata["ports"] = ports;
    
    // Connection tracking
    QJsonObject connections;
    connections["inputConnections"] = QJsonArray();
    connections["outputConnections"] = QJsonArray();
    connections["totalConnections"] = 0;
    connections["maxInputConnections"] = -1; // -1 means unlimited
    connections["maxOutputConnections"] = -1;
    connections["connectionHistory"] = QJsonArray();
    metadata["connections"] = connections;
    
    // Validation and health
    QJsonObject validation;
    validation["status"] = "valid";
    validation["errors"] = QJsonArray();
    validation["warnings"] = QJsonArray();
    validation["lastValidated"] = currentTime.toString(Qt::ISODate);
    validation["validationRules"] = QJsonArray{
        "position-in-bounds",
        "size-within-limits",
        "no-overlapping-ports"
    };
    metadata["validation"] = validation;
    
    // Performance and statistics
    QJsonObject statistics;
    statistics["usageCount"] = 0;
    statistics["lastUsed"] = currentTime.toString(Qt::ISODate);
    statistics["moveCount"] = 0;
    statistics["resizeCount"] = 0;
    statistics["rotateCount"] = 0;
    statistics["connectionCount"] = 0;
    statistics["selectionCount"] = 0;
    statistics["averageSessionTime"] = 0.0;
    metadata["statistics"] = statistics;
    
    // History and audit trail
    QJsonObject history;
    history["creationEvent"] = QJsonObject{
        {"action", "create"},
        {"timestamp", currentTime.toString(Qt::ISODate)},
        {"user", "system"},
        {"details", QString("Component %1 created via drag-drop").arg(componentType)}
    };
    history["modificationHistory"] = QJsonArray();
    history["versionHistory"] = QJsonArray{QJsonObject{
        {"version", "1.0.0"},
        {"timestamp", currentTime.toString(Qt::ISODate)},
        {"changes", "Initial creation"}
    }};
    metadata["history"] = history;
    
    // Technical specifications
    QJsonObject specifications;
    specifications["technology"] = "SystemC";
    specifications["language"] = "C++";
    specifications["compiler"] = "SystemC";
    specifications["targetPlatform"] = "Generic";
    specifications["clockDomain"] = "";
    specifications["powerDomain"] = "";
    specifications["voltageDomain"] = "";
    specifications["temperatureRange"] = QJsonObject{{"min", -40}, {"max", 125}};
    metadata["specifications"] = specifications;
    
    return metadata;
}

QJsonObject ComponentPersistence::migrateLegacyMetadataToEnhanced(const QJsonObject& legacyMetadata)
{
    QJsonObject enhancedMetadata;
    
    // Copy basic information
    enhancedMetadata["id"] = legacyMetadata["id"];
    enhancedMetadata["type"] = legacyMetadata["type"];
    enhancedMetadata["version"] = "1.0";
    enhancedMetadata["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    enhancedMetadata["modified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Migrate geometry
    QJsonObject geometry;
    if (legacyMetadata.contains("position")) {
        geometry["position"] = legacyMetadata["position"];
    } else {
        geometry["position"] = QJsonObject{{"x", 0}, {"y", 0}};
    }
    
    if (legacyMetadata.contains("size")) {
        geometry["size"] = legacyMetadata["size"];
    } else {
        geometry["size"] = QJsonObject{{"width", 120}, {"height", 80}};
    }
    
    geometry["rotation"] = 0;
    geometry["zIndex"] = 1;
    enhancedMetadata["geometry"] = geometry;
    
    // Migrate appearance
    QJsonObject appearance;
    appearance["color"] = legacyMetadata.value("color").toString();
    appearance["opacity"] = 1.0;
    appearance["visible"] = true;
    appearance["highlighted"] = false;
    enhancedMetadata["appearance"] = appearance;
    
    // Add default properties
    QJsonObject properties;
    properties["enabled"] = true;
    properties["locked"] = false;
    properties["customAttributes"] = QJsonObject();
    enhancedMetadata["properties"] = properties;
    
    // Add default validation
    QJsonObject validation;
    validation["status"] = "valid";
    validation["errors"] = QJsonArray();
    validation["warnings"] = QJsonArray();
    enhancedMetadata["validation"] = validation;
    
    // Add default connections
    QJsonObject connections;
    connections["inputConnections"] = QJsonArray();
    connections["outputConnections"] = QJsonArray();
    enhancedMetadata["connections"] = connections;
    
    return enhancedMetadata;
}

void ComponentPersistence::restorePortConfigurations(ReadyComponentGraphicsItem* component, const QJsonObject& portsConfig)
{
    if (!component) return;
    
    // This would need to be implemented in ReadyComponentGraphicsItem
    // to support custom port configurations
    Q_UNUSED(portsConfig);
    
    // For now, we just log that port configurations are available
    QJsonArray inputs = portsConfig["inputs"].toArray();
    QJsonArray outputs = portsConfig["outputs"].toArray();
    
    qDebug() << "Restoring port configurations for" << component->getName()
             << ":" << inputs.size() << "inputs," << outputs.size() << "outputs";
}

void ComponentPersistence::restoreComponentProperties(ReadyComponentGraphicsItem* component, const QJsonObject& propertiesConfig)
{
    if (!component) return;
    
    // Restore enabled state
    if (propertiesConfig.contains("enabled")) {
        // This would need to be implemented in ReadyComponentGraphicsItem
        // component->setEnabled(propertiesConfig["enabled"].toBool());
    }
    
    // Restore locked state
    if (propertiesConfig.contains("locked")) {
        // This would need to be implemented in ReadyComponentGraphicsItem
        // component->setLocked(propertiesConfig["locked"].toBool());
    }
    
    // Restore custom attributes
    if (propertiesConfig.contains("customAttributes")) {
        QJsonObject customAttrs = propertiesConfig["customAttributes"].toObject();
        // Store custom attributes for future use
        qDebug() << "Restored custom attributes for" << component->getName()
                 << ":" << customAttrs.keys().size() << "attributes";
    }
}

QJsonObject ComponentPersistence::loadMetadataFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QJsonObject();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        return QJsonObject();
    }
    
    return doc.object();
}

void ComponentPersistence::saveMetadataToFile(const QString& filePath, const QJsonObject& metadata)
{
    qDebug() << "💾 ComponentPersistence::saveMetadataToFile() called for file:" << filePath;
    // This method is now deprecated - use saveAllMetadataToFile() for centralized approach
    Q_UNUSED(filePath)
    Q_UNUSED(metadata)
    qWarning() << "⚠️ saveMetadataToFile() is deprecated - use saveAllMetadataToFile() instead";
}

QString ComponentPersistence::getMetadataFilePath(const QString& componentId)
{
    // No longer used - metadata is stored in centralized meta.json
    Q_UNUSED(componentId)
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    return QDir(metaDir).filePath("meta.json");
}

// Metadata caching methods
QJsonObject ComponentPersistence::getCachedMetadata(const QString& componentId)
{
    // Check cache first
    if (m_metadataCache.contains(componentId)) {
        return m_metadataCache[componentId];
    }
    
    // Load from centralized meta.json
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QString metaFilePath = QDir(metaDir).filePath("meta.json");
    
    QFile file(metaFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QJsonObject(); // No meta.json exists yet
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        return QJsonObject();
    }
    
    QJsonObject rootObj = doc.object();
    QJsonObject components = rootObj["components"].toObject();
    
    // Find the specific component by key
    if (components.contains(componentId)) {
        QJsonObject metadata = components[componentId].toObject();
        m_metadataCache[componentId] = metadata;
        return metadata;
    }
    
    return QJsonObject();
}

void ComponentPersistence::updateCachedMetadata(const QString& componentId, const QJsonObject& metadata)
{
    m_metadataCache[componentId] = metadata;
    m_cacheTimestamps[componentId] = QDateTime::currentDateTime();
    
    // Save to centralized meta.json immediately
    saveAllMetadataToFile();
    
    qDebug() << "✅ Cached metadata updated and saved to meta.json for component:" << componentId;
}

void ComponentPersistence::saveAllMetadataToFile()
{
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "⚠️ No working directory set - cannot save metadata";
        return;
    }
    
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QDir dir(metaDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString metaFilePath = dir.filePath("meta.json");
    
    // Load existing meta.json or create new one
    QJsonObject rootObj;
    QJsonObject existingComponents;
    QFile metaFile(metaFilePath);
    if (metaFile.exists() && metaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(metaFile.readAll());
        metaFile.close();
        if (doc.isObject()) {
            rootObj = doc.object();
            // Preserve existing components
            existingComponents = rootObj["components"].toObject();
            qDebug() << "📂 Loaded" << existingComponents.size() << "existing components from meta.json";
        }
    }
    
    // Merge existing components with cached components (preserve existing, update/add new)
    QJsonObject components = existingComponents; // Start with existing components
    for (auto it = m_metadataCache.begin(); it != m_metadataCache.end(); ++it) {
        QString componentId = it.key();
        QJsonObject componentMetadata = it.value();
        // Update modification timestamp
        componentMetadata["modified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        componentMetadata["modifiedTimestamp"] = QDateTime::currentDateTime().toMSecsSinceEpoch();
        components[componentId] = componentMetadata; // This will add new or update existing
    }
    
    qDebug() << "💾 Saving" << components.size() << "total components to meta.json (preserved" 
             << existingComponents.size() << "existing, updated" << m_metadataCache.size() << "cached)";
    
    // Create the proper meta.json structure as requested
    rootObj["components"] = components;
    rootObj["version"] = "1.0";
    rootObj["totalComponents"] = components.size();
    
    // Ensure other sections exist with proper structure
    if (!rootObj.contains("connections")) {
        rootObj["connections"] = QJsonArray();
    }
    if (!rootObj.contains("textItems")) {
        rootObj["textItems"] = QJsonArray();
    }
    if (!rootObj.contains("wires")) {
        rootObj["wires"] = QJsonArray();
    }
    
    // Add metadata section
    QJsonObject metadata;
    metadata["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    metadata["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    metadata["totalComponents"] = components.size();
    metadata["totalConnections"] = rootObj["connections"].toArray().size();
    metadata["totalTextItems"] = rootObj["textItems"].toArray().size();
    metadata["totalWires"] = rootObj["wires"].toArray().size();
    rootObj["metadata"] = metadata;
    
    // Save to file
    if (!metaFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "❌ Failed to save meta.json:" << metaFile.errorString();
        return;
    }
    
    QJsonDocument doc(rootObj);
    metaFile.write(doc.toJson(QJsonDocument::Indented));
    metaFile.close();
    
    qDebug() << "💾 Saved enhanced metadata for" << components.size() << "components to meta.json";
    qDebug() << "📄 meta.json file updated with latest component positions and geometry";
}

void ComponentPersistence::restoreComponentCounter()
{
    if (m_workingDirectory.isEmpty()) {
        return;
    }
    
    QDir dir(m_workingDirectory);
    QStringList metaFiles = dir.entryList(QStringList() << "*.meta.json", QDir::Files);
    
    int maxCounter = 0;
    for (const QString& metaFile : metaFiles) {
        QString metaFilePath = dir.filePath(metaFile);
        QJsonObject metadata = loadMetadataFromFile(metaFilePath);
        
        if (metadata.isEmpty()) {
            continue;
        }
        
        QString id = metadata["id"].toString();
        if (id.isEmpty()) {
            continue;
        }
        
        // Extract number from component ID (format: ComponentType_Number)
        QString numberStr = id.mid(id.lastIndexOf('_') + 1);
        bool ok;
        int number = numberStr.toInt(&ok);
        if (ok && number > maxCounter) {
            maxCounter = number;
        }
    }
    
    // Set the counter to the highest found number to avoid ID collisions
    m_componentCounter = maxCounter;
    qDebug() << "🔢 Restored component counter to:" << m_componentCounter << "from" << metaFiles.size() << "metadata files";
}

// Additional update methods for new properties
void ComponentPersistence::updateComponentOpacity(const QString& componentId, qreal opacity)
{
    updateComponentProperty(componentId, "opacity", QVariant(opacity));
}

void ComponentPersistence::updateComponentVisibility(const QString& componentId, bool visible)
{
    updateComponentProperty(componentId, "visible", QVariant(visible));
}

