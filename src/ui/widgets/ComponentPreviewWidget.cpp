// ComponentPreviewWidget.cpp
#include "ui/widgets/ComponentPreviewWidget.h"
#include "graphics/ready/ComponentRenderer.h"
#include "graphics/ready/ComponentPortManager.h"
#include <QPainter>
#include <QTimer>
#include <QApplication>
#include <QStyleOptionGraphicsItem>
#include <QDebug>

ComponentPreviewWidget::ComponentPreviewWidget(const QString& componentName, const QString& description, QWidget* parent)
    : QWidget(parent)
    , m_componentName(componentName)
    , m_description(description)
    , m_width(120)
    , m_height(80)
    , m_fadeAlpha(0)
{
    // Set up the widget properties
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    
    // Initialize renderer and port manager
    m_renderer = std::make_unique<ComponentRenderer>();
    setupComponent();
    
    // Set up fade animation
    m_fadeTimer = new QTimer(this);
    m_fadeTimer->setInterval(FADE_DURATION / FADE_STEPS);
    connect(m_fadeTimer, &QTimer::timeout, this, [this]() {
        m_fadeAlpha += 255 / FADE_STEPS;
        if (m_fadeAlpha >= 255) {
            m_fadeAlpha = 255;
            m_fadeTimer->stop();
        }
        update();
    });
    
    // Set optimal size
    QSize optimalSize = calculateOptimalSize();
    setFixedSize(optimalSize);
    
    // Set up visual properties based on component type
    setupVisualProperties();
}

ComponentPreviewWidget::~ComponentPreviewWidget()
{
}

void ComponentPreviewWidget::setComponentName(const QString& name)
{
    if (m_componentName != name) {
        m_componentName = name;
        setupComponent();
        setupVisualProperties();
        QSize optimalSize = calculateOptimalSize();
        setFixedSize(optimalSize);
        update();
    }
}

void ComponentPreviewWidget::setDescription(const QString& description)
{
    if (m_description != description) {
        m_description = description;
        QSize optimalSize = calculateOptimalSize();
        setFixedSize(optimalSize);
        update();
    }
}

void ComponentPreviewWidget::setupComponent()
{
    // Set default size based on component type
    if (m_componentName == "Transactor") {
        // Transactor is taller - much more height than width
        m_width = 100;
        m_height = 200;
    } else {
        // Default size for other components
        m_width = 120;
        m_height = 80;
    }
    
    // Create port manager with the component dimensions
    m_portManager = std::make_unique<ComponentPortManager>(m_componentName, m_width, m_height);
}

void ComponentPreviewWidget::setupVisualProperties()
{
    // Define colors for different component types
    if (m_componentName == "Transactor") {
        m_backgroundColor = QColor("#4A90E2");
        m_borderColor = QColor("#357ABD");
        m_neonGlowColor = QColor("#4A90E2");
    } else if (m_componentName == "RM") {
        m_backgroundColor = QColor("#7ED321");
        m_borderColor = QColor("#5BA817");
        m_neonGlowColor = QColor("#7ED321");
    } else if (m_componentName == "Compare") {
        m_backgroundColor = QColor("#F5A623");
        m_borderColor = QColor("#D68910");
        m_neonGlowColor = QColor("#F5A623");
    } else if (m_componentName == "Driver") {
        m_backgroundColor = QColor("#D0021B");
        m_borderColor = QColor("#A0151A");
        m_neonGlowColor = QColor("#D0021B");
    } else if (m_componentName == "Stimuli") {
        m_backgroundColor = QColor("#50E3C2");
        m_borderColor = QColor("#3BB5A0");
        m_neonGlowColor = QColor("#50E3C2");
    } else if (m_componentName == "RTL") {
        m_backgroundColor = QColor("#B8E986");
        m_borderColor = QColor("#8BC34A");
        m_neonGlowColor = QColor("#B8E986");
    } else {
        m_backgroundColor = QColor("#637AB9");
        m_borderColor = QColor("#4A5A8A");
        m_neonGlowColor = QColor("#637AB9");
    }
    
    // Set renderer colors
    m_renderer->setDefaultColors(m_backgroundColor, m_borderColor, m_neonGlowColor);
}

void ComponentPreviewWidget::updateComponentSize()
{
    if (m_portManager) {
        m_portManager->updateDimensions(m_width, m_height);
    }
}

QSize ComponentPreviewWidget::calculateOptimalSize() const
{
    // Calculate size needed for component + ports + padding
    qreal portRadius = ComponentPortManager::PORT_RADIUS;
    qreal padding = 20; // Extra padding around the component
    
    qreal componentWidth = m_width + (portRadius * 2) + padding;
    qreal componentHeight = m_height + (portRadius * 2) + padding;
    
    // Add space for description if it exists
    qreal totalHeight = componentHeight;
    if (!m_description.isEmpty()) {
        // Add space for description text (2 lines max, with padding)
        QFontMetrics fontMetrics(QFont("Tajawal", 10));
        int descriptionHeight = fontMetrics.height() * 2 + 20; // 2 lines + padding
        totalHeight += descriptionHeight;
    }
    
    return QSize(static_cast<int>(componentWidth), static_cast<int>(totalHeight));
}

void ComponentPreviewWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Set fade alpha
    painter.setOpacity(m_fadeAlpha / 255.0);
    
    // Calculate component position (centered in widget)
    qreal portRadius = ComponentPortManager::PORT_RADIUS;
    QSize optimalSize = calculateOptimalSize();
    qreal offsetX = (optimalSize.width() - m_width - portRadius * 2) / 2;
    qreal offsetY = (optimalSize.height() - m_height - portRadius * 2) / 2;
    
    // Draw drop shadow
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 30));
    painter.drawRoundedRect(QRectF(offsetX + 3, offsetY + 3, m_width + portRadius * 2, m_height + portRadius * 2), 8, 8);
    
    // Draw component using the renderer
    QStyleOptionGraphicsItem option;
    m_renderer->paint(&painter, &option, nullptr, m_componentName, m_width, m_height, 
                     false, true, m_backgroundColor, portRadius);
    
    // Draw ports
    m_renderer->drawPorts(&painter, m_portManager.get(), QList<class WireGraphicsItem*>(), portRadius);
    
    // Draw description below the component if it exists
    if (!m_description.isEmpty()) {
        painter.setFont(QFont("Tajawal", 10));
        painter.setPen(QColor("#666666"));
        
        // Calculate description area
        QRectF descriptionRect(offsetX, offsetY + m_height + portRadius * 2 + 10, 
                              m_width + portRadius * 2, 40);
        
        // Draw description background
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 200));
        painter.drawRoundedRect(descriptionRect.adjusted(-5, -5, 5, 5), 5, 5);
        
        // Draw description text
        painter.setPen(QColor("#333333"));
        painter.drawText(descriptionRect, Qt::AlignCenter | Qt::TextWordWrap, m_description);
    }
}

void ComponentPreviewWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    
    // Start fade-in animation
    m_fadeAlpha = 0;
    m_fadeTimer->start();
}

void ComponentPreviewWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    
    // Stop animation
    m_fadeTimer->stop();
    m_fadeAlpha = 0;
}
