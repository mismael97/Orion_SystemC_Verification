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
    qDebug() << "🔗 ComponentWireManager::updateWires - updating" << m_wires.size() << "wires";
    
    for (WireGraphicsItem* wire : m_wires) {
        if (!wire) {
            qWarning() << "⚠️ Found null wire in updateWires";
            continue;
        }
        
        // Safety check - ensure wire is still valid
        if (!wire->getSource() || !wire->getTarget()) {
            qWarning() << "⚠️ Wire has null source or target in updateWires, skipping";
            continue;
        }
        
        try {
            wire->updatePath();
        } catch (const std::exception& e) {
            qWarning() << "⚠️ Exception updating wire path:" << e.what();
        } catch (...) {
            qWarning() << "⚠️ Unknown exception updating wire path";
        }
    }
}

void ComponentWireManager::updateWirePortPositions(ReadyComponentGraphicsItem* component)
{
    if (!component) {
        qWarning() << "⚠️ ComponentWireManager::updateWirePortPositions - component is null";
        return;
    }
    
    // Get the current port positions from the component
    QList<QPointF> inputPorts = component->getInputPorts();
    QList<QPointF> outputPorts = component->getOutputPorts();
    
    qDebug() << "🔗 Updating wire port positions for component:" << component->getName()
             << "| Input ports:" << inputPorts.size()
             << "| Output ports:" << outputPorts.size()
             << "| Connected wires:" << m_wires.size();
    
    // Update each wire's port positions
    for (WireGraphicsItem* wire : m_wires) {
        if (!wire) {
            qWarning() << "⚠️ Found null wire in wire manager";
            continue;
        }
        
        // Safety check - ensure wire is still valid
        if (!wire->getSource() || !wire->getTarget()) {
            qWarning() << "⚠️ Wire has null source or target, skipping";
            continue;
        }
        
        QPointF oldSourcePort = wire->getSourcePort();
        QPointF oldTargetPort = wire->getTargetPort();
        bool portsChanged = false;
        
        // Find the closest current port to the old port position
        // This handles the case where ports have been redistributed during resize
        
        // Check if this component is the source
        if (wire->getSource() == component) {
            // Safety check - ensure we have output ports
            if (outputPorts.isEmpty()) {
                qWarning() << "⚠️ No output ports available for wire source update";
                continue;
            }
            
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
            // Safety check - ensure we have input ports
            if (inputPorts.isEmpty()) {
                qWarning() << "⚠️ No input ports available for wire target update";
                continue;
            }
            
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
                try {
                    wire->saveConnectionToPersistence(oldSourcePort, oldTargetPort);
                    qDebug() << "✅ Wire connection saved to persistence successfully";
                } catch (const std::exception& e) {
                    qWarning() << "⚠️ Exception saving wire connection:" << e.what();
                } catch (...) {
                    qWarning() << "⚠️ Unknown exception saving wire connection";
                }
            } else {
                qWarning() << "⚠️ Cannot save wire connection - wire or components are null";
            }
        }
    }
}

