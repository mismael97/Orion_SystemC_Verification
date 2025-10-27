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
    , m_useDynamicPorts(true)  // Enable dynamic ports by default
    , m_dynamicInputCount(0)   // Will be set based on component type
    , m_dynamicOutputCount(0)  // Will be set based on component type
{
    // Set specific port counts for each component type
    if (componentName == "Stimuli") {
        m_dynamicInputCount = 0;   // 0 inputs
        m_dynamicOutputCount = 1;  // 1 output
    } else if (componentName == "Stimuler") {
        m_dynamicInputCount = 1;   // 1 input
        m_dynamicOutputCount = 1;  // 1 output
    } else if (componentName == "Driver") {
        m_dynamicInputCount = 1;   // 1 input
        m_dynamicOutputCount = 2;  // 2 outputs
    } else if (componentName == "RM") {
        m_dynamicInputCount = 1;   // 1 input
        m_dynamicOutputCount = 1;  // 1 output
    } else if (componentName == "Transactor") {
        m_dynamicInputCount = 2;   // 2 inputs
        m_dynamicOutputCount = 2;  // 2 outputs
    } else if (componentName == "RTL") {
        m_dynamicInputCount = 1;   // 1 input
        m_dynamicOutputCount = 1;  // 1 output
    } else if (componentName == "Compare") {
        m_dynamicInputCount = 2;   // 2 inputs
        m_dynamicOutputCount = 0;  // 0 outputs
    } else {
        // Default for unknown components
        m_dynamicInputCount = 1;   // 1 input
        m_dynamicOutputCount = 1;  // 1 output
    }
    
    qDebug() << "🔌 ComponentPortManager initialized for" << m_componentName 
             << "| Inputs:" << m_dynamicInputCount 
             << "| Outputs:" << m_dynamicOutputCount;
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
    
    // Otherwise fall back to hardcoded defaults (matching the constructor)
    if (m_componentName == "Stimuli") return 0;     // 0 inputs
    if (m_componentName == "Stimuler") return 1;    // 1 input
    if (m_componentName == "Driver") return 1;      // 1 input
    if (m_componentName == "RM") return 1;          // 1 input
    if (m_componentName == "Transactor") return 2;  // 2 inputs
    if (m_componentName == "RTL") return 1;         // 1 input
    if (m_componentName == "Compare") return 2;     // 2 inputs
    return 1; // Default for unknown components
}

int ComponentPortManager::getNumOutputPorts() const
{
    // If using dynamic ports from file, return the dynamic count
    if (m_useDynamicPorts) {
        return m_dynamicOutputCount;
    }
    
    // Otherwise fall back to hardcoded defaults (matching the constructor)
    if (m_componentName == "Stimuli") return 1;     // 1 output
    if (m_componentName == "Stimuler") return 1;    // 1 output
    if (m_componentName == "Driver") return 2;      // 2 outputs
    if (m_componentName == "RM") return 1;          // 1 output
    if (m_componentName == "Transactor") return 2;  // 2 outputs
    if (m_componentName == "RTL") return 1;         // 1 output
    if (m_componentName == "Compare") return 0;     // 0 outputs
    return 1; // Default for unknown components
}

QList<QPointF> ComponentPortManager::getInputPorts() const
{
    QList<QPointF> ports;
    int numInputs = getNumInputPorts();
    
    qDebug() << "🔌 ComponentPortManager::getInputPorts for" << m_componentName 
             << "| Count:" << numInputs 
             << "| Dynamic:" << m_useDynamicPorts
             << "| Size:" << m_width << "x" << m_height;
    
    // RTL components now use standard port positioning like other components
    
    // All components now follow standard positioning: inputs on LEFT side
    // (Removed special case for Transactor to maintain consistency)
    
    // Standard case: distribute inputs evenly on the left
    if (numInputs > 0) {
        qreal portSpacing = m_height / (numInputs + 1.0);
        for (int i = 0; i < numInputs; ++i) {
            ports.append(QPointF(0, portSpacing * (i + 1)));
        }
        qDebug() << "🔌 Standard input ports:" << ports;
    }
    
    return ports;
}

QList<QPointF> ComponentPortManager::getOutputPorts() const
{
    QList<QPointF> ports;
    int numOutputs = getNumOutputPorts();
    
    qDebug() << "🔌 ComponentPortManager::getOutputPorts for" << m_componentName 
             << "| Count:" << numOutputs 
             << "| Dynamic:" << m_useDynamicPorts
             << "| Size:" << m_width << "x" << m_height;
    
    // RTL components now use standard port positioning like other components
    
    // All components now follow standard positioning: outputs on RIGHT side
    // (Removed special case for Transactor to maintain consistency)
    
    // Standard case: outputs on the right
    if (numOutputs > 0) {
        qreal portSpacing = m_height / (numOutputs + 1.0);
        for (int i = 0; i < numOutputs; ++i) {
            ports.append(QPointF(m_width, portSpacing * (i + 1)));
        }
        qDebug() << "🔌 Standard output ports:" << ports;
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

