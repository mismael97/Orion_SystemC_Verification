// ComponentPortManager.h
#ifndef COMPONENTPORTMANAGER_H
#define COMPONENTPORTMANAGER_H

#include <QPointF>
#include <QList>
#include <QString>
#include <QColor>

class WireGraphicsItem;
struct ModuleInfo;

/**
 * @brief Manages port functionality for ready components
 * 
 * This class handles:
 * - Port position calculations
 * - Port detection and highlighting
 * - Port configuration based on component type
 * - Dynamic port updates from parsed files
 */
class ComponentPortManager
{
public:
    ComponentPortManager(const QString& componentName, qreal width, qreal height);
    
    // Port configuration
    QList<QPointF> getInputPorts() const;
    QList<QPointF> getOutputPorts() const;
    QPointF getPortAt(const QPointF& pos, bool& isInput) const;
    bool isNearPort(const QPointF& pos) const;
    
    // Port highlighting
    void setHighlightedPort(const QPointF& port) { m_highlightedPort = port; }
    void clearHighlightedPort() { m_highlightedPort = QPointF(); }
    QPointF getHighlightedPort() const { return m_highlightedPort; }
    
    // Port color management
    QColor getPortColor(const QPointF& port, bool isInput, const QList<WireGraphicsItem*>& wires) const;
    bool isPortConnected(const QPointF& port, bool isInput, const QList<WireGraphicsItem*>& wires) const;
    WireGraphicsItem* getWireAtPort(const QPointF& port, bool isInput, const QList<WireGraphicsItem*>& wires) const;
    
    // Update dimensions
    void updateDimensions(qreal width, qreal height);
    void setComponentName(const QString& name) { m_componentName = name; }
    
    // Dynamic port updates
    void updatePortsFromModuleInfo(const ModuleInfo& moduleInfo);
    bool hasDynamicPorts() const { return m_useDynamicPorts; }
    
    // Constants
    static constexpr int PORT_RADIUS = 6;
    static constexpr int PORT_DETECTION_RADIUS = 15;

private:
    QString m_componentName;
    qreal m_width;
    qreal m_height;
    QPointF m_highlightedPort;
    
    // Dynamic port storage
    bool m_useDynamicPorts;
    int m_dynamicInputCount;
    int m_dynamicOutputCount;
    
    // Helper methods
    int getNumInputPorts() const;
    int getNumOutputPorts() const;
};

#endif // COMPONENTPORTMANAGER_H

