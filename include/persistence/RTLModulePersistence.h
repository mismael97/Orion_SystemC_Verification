// RTLModulePersistence.h
#ifndef RTLMODULEPERSISTENCE_H
#define RTLMODULEPERSISTENCE_H

#include <QString>
#include <QJsonObject>
#include <QPointF>
#include <QList>

class QGraphicsScene;
class PersistenceManager;
struct Port;

struct RTLModuleData {
    QString moduleName;
    QString filePath;
    QPointF position;
};

class RTLModulePersistence
{
public:
    explicit RTLModulePersistence(const QString& workingDirectory);
    
    // RTL module placement
    void saveRTLModulePlacement(const QString& moduleName, const QString& filePath, const QPointF& position);
    void updateRTLModulePosition(const QString& moduleName, const QPointF& position);
    void updateRTLModulePorts(const QString& moduleName, const QList<Port>& inputs, const QList<Port>& outputs);
    void removeRTLModulePlacement(const QString& moduleName);
    bool loadRTLModules(QGraphicsScene* scene, PersistenceManager* pm);
    
    // RTL module queries
    QString getRTLModuleFilePath(const QString& moduleName);
    
    // Working directory
    void setWorkingDirectory(const QString& directory);
    QString getWorkingDirectory() const { return m_workingDirectory; }
    
    // Public for legacy code access
    QJsonObject loadRTLPlacementsJson();
    
private:
    QString m_workingDirectory;
    
    void saveRTLPlacementsJson(const QJsonObject& json);
    QList<RTLModuleData> parseRTLPlacements(const QJsonObject& json);
};

#endif // RTLMODULEPERSISTENCE_H

