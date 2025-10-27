// SchematicPersistence.cpp
#include "persistence/SchematicPersistence.h"
#include "graphics/TextGraphicsItem.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QDebug>
#include <QGraphicsScene>
#include <QDateTime>

SchematicPersistence::SchematicPersistence(const QString& workingDirectory)
    : m_workingDirectory(workingDirectory)
{
}

void SchematicPersistence::setWorkingDirectory(const QString& directory)
{
    m_workingDirectory = directory;
    
    // Check if meta.json exists, print contents if present
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QString metaFilePath = QDir(metaDir).filePath("meta.json");
    
    if (QFile::exists(metaFilePath)) {
        qDebug() << "📂 SchematicPersistence: meta.json exists in" << directory;
        QFile file(metaFilePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = file.readAll();
            file.close();
            qDebug() << "📄 Content of meta.json:\n" << QString::fromUtf8(data);
        } else {
            qWarning() << "⚠️ Failed to open meta.json for printing contents";
        }
    } else {
        qDebug() << "📂 SchematicPersistence: meta.json will be created on first save";
    }
}

QJsonObject SchematicPersistence::loadTextItemsJson()
{
    qDebug() << "📂 SchematicPersistence::loadTextItemsJson() called for directory:" << m_workingDirectory;
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "⚠️ Working directory not set for text items";
        return QJsonObject();
    }
    
    // Load from centralized meta.json instead of separate text_items.json
    QString metaDir = QDir(m_workingDirectory).filePath(".srcv");
    QString metaFilePath = QDir(metaDir).filePath("meta.json");
    
    QFile file(metaFilePath);
    if (!file.exists()) {
        qDebug() << "📄 meta.json does not exist yet, will create on first save";
        // Return empty but valid structure
        QJsonObject emptyData;
        emptyData["version"] = "1.0";
        emptyData["textItems"] = QJsonArray();
        return emptyData;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "⚠️ Failed to open meta.json for reading";
        return QJsonObject();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    if (data.isEmpty()) {
        qWarning() << "⚠️ meta.json is empty, returning default structure";
        QJsonObject emptyData;
        emptyData["version"] = "1.0";
        emptyData["textItems"] = QJsonArray();
        return emptyData;
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "⚠️ JSON parse error in meta.json:" << parseError.errorString();
        qWarning() << "Creating backup and returning default structure";
        
        // Create backup of corrupted file
        QString backupPath = metaFilePath + ".backup";
        QFile::copy(metaFilePath, backupPath);
        
        QJsonObject emptyData;
        emptyData["version"] = "1.0";
        emptyData["textItems"] = QJsonArray();
        return emptyData;
    }
    
    QJsonObject rootObj = doc.object();
    QJsonObject textItemsObj;
    textItemsObj["version"] = "1.0";
    textItemsObj["textItems"] = rootObj["textItems"].toArray();
    
    qDebug() << "📂 Loaded text items from meta.json with" << textItemsObj["textItems"].toArray().size() << "text item(s)";
    
    return textItemsObj;
}

void SchematicPersistence::saveTextItemsJson(const QJsonObject& json)
{
    qDebug() << "💿 saveTextItemsJson called";
    
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "⚠️ Working directory not set for text items";
        return;
    }
    
    // Save to centralized meta.json instead of separate text_items.json
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QDir dir(metaDir);
    if (!dir.exists()) {
        bool createdMeta = dir.mkpath(".");
        qDebug() << "📁 Created metadata directory for meta.json:" << createdMeta;
        if (!createdMeta) {
            qWarning() << "❌ Failed to create metadata directory:" << dir.absolutePath();
            return;
        }
    }
    QString metaFilePath = dir.filePath("meta.json");
    qDebug() << "   Target file:" << metaFilePath;
    
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
    
    // Update text items section
    rootObj["textItems"] = json["textItems"].toArray();
    
    int itemCount = json["textItems"].toArray().size();
    qDebug() << "   Items to save:" << itemCount;
    
    // Write with indentation for readability
    QJsonDocument doc(rootObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
    qDebug() << "   JSON data size:" << jsonData.size() << "bytes";
    
    if (!metaFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "❌ Failed to open meta.json for writing:" << metaFile.errorString();
        qWarning() << "   Path:" << metaFilePath;
        return;
    }
    
    qDebug() << "   File opened successfully for writing";
    
    qint64 bytesWritten = metaFile.write(jsonData);
    metaFile.flush(); // Ensure data is written to disk
    metaFile.close();
    
    qDebug() << "   Bytes written:" << bytesWritten;
    
    if (bytesWritten > 0) {
        qDebug() << "✅ Successfully saved text items to meta.json with" << itemCount << "text item(s) (" << bytesWritten << "bytes)";
        qDebug() << "📁 File location:" << metaFilePath;
        
        // Verify the file was created
        if (QFile::exists(metaFilePath)) {
            QFileInfo fileInfo(metaFilePath);
            qDebug() << "✅ File verified to exist on disk, size:" << fileInfo.size() << "bytes";
        } else {
            qWarning() << "❌ File does not exist after write!";
        }
    } else {
        qWarning() << "❌ Failed to write data to meta.json (0 bytes written)";
    }
}

void SchematicPersistence::initializeSchematicFile()
{
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "⚠️ Working directory not set - cannot initialize schematic file";
        return;
    }
    
    // Create meta.json instead of schematic.json
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QDir dir(metaDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString metaFilePath = dir.filePath("meta.json");
    QFile file(metaFilePath);
    
    // Only create if it doesn't exist
    if (!file.exists()) {
        QJsonObject metaData;
        metaData["version"] = "1.0";
        metaData["components"] = QJsonObject(); // Object instead of array
        metaData["connections"] = QJsonArray();
        metaData["textItems"] = QJsonArray();
        metaData["wires"] = QJsonArray();
        metaData["totalComponents"] = 0;
        
        // Add metadata section
        QJsonObject metadata;
        metadata["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        metadata["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        metadata["totalComponents"] = 0;
        metadata["totalConnections"] = 0;
        metadata["totalTextItems"] = 0;
        metadata["totalWires"] = 0;
        metaData["metadata"] = metadata;
        
        saveSchematicJson(metaData);
        qDebug() << "📝 Created new meta.json in" << metaFilePath;
    } else {
        qDebug() << "✅ meta.json already exists, not modifying it";
    }
}

QJsonObject SchematicPersistence::loadSchematicJson()
{
    qDebug() << "📂 SchematicPersistence::loadSchematicJson() called for directory:" << m_workingDirectory;
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "⚠️ Working directory not set for schematic";
        return QJsonObject();
    }
    
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QString schematicPath = QDir(metaDir).filePath("schematic.json");
    if (!QFile::exists(schematicPath)) {
        // Fallback to legacy root
        schematicPath = QDir(m_workingDirectory).filePath("schematic.json");
    }
    QFile file(schematicPath);
    
    if (!file.exists()) {
        qDebug() << "📄 schematic.json does not exist yet, will create on first save";
        // Return empty but valid structure - don't create yet
        QJsonObject emptyData;
        emptyData["version"] = "1.0";
        emptyData["textItems"] = QJsonArray();
        return emptyData;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "⚠️ Failed to open schematic.json for reading";
        return QJsonObject();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    if (data.isEmpty()) {
        qWarning() << "⚠️ schematic.json is empty, returning default structure";
        QJsonObject emptyData;
        emptyData["version"] = "1.0";
        emptyData["textItems"] = QJsonArray();
        return emptyData;
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "⚠️ JSON parse error in schematic.json:" << parseError.errorString();
        qWarning() << "Creating backup and returning default structure";
        
        // Create backup of corrupted file
        QString backupPath = schematicPath + ".backup";
        QFile::copy(schematicPath, backupPath);
        
        QJsonObject emptyData;
        emptyData["version"] = "1.0";
        emptyData["textItems"] = QJsonArray();
        return emptyData;
    }
    
    QJsonObject jsonObj = doc.object();
    
    // Ensure required fields exist
    if (!jsonObj.contains("version")) {
        jsonObj["version"] = "1.0";
    }
    if (!jsonObj.contains("textItems")) {
        jsonObj["textItems"] = QJsonArray();
    }
    
    qDebug() << "📂 Loaded schematic.json with" << jsonObj["textItems"].toArray().size() << "text item(s)";
    
    return jsonObj;
}

void SchematicPersistence::saveSchematicJson(const QJsonObject& json)
{
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "⚠️ Working directory not set for schematic";
        return;
    }
    
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QDir dir(metaDir);
    if (!dir.exists()) {
        dir.mkpath(".");
        qDebug() << "📁 Created metadata directory for schematic.json";
    }
    QString metaFilePath = dir.filePath("meta.json");
    
    // Ensure version field exists
    QJsonObject jsonToSave = json;
    if (!jsonToSave.contains("version")) {
        jsonToSave["version"] = "1.0";
    }
    
    // Ensure all required fields exist
    if (!jsonToSave.contains("components")) {
        jsonToSave["components"] = QJsonObject(); // Object instead of array
    }
    if (!jsonToSave.contains("connections")) {
        jsonToSave["connections"] = QJsonArray();
    }
    if (!jsonToSave.contains("textItems")) {
        jsonToSave["textItems"] = QJsonArray();
    }
    if (!jsonToSave.contains("wires")) {
        jsonToSave["wires"] = QJsonArray();
    }
    if (!jsonToSave.contains("totalComponents")) {
        jsonToSave["totalComponents"] = jsonToSave["components"].toArray().size();
    }
    
    // Ensure metadata field exists
    if (!jsonToSave.contains("metadata")) {
        QJsonObject metadata;
        metadata["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        metadata["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        metadata["totalComponents"] = jsonToSave["components"].toObject().size();
        metadata["totalConnections"] = jsonToSave["connections"].toArray().size();
        metadata["totalWires"] = jsonToSave["wires"].toArray().size();
        metadata["totalTextItems"] = jsonToSave["textItems"].toArray().size();
        jsonToSave["metadata"] = metadata;
    } else {
        // Update metadata
        QJsonObject metadata = jsonToSave["metadata"].toObject();
        metadata["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        metadata["totalComponents"] = jsonToSave["components"].toObject().size();
        metadata["totalConnections"] = jsonToSave["connections"].toArray().size();
        metadata["totalWires"] = jsonToSave["wires"].toArray().size();
        metadata["totalTextItems"] = jsonToSave["textItems"].toArray().size();
        jsonToSave["metadata"] = metadata;
    }
    
    // Write with indentation for readability
    QJsonDocument doc(jsonToSave);
    
    QFile file(metaFilePath);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "⚠️ Failed to open meta.json for writing:" << file.errorString();
        qWarning() << "⚠️ Path:" << metaFilePath;
        return;
    }
    
    qint64 bytesWritten = file.write(doc.toJson(QJsonDocument::Indented));
    file.flush(); // Ensure data is written to disk
    file.close();
    
    if (bytesWritten > 0) {
        int componentCount = jsonToSave["components"].toObject().size();
        int connectionCount = jsonToSave["connections"].toArray().size();
        int textItemCount = jsonToSave["textItems"].toArray().size();
        int wireCount = jsonToSave["wires"].toArray().size();
        qDebug() << "💾 Saved meta.json with" << componentCount << "component(s)," 
                 << connectionCount << "connection(s)," << textItemCount << "text item(s),"
                 << wireCount << "wire(s) (" << bytesWritten << "bytes)";
    } else {
        qWarning() << "⚠️ Failed to write data to meta.json";
    }
    
    // Verify the file was written correctly
    QFile verifyFile(metaFilePath);
    if (verifyFile.exists() && verifyFile.size() > 0) {
        qDebug() << "✅ Verified meta.json saved successfully, size:" << verifyFile.size() << "bytes";
    } else {
        qWarning() << "⚠️ Verification failed - meta.json may not have been saved!";
    }
}

QList<TextItemData> SchematicPersistence::parseTextItems(const QJsonObject& json)
{
    QList<TextItemData> textItems;
    QJsonArray itemsArray = json["textItems"].toArray();
    
    for (const QJsonValue& value : itemsArray) {
        QJsonObject itemObj = value.toObject();
        
        TextItemData data;
        data.text = itemObj["text"].toString();
        
        QJsonObject posObj = itemObj["position"].toObject();
        data.position = QPointF(posObj["x"].toDouble(), posObj["y"].toDouble());
        
        QJsonObject colorObj = itemObj["color"].toObject();
        data.color = QColor(colorObj["r"].toInt(), colorObj["g"].toInt(), 
                           colorObj["b"].toInt(), colorObj["a"].toInt());
        
        QJsonObject fontObj = itemObj["font"].toObject();
        data.font.setFamily(fontObj["family"].toString());
        data.font.setPointSize(fontObj["size"].toInt());
        data.font.setBold(fontObj["bold"].toBool());
        data.font.setItalic(fontObj["italic"].toBool());
        
        textItems.append(data);
    }
    
    return textItems;
}

void SchematicPersistence::saveTextItem(const QString& text, const QPointF& position, 
                                       const QColor& color, const QFont& font)
{
    qDebug() << "📝 SchematicPersistence::saveTextItem called";
    qDebug() << "   Working directory:" << m_workingDirectory;
    
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "❌ Cannot save text item - working directory is empty!";
        return;
    }
    
    // Load existing text items data (or get empty structure)
    QJsonObject json = loadTextItemsJson();
    
    // Ensure version field exists
    if (!json.contains("version")) {
        json["version"] = "1.0";
    }
    
    // Get existing items array
    QJsonArray itemsArray = json["textItems"].toArray();
    qDebug() << "   Current item count:" << itemsArray.size();
    
    // Create new text item object with all properties
    QJsonObject itemObj;
    itemObj["text"] = text;
    
    // Save position
    QJsonObject posObj;
    posObj["x"] = position.x();
    posObj["y"] = position.y();
    itemObj["position"] = posObj;
    
    // Save color
    QJsonObject colorObj;
    colorObj["r"] = color.red();
    colorObj["g"] = color.green();
    colorObj["b"] = color.blue();
    colorObj["a"] = color.alpha();
    itemObj["color"] = colorObj;
    
    // Save font
    QJsonObject fontObj;
    fontObj["family"] = font.family();
    fontObj["size"] = font.pointSize();
    fontObj["bold"] = font.bold();
    fontObj["italic"] = font.italic();
    itemObj["font"] = fontObj;
    
    // Add to array
    itemsArray.append(itemObj);
    json["textItems"] = itemsArray;
    
    qDebug() << "   New item count (after adding):" << itemsArray.size();
    
    // Save to text_items.json
    saveTextItemsJson(json);
    
    QString textItemsPath = QDir(m_workingDirectory).filePath("text_items.json");
    qDebug() << "💾 Saved text item to:" << textItemsPath;
    qDebug() << "   Text:" << text 
             << "| Position: (" << position.x() << "," << position.y() << ")"
             << "| Total items:" << itemsArray.size();
}

void SchematicPersistence::updateTextItem(const QString& oldText, const QPointF& oldPosition,
                                         const QString& newText, const QPointF& newPosition,
                                         const QColor& color, const QFont& font)
{
    QJsonObject json = loadTextItemsJson();
    QJsonArray itemsArray = json["textItems"].toArray();
    
    bool found = false;
    
    // Find and update the matching item
    for (int i = 0; i < itemsArray.size(); ++i) {
        QJsonObject itemObj = itemsArray[i].toObject();
        QJsonObject posObj = itemObj["position"].toObject();
        QPointF pos(posObj["x"].toDouble(), posObj["y"].toDouble());
        QString text = itemObj["text"].toString();
        
        // Match by both text and position (within tolerance)
        if (text == oldText && (pos - oldPosition).manhattanLength() < 5.0) {
            // Update all properties simultaneously
            itemObj["text"] = newText;
            
            QJsonObject newPosObj;
            newPosObj["x"] = newPosition.x();
            newPosObj["y"] = newPosition.y();
            itemObj["position"] = newPosObj;
            
            QJsonObject colorObj;
            colorObj["r"] = color.red();
            colorObj["g"] = color.green();
            colorObj["b"] = color.blue();
            colorObj["a"] = color.alpha();
            itemObj["color"] = colorObj;
            
            QJsonObject fontObj;
            fontObj["family"] = font.family();
            fontObj["size"] = font.pointSize();
            fontObj["bold"] = font.bold();
            fontObj["italic"] = font.italic();
            itemObj["font"] = fontObj;
            
            itemsArray[i] = itemObj;
            json["textItems"] = itemsArray;
            saveTextItemsJson(json);
            
            qDebug() << "✅ Updated text item:" 
                     << "Text:" << oldText << "→" << newText
                     << "| Position:" << oldPosition << "→" << newPosition;
            
            found = true;
            break;
        }
    }
    
    if (!found) {
        qWarning() << "⚠️ Text item not found for update:" << oldText << "at" << oldPosition;
        qDebug() << "Available items in text_items.json:";
        for (int i = 0; i < itemsArray.size(); ++i) {
            QJsonObject itemObj = itemsArray[i].toObject();
            QJsonObject posObj = itemObj["position"].toObject();
            qDebug() << "  -" << itemObj["text"].toString() 
                     << "at (" << posObj["x"].toDouble() << "," << posObj["y"].toDouble() << ")";
        }
    }
}

void SchematicPersistence::removeTextItem(const QString& text, const QPointF& position)
{
    QJsonObject json = loadTextItemsJson();
    QJsonArray itemsArray = json["textItems"].toArray();
    
    // Find and remove the matching item
    for (int i = 0; i < itemsArray.size(); ++i) {
        QJsonObject itemObj = itemsArray[i].toObject();
        QJsonObject posObj = itemObj["position"].toObject();
        QPointF pos(posObj["x"].toDouble(), posObj["y"].toDouble());
        QString itemText = itemObj["text"].toString();
        
        // Match by both text and position (within tolerance)
        if (itemText == text && (pos - position).manhattanLength() < 5.0) {
            itemsArray.removeAt(i);
            json["textItems"] = itemsArray;
            saveTextItemsJson(json);
            
            qDebug() << "🗑️ Removed text item from text_items.json:" << text 
                     << "| Remaining items:" << itemsArray.size();
            return;
        }
    }
    
    qWarning() << "⚠️ Text item not found for removal:" << text << "at" << position;
}

bool SchematicPersistence::loadTextItems(QGraphicsScene* scene)
{
    qDebug() << "📂 SchematicPersistence::loadTextItems() called for scene:" << (scene ? "valid" : "null");
    if (!scene) {
        qWarning() << "⚠️ Null scene provided to loadTextItems";
        return false;
    }
    
    QJsonObject json = loadTextItemsJson();
    if (json.isEmpty() || !json.contains("textItems")) {
        qDebug() << "📄 No text items in text_items.json (file may be new)";
        return true; // Not an error, just no items
    }
    
    QList<TextItemData> textItems = parseTextItems(json);
    
    if (textItems.isEmpty()) {
        qDebug() << "📄 text_items.json exists but textItems array is empty";
        return true;
    }
    
    qDebug() << "📂 Loading" << textItems.size() << "text item(s) from text_items.json";
    
    for (const TextItemData& data : textItems) {
        TextGraphicsItem* textItem = new TextGraphicsItem(data.text);
        textItem->setPos(data.position);
        textItem->setTextColor(data.color);
        textItem->setTextFont(data.font);
        scene->addItem(textItem);
        
        qDebug() << "  ✅ Loaded:" << data.text 
                 << "at (" << data.position.x() << "," << data.position.y() << ")"
                 << "| In scene:" << (textItem->scene() != nullptr)
                 << "| Visible:" << textItem->isVisible()
                 << "| ZValue:" << textItem->zValue();
    }
    
    qDebug() << "✅ Successfully loaded" << textItems.size() << "text item(s) from text_items.json";
    return true;
}

// Wire metadata operations
void SchematicPersistence::saveWireMetadata(const QString& wireId, const QString& sourceId, const QPointF& sourcePort,
                                           const QString& targetId, const QPointF& targetPort,
                                           bool sourceIsRTL, bool targetIsRTL,
                                           const QList<QPointF>& controlPoints, qreal orthogonalOffset,
                                           const QJsonObject& additionalMetadata)
{
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "⚠️ Working directory not set for wire metadata";
        return;
    }
    
    // Load existing meta.json
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QString metaFilePath = QDir(metaDir).filePath("meta.json");
    
    QJsonObject rootObj;
    QFile metaFile(metaFilePath);
    if (metaFile.exists() && metaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(metaFile.readAll());
        metaFile.close();
        if (doc.isObject()) {
            rootObj = doc.object();
        }
    }
    
    QJsonArray wiresArray = rootObj["wires"].toArray();
    
    // Create comprehensive wire metadata
    QJsonObject wireMetadata;
    wireMetadata["id"] = wireId;
    wireMetadata["version"] = "1.0";
    wireMetadata["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    wireMetadata["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Connection information
    wireMetadata["sourceId"] = sourceId;
    wireMetadata["targetId"] = targetId;
    wireMetadata["sourceIsRTL"] = sourceIsRTL;
    wireMetadata["targetIsRTL"] = targetIsRTL;
    
    // Port positions
    QJsonObject sourcePortObj;
    sourcePortObj["x"] = sourcePort.x();
    sourcePortObj["y"] = sourcePort.y();
    wireMetadata["sourcePort"] = sourcePortObj;
    
    QJsonObject targetPortObj;
    targetPortObj["x"] = targetPort.x();
    targetPortObj["y"] = targetPort.y();
    wireMetadata["targetPort"] = targetPortObj;
    
    // Wire geometry
    QJsonObject geometry;
    geometry["orthogonalOffset"] = orthogonalOffset;
    
    // Control points
    QJsonArray controlPointsArray;
    for (const QPointF& point : controlPoints) {
        QJsonObject pointObj;
        pointObj["x"] = point.x();
        pointObj["y"] = point.y();
        controlPointsArray.append(pointObj);
    }
    geometry["controlPoints"] = controlPointsArray;
    wireMetadata["geometry"] = geometry;
    
    // Wire properties
    QJsonObject properties;
    properties["visible"] = true;
    properties["locked"] = false;
    properties["routingMode"] = "auto";
    properties["label"] = "";
    wireMetadata["properties"] = properties;
    
    // Additional metadata
    if (!additionalMetadata.isEmpty()) {
        wireMetadata["customMetadata"] = additionalMetadata;
    }
    
    // Validation
    QJsonObject validation;
    validation["status"] = "valid";
    validation["errors"] = QJsonArray();
    validation["warnings"] = QJsonArray();
    wireMetadata["validation"] = validation;
    
    // Add to wires array
    wiresArray.append(wireMetadata);
    rootObj["wires"] = wiresArray;
    
    // Update metadata section
    QJsonObject metadata = rootObj["metadata"].toObject();
    metadata["totalWires"] = wiresArray.size();
    metadata["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    rootObj["metadata"] = metadata;
    
    // Save to meta.json
    if (!metaFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "⚠️ Failed to save wire metadata to meta.json:" << metaFile.errorString();
        return;
    }
    
    QJsonDocument doc(rootObj);
    metaFile.write(doc.toJson(QJsonDocument::Indented));
    metaFile.close();
    
    qDebug() << "🔗 Saved wire metadata for wire:" << wireId << "from" << sourceId << "to" << targetId;
}

void SchematicPersistence::updateWireMetadata(const QString& wireId, const QJsonObject& metadata)
{
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "⚠️ Working directory not set for wire metadata update";
        return;
    }
    
    // Load existing meta.json
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QString metaFilePath = QDir(metaDir).filePath("meta.json");
    
    QJsonObject rootObj;
    QFile metaFile(metaFilePath);
    if (metaFile.exists() && metaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(metaFile.readAll());
        metaFile.close();
        if (doc.isObject()) {
            rootObj = doc.object();
        }
    }
    
    QJsonArray wiresArray = rootObj["wires"].toArray();
    
    for (int i = 0; i < wiresArray.size(); ++i) {
        QJsonObject wire = wiresArray[i].toObject();
        if (wire["id"].toString() == wireId) {
            // Update the wire metadata
            QJsonObject updatedWire = metadata;
            updatedWire["id"] = wireId;
            updatedWire["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
            
            wiresArray[i] = updatedWire;
            rootObj["wires"] = wiresArray;
            
            // Update metadata section
            QJsonObject metadata = rootObj["metadata"].toObject();
            metadata["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
            rootObj["metadata"] = metadata;
            
            // Save to meta.json
            if (!metaFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                qWarning() << "⚠️ Failed to update wire metadata in meta.json:" << metaFile.errorString();
                return;
            }
            
            QJsonDocument doc(rootObj);
            metaFile.write(doc.toJson(QJsonDocument::Indented));
            metaFile.close();
            
            qDebug() << "🔗 Updated wire metadata for wire:" << wireId;
            return;
        }
    }
    
    qWarning() << "⚠️ Wire not found for metadata update:" << wireId;
}

void SchematicPersistence::removeWireMetadata(const QString& wireId)
{
    if (m_workingDirectory.isEmpty()) {
        qWarning() << "⚠️ Working directory not set for wire metadata removal";
        return;
    }
    
    // Load existing meta.json
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QString metaFilePath = QDir(metaDir).filePath("meta.json");
    
    QJsonObject rootObj;
    QFile metaFile(metaFilePath);
    if (metaFile.exists() && metaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(metaFile.readAll());
        metaFile.close();
        if (doc.isObject()) {
            rootObj = doc.object();
        }
    }
    
    QJsonArray wiresArray = rootObj["wires"].toArray();
    
    for (int i = 0; i < wiresArray.size(); ++i) {
        QJsonObject wire = wiresArray[i].toObject();
        if (wire["id"].toString() == wireId) {
            wiresArray.removeAt(i);
            rootObj["wires"] = wiresArray;
            
            // Update metadata section
            QJsonObject metadata = rootObj["metadata"].toObject();
            metadata["totalWires"] = wiresArray.size();
            metadata["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
            rootObj["metadata"] = metadata;
            
            // Save to meta.json
            if (!metaFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                qWarning() << "⚠️ Failed to remove wire metadata from meta.json:" << metaFile.errorString();
                return;
            }
            
            QJsonDocument doc(rootObj);
            metaFile.write(doc.toJson(QJsonDocument::Indented));
            metaFile.close();
            
            qDebug() << "🔗 Removed wire metadata for wire:" << wireId;
            return;
        }
    }
    
    qWarning() << "⚠️ Wire not found for metadata removal:" << wireId;
}

QJsonObject SchematicPersistence::getWireMetadata(const QString& wireId)
{
    if (m_workingDirectory.isEmpty()) {
        return QJsonObject();
    }
    
    // Load existing meta.json
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QString metaFilePath = QDir(metaDir).filePath("meta.json");
    
    QFile metaFile(metaFilePath);
    if (!metaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QJsonObject();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(metaFile.readAll());
    metaFile.close();
    
    if (!doc.isObject()) {
        return QJsonObject();
    }
    
    QJsonObject rootObj = doc.object();
    QJsonArray wiresArray = rootObj["wires"].toArray();
    
    for (const QJsonValue& value : wiresArray) {
        QJsonObject wire = value.toObject();
        if (wire["id"].toString() == wireId) {
            return wire;
        }
    }
    
    return QJsonObject();
}

QJsonArray SchematicPersistence::getAllWiresMetadata()
{
    if (m_workingDirectory.isEmpty()) {
        return QJsonArray();
    }
    
    // Load existing meta.json
    QString metaDir = QDir(m_workingDirectory).filePath(".scv");
    QString metaFilePath = QDir(metaDir).filePath("meta.json");
    
    QFile metaFile(metaFilePath);
    if (!metaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QJsonArray();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(metaFile.readAll());
    metaFile.close();
    
    if (!doc.isObject()) {
        return QJsonArray();
    }
    
    QJsonObject rootObj = doc.object();
    return rootObj["wires"].toArray();
}


