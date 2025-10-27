// ComponentRenderer.cpp
#include "graphics/ready/ComponentRenderer.h"
#include "graphics/ready/ComponentPortManager.h"
#include "graphics/wire/WireGraphicsItem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QFont>
#include <QtMath>

ComponentRenderer::ComponentRenderer()
    : m_defaultBackgroundColor("#F5F5F5")
    , m_defaultBorderColor("#33313B")
    , m_defaultNeonGlowColor("#33313B")
{
}

void ComponentRenderer::setDefaultColors(const QColor& background, const QColor& border, const QColor& neonGlow)
{
    m_defaultBackgroundColor = background;
    m_defaultBorderColor = border;
    m_defaultNeonGlowColor = neonGlow;
}

void ComponentRenderer::drawNeonGlow(QPainter* painter, const QRectF& rect, const QColor& glowColor)
{
    for (int i = 2; i > 0; --i) {
        QColor color = glowColor;
        color.setAlpha(40 / i);
        QPen glowPen(color, 2 + i * 2, Qt::SolidLine);
        painter->setPen(glowPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(rect.adjusted(-i, -i, i, i), 6, 6);
    }
}

void ComponentRenderer::drawComponentBody(QPainter* painter, const QRectF& rect, bool isSelected,
                                         const QColor& backgroundColor, const QColor& borderColor)
{
    if (isSelected) {
        painter->setPen(QPen(borderColor.lighter(180), 4));
        painter->setBrush(backgroundColor.lighter(120));
    } else {
        painter->setPen(QPen(borderColor, 3));
        painter->setBrush(backgroundColor);
    }
    painter->drawRoundedRect(rect, 5, 5);
    
    // // Draw inner highlight for neon effect
    // QRectF innerRect = rect.adjusted(2, 2, -2, -2);
    // painter->setPen(QPen(borderColor.lighter(150), 1));
    // painter->setBrush(Qt::NoBrush);
    // painter->drawRoundedRect(innerRect, 4, 4);
}

void ComponentRenderer::drawComponentName(QPainter* painter, const QRectF& rect, 
                                         const QString& name, const QColor& textColor)
{
    painter->setPen(textColor);
    painter->setFont(QFont("Tajawal", 10, QFont::Bold));
    painter->drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, name);
}

void ComponentRenderer::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget,
                              const QString& name, qreal width, qreal height, bool isSelected,
                              bool hasCustomColor, const QColor& customColor, qreal portRadius)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    qreal offset = portRadius;
    QRectF rect(offset + 2, offset + 2, width - 4, height - 4);
    
    // Determine colors
    QColor backgroundColor, borderColor, neonGlowColor;
    if (hasCustomColor) {
        backgroundColor = customColor;
        borderColor = customColor.lighter(150);
        neonGlowColor = customColor;
    } else {
        backgroundColor = m_defaultBackgroundColor;
        borderColor = m_defaultBorderColor;
        neonGlowColor = m_defaultNeonGlowColor;
    }
    
    // Draw neon glow effect
    drawNeonGlow(painter, rect, neonGlowColor);
    
    // Draw main component body
    drawComponentBody(painter, rect, isSelected, backgroundColor, borderColor);
    
    // Draw component name
    drawComponentName(painter, rect, name, QColor("#33313B"));
}

void ComponentRenderer::drawInputPort(QPainter* painter, const QPointF& port, 
                                     const QColor& portColor, bool isHighlighted, int portRadius)
{
    const int squareSize = portRadius * 2;
    QColor portBackground("#F5F5F5");  // Light gray background
    QColor portBorder("#33313B");      // Teal border
    
    // Draw neon glow if wire is connected
    if (portColor != QColor(180, 180, 180)) {
        for (int i = 1; i > 0; --i) {
            QColor glowColor = portColor;
            glowColor.setAlpha(50 / i);
            painter->setPen(Qt::NoPen);
            painter->setBrush(glowColor);
            QRectF glowRect(port.x() - squareSize/2 - i*2, port.y() - squareSize/2 - i*2, 
                          squareSize + i*4, squareSize + i*4);
            painter->drawRect(glowRect);
        }
    }
    
    // Draw outer glow if highlighted
    if (isHighlighted) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(100, 255, 100, 200));
        QRectF glowRect(port.x() - squareSize/2 - 2, port.y() - squareSize/2 - 2, 
                      squareSize + 4, squareSize + 4);
        painter->drawRect(glowRect);
    }
    
    // Draw main square port
    QRectF portRect(port.x() - squareSize/2, port.y() - squareSize/2, squareSize, squareSize);
    
    if (isHighlighted) {
        painter->setPen(QPen(Qt::green, 2));
        painter->setBrush(QColor(150, 255, 150));
    } else if (portColor != QColor(180, 180, 180)) {
        // Connected port - use custom colors with wire color border
        painter->setPen(QPen(portColor.darker(150), 2));
        painter->setBrush(portBackground);
    } else {
        // Unconnected port - use custom colors
        painter->setPen(QPen(portBorder, 2));
        painter->setBrush(portBackground);
    }
    painter->drawRoundedRect(portRect, 3, 3);
    
    // Draw colored center dot if connected
    if (portColor != QColor(180, 180, 180) && !isHighlighted) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(portColor);
        painter->drawEllipse(portRect.center(), squareSize/6, squareSize/6);
    }
}

void ComponentRenderer::drawOutputPort(QPainter* painter, const QPointF& port, 
                                      const QColor& portColor, bool isHighlighted, int portRadius)
{
    QColor portBackground("#F5F5F5");  // Light gray background
    QColor portBorder("#33313B");      // Teal border

    // Draw neon glow if wire is connected
    if (portColor != QColor(180, 180, 180)) {
        for (int i = 1; i > 0; --i) {
            QColor glowColor = portColor;
            glowColor.setAlpha(50 / i);
            painter->setPen(Qt::NoPen);
            painter->setBrush(glowColor);
            painter->drawEllipse(port, portRadius + i*2, portRadius + i*2);
        }
    }
    
    // Draw outer glow if highlighted
    if (isHighlighted) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(100, 255, 100, 200));
        painter->drawEllipse(port, portRadius + 4, portRadius + 4);
    }
    
    // Draw main circular port
    if (isHighlighted) {
        painter->setPen(QPen(Qt::green, 2));
        painter->setBrush(QColor(150, 255, 150));
    } else if (portColor != QColor(180, 180, 180)) {
        // Connected port - use custom colors with wire color border
        painter->setPen(QPen(portColor.darker(150), 2));
        painter->setBrush(portBackground);
    } else {
        // Unconnected port - use custom colors
        painter->setPen(QPen(portBorder, 2));
        painter->setBrush(portBackground);
    }
    painter->drawEllipse(port, portRadius, portRadius);
    
    // Draw colored center dot if connected
    if (portColor != QColor(180, 180, 180) && !isHighlighted) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(portColor);
        painter->drawEllipse(port, portRadius/3, portRadius/3);
    }
}

void ComponentRenderer::drawPorts(QPainter* painter, const ComponentPortManager* portManager,
                                 const QList<WireGraphicsItem*>& wires, qreal offset)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    const int portRadius = ComponentPortManager::PORT_RADIUS;
    QPointF highlightedPort = portManager->getHighlightedPort();
    
    // Draw input ports (squares)
    QList<QPointF> inputPorts = portManager->getInputPorts();
    for (const QPointF& port : inputPorts) {
        bool isHighlighted = (!highlightedPort.isNull() && 
                             qAbs(port.x() - highlightedPort.x()) < 1 && 
                             qAbs(port.y() - highlightedPort.y()) < 1);
        
        QPointF adjustedPort = port + QPointF(offset, offset);
        QColor portColor = portManager->getPortColor(port, true, wires);
        
        drawInputPort(painter, adjustedPort, portColor, isHighlighted, portRadius);
    }
    
    // Draw output ports (circles)
    QList<QPointF> outputPorts = portManager->getOutputPorts();
    for (const QPointF& port : outputPorts) {
        bool isHighlighted = (!highlightedPort.isNull() && 
                             qAbs(port.x() - highlightedPort.x()) < 1 && 
                             qAbs(port.y() - highlightedPort.y()) < 1);
        
        QPointF adjustedPort = port + QPointF(offset, offset);
        QColor portColor = portManager->getPortColor(port, false, wires);
        
        drawOutputPort(painter, adjustedPort, portColor, isHighlighted, portRadius);
    }
}

