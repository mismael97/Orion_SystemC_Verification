// SchematicPersistence.h
#ifndef SCHEMATICPERSISTENCE_H
#define SCHEMATICPERSISTENCE_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QPointF>
#include <QColor>
#include <QFont>
#include <QList>

class QGraphicsScene;

struct TextItemData {
    QString text;
    QPointF position;
    QColor color;
    QFont font;
};

class SchematicPersistence
{
public:
    explicit SchematicPersistence(const QString& workingDirectory);
    
    // Text items file management
    QJsonObject loadTextItemsJson();
    void saveTextItemsJson(const QJsonObject& json);
    
    // Legacy schematic file management (deprecated)
    void initializeSchematicFile();
    QJsonObject loadSchematicJson();
    void saveSchematicJson(const QJsonObject& json);
    
    // Text item operations
    void saveTextItem(const QString& text, const QPointF& position, const QColor& color, const QFont& font);
    void updateTextItem(const QString& oldText, const QPointF& oldPosition, 
                       const QString& newText, const QPointF& newPosition,
                       const QColor& color, const QFont& font);
    void removeTextItem(const QString& text, const QPointF& position);
    bool loadTextItems(QGraphicsScene* scene);
    QList<TextItemData> parseTextItems(const QJsonObject& json);
    
    // Wire metadata operations
    void saveWireMetadata(const QString& wireId, const QString& sourceId, const QPointF& sourcePort,
                         const QString& targetId, const QPointF& targetPort,
                         bool sourceIsRTL, bool targetIsRTL,
                         const QList<QPointF>& controlPoints, qreal orthogonalOffset,
                         const QJsonObject& additionalMetadata = QJsonObject());
    void updateWireMetadata(const QString& wireId, const QJsonObject& metadata);
    void removeWireMetadata(const QString& wireId);
    QJsonObject getWireMetadata(const QString& wireId);
    QJsonArray getAllWiresMetadata();
    
    // Working directory
    void setWorkingDirectory(const QString& directory);
    QString getWorkingDirectory() const { return m_workingDirectory; }
    
private:
    QString m_workingDirectory;
};

#endif // SCHEMATICPERSISTENCE_H

