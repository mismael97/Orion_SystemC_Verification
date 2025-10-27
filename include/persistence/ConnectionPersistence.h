// ConnectionPersistence.h
#ifndef CONNECTIONPERSISTENCE_H
#define CONNECTIONPERSISTENCE_H

#include <QString>
#include <QJsonObject>
#include <QPointF>
#include <QSizeF>
#include <QList>

class QGraphicsScene;
class PersistenceManager;

struct ConnectionData {
    QString sourceId;
    QPointF sourcePort;
    QString targetId;
    QPointF targetPort;
    bool sourceIsRTL;
    bool targetIsRTL;
    QList<QPointF> controlPoints;
    qreal orthogonalOffset;
};


class ConnectionPersistence
{
public:
    explicit ConnectionPersistence(const QString& workingDirectory);
    
    // Connection operations
    void saveConnection(const QString& sourceId, const QPointF& sourcePort,
                       const QString& targetId, const QPointF& targetPort,
                       bool sourceIsRTL, bool targetIsRTL,
                       const QList<QPointF>& controlPoints, qreal orthogonalOffset = 0.0);
    void removeConnection(const QString& sourceId, const QPointF& sourcePort,
                         const QString& targetId, const QPointF& targetPort);
    void updateConnectionControlPoints(const QString& sourceId, const QPointF& sourcePort,
                                       const QString& targetId, const QPointF& targetPort,
                                       const QList<QPointF>& controlPoints);
    void updateConnectionOrthogonalOffset(const QString& sourceId, const QPointF& sourcePort,
                                          const QString& targetId, const QPointF& targetPort,
                                          qreal orthogonalOffset);
    bool loadConnections(QGraphicsScene* scene, PersistenceManager* pm);
    
    // Component tracking in connections
    void updateRTLComponentInConnections(const QString& componentId, const QPointF& position);
    void removeComponentFromConnections(const QString& componentId);
    void removeComponentOnlyFromConnections(const QString& componentId);
    
    // Working directory
    void setWorkingDirectory(const QString& directory);
    QString getWorkingDirectory() const { return m_workingDirectory; }
    
private:
    QString m_workingDirectory;
    
    QJsonObject loadConnectionsJson();
    void saveConnectionsJson(const QJsonObject& json);
    QList<ConnectionData> parseConnections(const QJsonObject& json);
};

#endif // CONNECTIONPERSISTENCE_H

