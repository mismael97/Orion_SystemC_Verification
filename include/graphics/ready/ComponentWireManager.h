// ComponentWireManager.h
#ifndef COMPONENTWIREMANAGER_H
#define COMPONENTWIREMANAGER_H

#include <QList>

class WireGraphicsItem;

/**
 * @brief Manages wire connections for ready components
 * 
 * This class handles:
 * - Wire addition and removal
 * - Wire updates and tracking
 */
class ComponentWireManager
{
public:
    ComponentWireManager();
    
    // Wire management
    void addWire(WireGraphicsItem* wire);
    void removeWire(WireGraphicsItem* wire);
    QList<WireGraphicsItem*> getWires() const { return m_wires; }
    void updateWires();
    void updateWirePortPositions(class ReadyComponentGraphicsItem* component);
    void clearWires() { m_wires.clear(); }
    
private:
    QList<WireGraphicsItem*> m_wires;
};

#endif // COMPONENTWIREMANAGER_H

