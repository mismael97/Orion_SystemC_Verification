// ComponentRenderer.h
#ifndef COMPONENTRENDERER_H
#define COMPONENTRENDERER_H

#include <QColor>
#include <QString>
#include <QRectF>
#include <QList>
#include <QPointF>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class WireGraphicsItem;
class ComponentPortManager;

/**
 * @brief Handles rendering of ready components
 * 
 * This class handles:
 * - Component body rendering with neon effects
 * - Port rendering
 * - Text rendering
 */
class ComponentRenderer
{
public:
    ComponentRenderer();
    
    // Main rendering
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget,
               const QString& name, qreal width, qreal height, bool isSelected,
               bool hasCustomColor, const QColor& customColor, qreal portRadius);
    
    // Port rendering
    void drawPorts(QPainter* painter, const ComponentPortManager* portManager,
                   const QList<WireGraphicsItem*>& wires, qreal offset);
    
    // Connect icon rendering
    void drawConnectIcon(QPainter* painter, qreal width, qreal height, qreal portRadius,
                        bool isConnected, const QPointF& iconPos = QPointF());
    
    // Color management
    void setDefaultColors(const QColor& background, const QColor& border, const QColor& neonGlow);
    
private:
    QColor m_defaultBackgroundColor;
    QColor m_defaultBorderColor;
    QColor m_defaultNeonGlowColor;
    
    // Helper methods
    void drawNeonGlow(QPainter* painter, const QRectF& rect, const QColor& glowColor);
    void drawComponentBody(QPainter* painter, const QRectF& rect, bool isSelected,
                          const QColor& backgroundColor, const QColor& borderColor);
    void drawComponentName(QPainter* painter, const QRectF& rect, const QString& name, 
                          const QColor& textColor);
    void drawInputPort(QPainter* painter, const QPointF& port, const QColor& portColor, 
                      bool isHighlighted, int portRadius);
    void drawOutputPort(QPainter* painter, const QPointF& port, const QColor& portColor, 
                       bool isHighlighted, int portRadius);
};

#endif // COMPONENTRENDERER_H

