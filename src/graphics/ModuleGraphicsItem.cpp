// ModuleGraphicsItem.cpp
#include "graphics/ModuleGraphicsItem.h"
#include "graphics/wire/WireGraphicsItem.h"
#include "graphics/ReadyComponentGraphicsItem.h"
#include "graphics/ready/ComponentPortManager.h"
#include "utils/PersistenceManager.h"
#include "ui/widgets/PortEditorDialog.h"
#include <QPainter>
#include <QPushButton>
#include <QFontMetrics>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QLabel>
#include <QDialog>
#include <QMenu>
#include <QtMath>

const int ModuleGraphicsItem::PORT_RADIUS = 8;
const int ModuleGraphicsItem::PORT_SPACING = 24;
const int ModuleGraphicsItem::LABEL_HEIGHT = 24;
const int ModuleGraphicsItem::PADDING = 12;

// RTL Detail Window class
class RTLDetailWindow : public QDialog {
public:
    RTLDetailWindow(const ModuleInfo& info, QWidget* parent = nullptr)
        : QDialog(parent), m_info(info)
    {
        setWindowTitle("RTL View - " + info.name);
        setMinimumSize(600, 400);

        QVBoxLayout* layout = new QVBoxLayout(this);

        // Title
        QLabel* titleLabel = new QLabel("RTL Implementation: " + info.name);
        titleLabel->setStyleSheet("font-size: 16pt; font-weight: bold; margin: 10px; font-family: Tajawal;");
        layout->addWidget(titleLabel);

        // Inputs section
        if (!info.inputs.isEmpty()) {
            QLabel* inputsLabel = new QLabel("Inputs:");
            inputsLabel->setStyleSheet("font-size: 12pt; font-weight: bold; margin: 5px; font-family: Tajawal;");
            layout->addWidget(inputsLabel);

            for (const Port& input : info.inputs) {
                QString text = input.name;
                if (!input.width.isEmpty()) {
                    text += " " + input.width;
                }
                QLabel* inputLabel = new QLabel("  • " + text);
                inputLabel->setStyleSheet("margin: 2px; font-family: Tajawal;");
                layout->addWidget(inputLabel);
            }
        }

        // Outputs section
        if (!info.outputs.isEmpty()) {
            QLabel* outputsLabel = new QLabel("Outputs:");
            outputsLabel->setStyleSheet("font-size: 12pt; font-weight: bold; margin: 5px; font-family: Tajawal;");
            layout->addWidget(outputsLabel);

            for (const Port& output : info.outputs) {
                QString text = output.name;
                if (!output.width.isEmpty()) {
                    text += " " + output.width;
                }
                QLabel* outputLabel = new QLabel("  • " + text);
                outputLabel->setStyleSheet("margin: 2px; font-family: Tajawal;");
                layout->addWidget(outputLabel);
            }
        }

        // Add stretch to push content to top
        layout->addStretch();

        // Close button
        QPushButton* closeButton = new QPushButton("Close");
        closeButton->setStyleSheet("font-family: Tajawal;");
        connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
        layout->addWidget(closeButton);
    }

private:
    ModuleInfo m_info;
};

ModuleGraphicsItem::ModuleGraphicsItem(const ModuleInfo& info, QGraphicsItem *parent)
    : ReadyComponentGraphicsItem(info.name, parent), 
      m_info(info), 
      m_isRTLView(true)  // Start with RTL view
{
    // Base class already sets flags, but we can override if needed
    // Enable double click
    setAcceptDrops(false);
}

QRectF ModuleGraphicsItem::boundingRect() const
{
    if (m_isRTLView) {
        // Extended bounding rect to include TLM ports that stick out
        const int offset = TLM_PORT_RADIUS;
        return QRectF(-offset, -offset, 
                      120 + offset * 3 + 4, 
                      80 + offset * 2);
    } else {
        // Original detailed view
        int maxPorts = qMax(m_info.inputs.size(), m_info.outputs.size());
        int bodyHeight = maxPorts > 0 ? LABEL_HEIGHT + PADDING + maxPorts * PORT_SPACING - PORT_SPACING / 2 : LABEL_HEIGHT + PADDING;
        int extraSpace = 4;
        return QRectF(0, 0, 200 + extraSpace, bodyHeight + extraSpace);
    }
}

void ModuleGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (m_isRTLView) {
        // Offset for extended bounding rect
        const qreal offset = TLM_PORT_RADIUS;
        
        // Draw simple RTL rectangle
        QRectF bodyRect(offset, offset, 120, 80);

        // Fill and border
        if (isSelected()) {
            painter->setPen(QPen(Qt::blue, 2));
            painter->setBrush(QColor(240, 240, 255)); // Light blue when selected
        } else {
            painter->setPen(QPen(Qt::black, 1));
            painter->setBrush(Qt::white);
        }

        // Base color
        QColor baseColor("#d78fee");
        QColor fillColor = m_hovered ? baseColor.darker(140) : baseColor; // darker on hover
        QColor borderColor = m_hovered ? fillColor.darker(200) : fillColor.darker(150); // more pronounced border on hover

        // Enhanced visual feedback for hover
        if (m_hovered) {
            // Add a subtle shadow/glow effect when hovered
            painter->setPen(QPen(borderColor, 3));
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect(bodyRect.adjusted(-1, -1, 1, 1), 9, 9);
        }

        // Pen + Brush
        painter->setPen(QPen(borderColor, isSelected() ? 2 : (m_hovered ? 2 : 1)));
        painter->setBrush(fillColor);

        // Rounded rectangle - reduced rounding
        painter->drawRoundedRect(bodyRect, 5, 5); // Reduced rounding

        // "RTL" title with enhanced visibility on hover
        painter->setPen(m_hovered ? Qt::white : Qt::black);
        painter->setFont(QFont("Tajawal", 10, QFont::Bold));
        painter->drawText(bodyRect, Qt::AlignCenter, "RTL");

        // Module name below with enhanced visibility on hover
        painter->setPen(m_hovered ? Qt::white : Qt::black);
        painter->setFont(QFont("Tajawal", 8));
        QRectF nameRect(offset, offset + 50, 120, 20);
        painter->drawText(nameRect, Qt::AlignCenter, m_info.name);
        
        // Draw TLM ports
        drawTLMPorts(painter);

    } else {
        // Original detailed view implementation
        QRectF bodyRect(PADDING, LABEL_HEIGHT, boundingRect().width() - 2*PADDING, boundingRect().height() - LABEL_HEIGHT - 10);
        painter->setPen(Qt::black);
        painter->setBrush(Qt::white);
        painter->drawRoundedRect(bodyRect, 10, 10);

        QRectF labelRect(0, 0, boundingRect().width(), LABEL_HEIGHT);
        painter->setPen(Qt::black);
        painter->setBrush(Qt::lightGray);
        painter->drawRoundedRect(labelRect, 5, 5);
        painter->setFont(QFont("Tajawal", 9, QFont::Bold));
        painter->drawText(labelRect, Qt::AlignCenter, m_info.name);

        QFont portFont("Tajawal", 8);
        painter->setFont(portFont);
        QFontMetrics fm(portFont);

        // Inputs
        for (int i = 0; i < m_info.inputs.size(); ++i) {
            int y = LABEL_HEIGHT + PADDING + i * PORT_SPACING;
            QPointF center(PADDING, y);

            bool isHovered = (m_isHovering && m_isInputHovered && m_hoveredPortIndex == i);
            int radius = isHovered ? PORT_RADIUS + 4 : PORT_RADIUS;
            QColor fillColor = isHovered ? Qt::green : Qt::white;

            painter->setPen(Qt::black);
            painter->setBrush(fillColor);
            painter->drawEllipse(center, radius, radius);

            QString displayText = isHovered
                                      ? (m_info.inputs[i].width.isEmpty() ? "[0:0]" : m_info.inputs[i].width)
                                      : (m_info.inputs[i].name.length() > 10 ? m_info.inputs[i].name.left(7) + "..." : m_info.inputs[i].name);

            if (!displayText.isEmpty()) {
                painter->setPen(Qt::black);
                painter->drawText(center.x() + radius + 5, y + 5, displayText);
            }
        }

        // Outputs
        for (int i = 0; i < m_info.outputs.size(); ++i) {
            int y = LABEL_HEIGHT + PADDING + i * PORT_SPACING;
            QPointF center(boundingRect().width() - PADDING, y);

            bool isHovered = (m_isHovering && !m_isInputHovered && m_hoveredPortIndex == i);
            int radius = isHovered ? PORT_RADIUS + 4 : PORT_RADIUS;
            QColor fillColor = isHovered ? Qt::red : Qt::white;

            painter->setPen(Qt::black);
            painter->setBrush(fillColor);
            painter->drawEllipse(center, radius, radius);

            QString displayText = isHovered
                                      ? (m_info.outputs[i].width.isEmpty() ? "[0:0]" : m_info.outputs[i].width)
                                      : (m_info.outputs[i].name.length() > 10 ? m_info.outputs[i].name.left(7) + "..." : m_info.outputs[i].name);

            if (!displayText.isEmpty()) {
                int textWidth = fm.horizontalAdvance(displayText);
                painter->setPen(Qt::black);
                painter->drawText(center.x() - radius - 5 - textWidth, y + 5, displayText);
            }
        }
    }
}

// void ModuleGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
// {
//     if (event->button() == Qt::LeftButton) {
//         // Open RTL detail window
//         RTLDetailWindow* detailWindow = new RTLDetailWindow(m_info);
//         detailWindow->setAttribute(Qt::WA_DeleteOnClose);
//         detailWindow->exec(); // Use exec() for modal, show() for non-modal

//         event->accept();
//         return;
//     }
//     QGraphicsItem::mouseDoubleClickEvent(event);
// }

void ModuleGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_isRTLView) {
        // Handle hover for RTL view
        m_hovered = true;
        update();
    } else {
        // Handle hover for detailed view
        QPointF pos = event->pos();
        bool isInput;
        QPointF port = getPortAt(pos, isInput);
        
        if (!port.isNull()) {
            // Find the port index
            if (isInput) {
                QList<QPointF> inputPorts = getInputPorts();
                for (int i = 0; i < inputPorts.size(); ++i) {
                    if (QLineF(inputPorts[i], port).length() < 1) {
                        m_hoveredPortIndex = i;
                        m_isInputHovered = true;
                        m_isHovering = true;
                        break;
                    }
                }
            } else {
                QList<QPointF> outputPorts = getOutputPorts();
                for (int i = 0; i < outputPorts.size(); ++i) {
                    if (QLineF(outputPorts[i], port).length() < 1) {
                        m_hoveredPortIndex = i;
                        m_isInputHovered = false;
                        m_isHovering = true;
                        break;
                    }
                }
            }
            update();
        }
    }

    QGraphicsItem::hoverEnterEvent(event);
}

void ModuleGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_isRTLView) {
        // Handle hover leave for RTL view
        m_hovered = false;
        update();
    } else {
        m_isHovering = false;
        m_hoveredPortIndex = -1;
        update();
    }
    QGraphicsItem::hoverLeaveEvent(event);
}

// Optional: Add method to toggle between RTL and detailed view
void ModuleGraphicsItem::setRTLView(bool enabled)
{
    if (m_isRTLView != enabled) {
        m_isRTLView = enabled;
        update();
        prepareGeometryChange();
    }
}

bool ModuleGraphicsItem::isRTLView() const
{
    return m_isRTLView;
}

// TLM Port implementations
QList<QPointF> ModuleGraphicsItem::getInputPorts() const
{
    QList<QPointF> ports;
    if (m_isRTLView) {
        // 1 TLM input port on the left side (centered vertically)
        ports.append(QPointF(0, 40));
    } else {
        // Detailed view - show all input ports
        for (int i = 0; i < m_info.inputs.size(); ++i) {
            int y = LABEL_HEIGHT + PADDING + i * PORT_SPACING;
            QPointF center(PADDING, y);
            ports.append(center);
        }
    }
    return ports;
}

QList<QPointF> ModuleGraphicsItem::getOutputPorts() const
{
    QList<QPointF> ports;
    if (m_isRTLView) {
        // 1 TLM output port on the right side (centered vertically)
        ports.append(QPointF(120, 40));
    } else {
        // Detailed view - show all output ports
        for (int i = 0; i < m_info.outputs.size(); ++i) {
            int y = LABEL_HEIGHT + PADDING + i * PORT_SPACING;
            QPointF center(boundingRect().width() - PADDING, y);
            ports.append(center);
        }
    }
    return ports;
}

QPointF ModuleGraphicsItem::getPortAt(const QPointF& pos, bool& isInput) const
{
    // Check input ports
    QList<QPointF> inputPorts = getInputPorts();
    for (const QPointF& port : inputPorts) {
        qreal detectionRadius = m_isRTLView ? TLM_PORT_DETECTION_RADIUS : PORT_RADIUS + 5;
        qreal distance = qSqrt(qPow(pos.x() - port.x(), 2) + qPow(pos.y() - port.y(), 2));
        if (distance < detectionRadius) {
            isInput = true;
            return port;
        }
    }
    
    // Check output ports
    QList<QPointF> outputPorts = getOutputPorts();
    for (const QPointF& port : outputPorts) {
        qreal detectionRadius = m_isRTLView ? TLM_PORT_DETECTION_RADIUS : PORT_RADIUS + 5;
        qreal distance = qSqrt(qPow(pos.x() - port.x(), 2) + qPow(pos.y() - port.y(), 2));
        if (distance < detectionRadius) {
            isInput = false;
            return port;
        }
    }
    
    return QPointF();
}

bool ModuleGraphicsItem::isNearPort(const QPointF& pos) const
{
    bool isInput;
    QPointF port = getPortAt(pos, isInput);
    return !port.isNull();
}

void ModuleGraphicsItem::updateWires()
{
    // Call base class implementation
    ReadyComponentGraphicsItem::updateWires();
}

QColor ModuleGraphicsItem::getPortColor(const QPointF& port, bool isInput) const
{
    // Call base class implementation
    return ReadyComponentGraphicsItem::getPortColor(port, isInput);
}

bool ModuleGraphicsItem::isPortConnected(const QPointF& port, bool isInput) const
{
    return ReadyComponentGraphicsItem::isPortConnected(port, isInput);
}

WireGraphicsItem* ModuleGraphicsItem::getWireAtPort(const QPointF& port, bool isInput) const
{
    return ReadyComponentGraphicsItem::getWireAtPort(port, isInput);
}

void ModuleGraphicsItem::drawTLMPorts(QPainter* painter)
{
    if (!m_isRTLView) {
        return;
    }
    
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    const int squareSize = TLM_PORT_RADIUS * 2;
    qreal offset = TLM_PORT_RADIUS;
    
    // Draw input port as rounded square on the left side
    QList<QPointF> inputPorts = getInputPorts();
    QPointF highlightedPort = m_portManager->getHighlightedPort();
    for (const QPointF& port : inputPorts) {
        bool isHighlighted = (!highlightedPort.isNull() && 
                             qAbs(port.x() - highlightedPort.x()) < 1 && 
                             qAbs(port.y() - highlightedPort.y()) < 1);
        
        bool isConnected = isPortConnected(port, true);
        QPointF adjustedPort = port + QPointF(offset, offset);
        
        // Draw green glow if highlighted
        if (isHighlighted) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(100, 255, 100, 150));
            QRectF glowRect(adjustedPort.x() - squareSize/2 - 3, adjustedPort.y() - squareSize/2 - 3, 
                          squareSize + 6, squareSize + 6);
            painter->drawRoundedRect(glowRect, 5, 5);
        }
        
        // Draw main square port with custom colors
        QRectF portRect(adjustedPort.x() - squareSize/2, adjustedPort.y() - squareSize/2, 
                       squareSize, squareSize);
        painter->setPen(QPen(QColor("#229799"), 2));  // Teal border
        painter->setBrush(QColor("#F5F5F5"));          // Light gray background
        painter->drawRoundedRect(portRect, 4, 4); // Rounded corners
    }
    
    // Draw output port as rounded circle on the right side
    QList<QPointF> outputPorts = getOutputPorts();
    for (const QPointF& port : outputPorts) {
        bool isHighlighted = (!highlightedPort.isNull() && 
                             qAbs(port.x() - highlightedPort.x()) < 1 && 
                             qAbs(port.y() - highlightedPort.y()) < 1);
        
        bool isConnected = isPortConnected(port, false);
        QPointF adjustedPort = port + QPointF(offset, offset);
        
        // Draw green glow if highlighted
        if (isHighlighted) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(100, 255, 100, 150));
            painter->drawEllipse(adjustedPort, TLM_PORT_RADIUS + 3, TLM_PORT_RADIUS + 3);
        }
        
        // Draw main circular port with custom colors
        painter->setPen(QPen(QColor("#229799"), 2));  // Teal border
        painter->setBrush(QColor("#F5F5F5"));          // Light gray background
        painter->drawEllipse(adjustedPort, TLM_PORT_RADIUS, TLM_PORT_RADIUS);
    }
}

void ModuleGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    if (m_isRTLView) {
        QPointF adjustedPos = event->pos() - QPointF(TLM_PORT_RADIUS, TLM_PORT_RADIUS);
        if (isNearPort(adjustedPos)) {
            setCursor(Qt::PointingHandCursor);
            setToolTip("Click and drag to connect");
        } else {
            setCursor(Qt::ArrowCursor);
            setToolTip("");
        }
    }
    QGraphicsItem::hoverMoveEvent(event);
}

void ModuleGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_isRTLView && event->button() == Qt::LeftButton) {
        QPointF adjustedPos = event->pos() - QPointF(TLM_PORT_RADIUS, TLM_PORT_RADIUS);
        
        // Check if clicking on a TLM port - let the scene handle it
        if (isNearPort(adjustedPos)) {
            event->accept();
            return;
        }
    }
    QGraphicsItem::mousePressEvent(event);
}

QVariant ModuleGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged) {
        updateWires();
        
        // Update position in persistence (for RTL modules)
        if (m_isRTLView) {
            PersistenceManager& pm = PersistenceManager::instance();
            QString moduleName = pm.getRTLModuleName(this);
            if (!moduleName.isEmpty()) {
                pm.updateRTLModulePosition(moduleName, pos());
            }
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void ModuleGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    
    // Style the menu with Tajawal font for Arabic support
    menu.setStyleSheet("QMenu { font-family: 'Tajawal'; font-size: 10pt; }"
                      "QMenu::item:selected { background-color: #637AB9; }");
    
    // Add "Toggle View" action
    QAction* toggleViewAction = menu.addAction(m_isRTLView ? "Show Detailed View" : "Show RTL View");
    toggleViewAction->setToolTip(m_isRTLView ? "Switch to detailed port view" : "Switch to compact RTL view");
    
    // Add separator
    menu.addSeparator();
    
    // Add "Properties" action
    QAction* propertiesAction = menu.addAction("Properties");
    propertiesAction->setToolTip("View module properties and information");
    
    // Show menu at cursor position
    QAction* selectedAction = menu.exec(event->screenPos());
    
    if (selectedAction == toggleViewAction) {
        // Toggle between RTL and detailed view
        setRTLView(!m_isRTLView);
    } else if (selectedAction == propertiesAction) {
        // Open RTL detail window
        RTLDetailWindow* detailWindow = new RTLDetailWindow(m_info);
        detailWindow->setAttribute(Qt::WA_DeleteOnClose);
        detailWindow->show();
    }
    
    event->accept();
}

void ModuleGraphicsItem::updateModuleInfo(const ModuleInfo& newInfo)
{
    // Update the module info
    m_info = newInfo;
    
    // Update the port manager with new port configuration
    if (m_portManager) {
        m_portManager->updatePortsFromModuleInfo(newInfo);
    }
    
    // Update the component name if it changed
    if (getName() != newInfo.name) {
        setName(newInfo.name);
    }
    
    // Calculate new component size based on port count (for detailed view)
    if (!m_isRTLView) {
        int maxPorts = qMax(newInfo.inputs.size(), newInfo.outputs.size());
        qreal minHeight = qMax(40.0, maxPorts * 20.0 + 20.0); // 20px per port + 20px padding
        qreal minWidth = qMax(50.0, 80.0); // Minimum width
        
        // Get current size using public method
        QSizeF currentSize = getSize();
        qreal currentWidth = currentSize.width();
        qreal currentHeight = currentSize.height();
        
        // Resize component if needed to accommodate ports
        if (currentHeight < minHeight || currentWidth < minWidth) {
            qreal newWidth = qMax(currentWidth, minWidth);
            qreal newHeight = qMax(currentHeight, minHeight);
            
            qDebug() << "📏 Resizing module to accommodate ports:" 
                     << "| Old size:" << currentWidth << "x" << currentHeight
                     << "| New size:" << newWidth << "x" << newHeight;
            
            setSize(newWidth, newHeight);
        }
    }
    
    // Update persistence with new port configuration
    PersistenceManager& pm = PersistenceManager::instance();
    QString moduleName = pm.getRTLModuleName(this);
    if (!moduleName.isEmpty()) {
        pm.updateRTLModulePorts(moduleName, newInfo.inputs, newInfo.outputs);
    }
    
    // Force a geometry update to recalculate bounding rect
    prepareGeometryChange();
    
    // Update the display
    update();
    
    // Update any connected wires
    updateWires();
    
    qDebug() << "Module info updated for:" << newInfo.name 
             << "| Inputs:" << newInfo.inputs.size() 
             << "| Outputs:" << newInfo.outputs.size();
}

