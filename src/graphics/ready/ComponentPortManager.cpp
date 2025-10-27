// ComponentPortManager.cpp
#include "graphics/ready/ComponentPortManager.h"
#include "graphics/wire/WireGraphicsItem.h"
#include "parsers/SvParser.h"
#include <QtMath>
#include <QDebug>

ComponentPortManager::ComponentPortManager(const QString& componentName, qreal width, qreal height)
    : m_componentName(componentName)
    , m_width(width)
    , m_height(height)
    , m_useDynamicPorts(false)
    , m_dynamicInputCount(0)
    , m_dynamicOutputCount(0)
{
}

void ComponentPortManager::updateDimensions(qreal width, qreal height)
{
    m_width = width;
    m_height = height;
}

void ComponentPortManager::updatePortsFromModuleInfo(const ModuleInfo& moduleInfo)
{
    m_useDynamicPorts = true;
    m_dynamicInputCount = moduleInfo.inputs.size();
    m_dynamicOutputCount = moduleInfo.outputs.size();
    
    qDebug() << "✅ Updated ports for" << m_componentName 
             << "| Inputs:" << m_dynamicInputCount 
             << "| Outputs:" << m_dynamicOutputCount;
}

int ComponentPortManager::getNumInputPorts() const
{
    // If using dynamic ports from file, return the dynamic count
    if (m_useDynamicPorts) {
        return m_dynamicInputCount;
    }
    
    // Otherwise fall back to hardcoded defaults
    if (m_componentName == "Transactor") return 2;  // Transactor has 2 inputs (on right side)
    if (m_componentName == "RM") return 1;
    if (m_componentName == "Compare") return 2;
    if (m_componentName == "Driver") return 1;
    if (m_componentName == "Stimuler") return 1;
    if (m_componentName == "Stimuli") return 0;
    if (m_componentName == "RTL") return 1;  // RTL has 1 input
    return 0;
}

int ComponentPortManager::getNumOutputPorts() const
{
    // If using dynamic ports from file, return the dynamic count
    if (m_useDynamicPorts) {
        return m_dynamicOutputCount;
    }
    
    // Otherwise fall back to hardcoded defaults
    if (m_componentName == "Transactor") return 1;  // Transactor has 1 output (on right side)
    if (m_componentName == "RM") return 1;
    if (m_componentName == "Compare") return 0;
    if (m_componentName == "Driver") return 2;
    if (m_componentName == "Stimuler") return 1;
    if (m_componentName == "Stimuli") return 1;
    if (m_componentName == "RTL") return 1;  // RTL has 1 output
    return 0;
}

QList<QPointF> ComponentPortManager::getInputPorts() const
{
    QList<QPointF> ports;
    int numInputs = getNumInputPorts();
    
    // RTL components now use standard port positioning like other components
    
    // Special case for Transactor: inputs on RIGHT side (below output)
    if (m_componentName == "Transactor" && numInputs > 0) {
        // Position inputs at 2/4 and 3/4 height (below the output at 1/4)
        ports.append(QPointF(m_width, m_height * 2.0 / 4.0));  // First input
        ports.append(QPointF(m_width, m_height * 3.0 / 4.0));  // Second input
        return ports;
    }
    
    // Standard case: distribute inputs evenly on the left
    if (numInputs > 0) {
        qreal portSpacing = m_height / (numInputs + 1.0);
        for (int i = 0; i < numInputs; ++i) {
            ports.append(QPointF(0, portSpacing * (i + 1)));
        }
    }
    
    return ports;
}

QList<QPointF> ComponentPortManager::getOutputPorts() const
{
    QList<QPointF> ports;
    int numOutputs = getNumOutputPorts();
    
    // RTL components now use standard port positioning like other components
    
    // Special case for Transactor: output on RIGHT side (at top, 1/4 height)
    if (m_componentName == "Transactor") {
        // Output at 1/4 height (top position)
        ports.append(QPointF(m_width, m_height * 1.0 / 4.0));
        return ports;
    }
    
    // Standard case: outputs on the right
    if (numOutputs > 0) {
        qreal portSpacing = m_height / (numOutputs + 1.0);
        for (int i = 0; i < numOutputs; ++i) {
            ports.append(QPointF(m_width, portSpacing * (i + 1)));
        }
    }
    
    return ports;
}

QPointF ComponentPortManager::getPortAt(const QPointF& pos, bool& isInput) const
{
    // Check input ports
    QList<QPointF> inputPorts = getInputPorts();
    for (const QPointF& port : inputPorts) {
        qreal distance = qSqrt(qPow(pos.x() - port.x(), 2) + qPow(pos.y() - port.y(), 2));
        if (distance < PORT_DETECTION_RADIUS) {
            isInput = true;
            return port;
        }
    }
    
    // Check output ports
    QList<QPointF> outputPorts = getOutputPorts();
    for (const QPointF& port : outputPorts) {
        qreal distance = qSqrt(qPow(pos.x() - port.x(), 2) + qPow(pos.y() - port.y(), 2));
        if (distance < PORT_DETECTION_RADIUS) {
            isInput = false;
            return port;
        }
    }
    
    return QPointF();
}

bool ComponentPortManager::isNearPort(const QPointF& pos) const
{
    bool isInput;
    QPointF port = getPortAt(pos, isInput);
    return !port.isNull();
}

QColor ComponentPortManager::getPortColor(const QPointF& port, bool isInput, 
                                          const QList<WireGraphicsItem*>& wires) const
{
    WireGraphicsItem* wire = getWireAtPort(port, isInput, wires);
    if (wire) {
        return wire->getNeonColor();
    }
    // Return default gray if no wire connected
    return QColor(180, 180, 180);
}

bool ComponentPortManager::isPortConnected(const QPointF& port, bool isInput, 
                                           const QList<WireGraphicsItem*>& wires) const
{
    return getWireAtPort(port, isInput, wires) != nullptr;
}

WireGraphicsItem* ComponentPortManager::getWireAtPort(const QPointF& port, bool isInput, 
                                                      const QList<WireGraphicsItem*>& wires) const
{
    // Find wire connected to this port
    for (WireGraphicsItem* wire : wires) {
        if (isInput) {
            // Check if this is the target port
            if (wire->getTarget() != nullptr) {
                QPointF wirePort = wire->getTargetPort();
                if (qAbs(wirePort.x() - port.x()) < 1 && qAbs(wirePort.y() - port.y()) < 1) {
                    return wire;
                }
            }
        } else {
            // Check if this is the source port
            if (wire->getSource() != nullptr) {
                QPointF wirePort = wire->getSourcePort();
                if (qAbs(wirePort.x() - port.x()) < 1 && qAbs(wirePort.y() - port.y()) < 1) {
                    return wire;
                }
            }
        }
    }
    return nullptr;
}

