// ComponentWireManager.cpp
#include "graphics/ready/ComponentWireManager.h"
#include "graphics/wire/WireGraphicsItem.h"
#include "graphics/ReadyComponentGraphicsItem.h"
#include <QLineF>
#include <QDebug>

ComponentWireManager::ComponentWireManager()
{
}

void ComponentWireManager::addWire(WireGraphicsItem* wire)
{
    if (!m_wires.contains(wire)) {
        m_wires.append(wire);
    }
}

void ComponentWireManager::removeWire(WireGraphicsItem* wire)
{
    m_wires.removeAll(wire);
}

void ComponentWireManager::updateWires()
{
    for (WireGraphicsItem* wire : m_wires) {
        wire->updatePath();
    }
}

void ComponentWireManager::updateWirePortPositions(ReadyComponentGraphicsItem* component)
{
    if (!component) return;
    
    // Get the current port positions from the component
    QList<QPointF> inputPorts = component->getInputPorts();
    QList<QPointF> outputPorts = component->getOutputPorts();
    
    // Update each wire's port positions
    for (WireGraphicsItem* wire : m_wires) {
        if (!wire) continue;
        
        QPointF oldSourcePort = wire->getSourcePort();
        QPointF oldTargetPort = wire->getTargetPort();
        bool portsChanged = false;
        
        // Find the closest current port to the old port position
        // This handles the case where ports have been redistributed during resize
        
        // Check if this component is the source
        if (wire->getSource() == component) {
            // Find closest output port to old source port
            QPointF closestPort = oldSourcePort;
            qreal minDist = 999999.0;
            
            for (const QPointF& port : outputPorts) {
                qreal dist = QLineF(port, oldSourcePort).length();
                if (dist < minDist) {
                    minDist = dist;
                    closestPort = port;
                }
            }
            
            // Update the wire's source port if it changed significantly
            if (QLineF(closestPort, oldSourcePort).length() > 0.1) {
                wire->setSourcePort(closestPort);
                portsChanged = true;
                qDebug() << "🔗 Updated wire source port:" << oldSourcePort << "→" << closestPort;
            }
        }
        
        // Check if this component is the target
        if (wire->getTarget() == component) {
            // Find closest input port to old target port
            QPointF closestPort = oldTargetPort;
            qreal minDist = 999999.0;
            
            for (const QPointF& port : inputPorts) {
                qreal dist = QLineF(port, oldTargetPort).length();
                if (dist < minDist) {
                    minDist = dist;
                    closestPort = port;
                }
            }
            
            // Update the wire's target port if it changed significantly
            if (QLineF(closestPort, oldTargetPort).length() > 0.1) {
                wire->setTargetPort(closestPort);
                portsChanged = true;
                qDebug() << "🔗 Updated wire target port:" << oldTargetPort << "→" << closestPort;
            }
        }
        
        // Save the updated port positions to persistence WITH OLD PORT POSITIONS
        // This ensures the old connection is properly removed before adding the new one
        if (portsChanged) {
            // Add safety check to prevent crashes
            if (wire && wire->getSource() && wire->getTarget()) {
                wire->saveConnectionToPersistence(oldSourcePort, oldTargetPort);
            } else {
                qWarning() << "⚠️ Cannot save wire connection - wire or components are null";
            }
        }
    }
}

