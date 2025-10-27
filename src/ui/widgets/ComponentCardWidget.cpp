// ComponentCardWidget.cpp
#include "ui/widgets/ComponentCardWidget.h"
#include "ui/widgets/ComponentPreviewWidget.h"
#include "ui/widgets/dragdropgraphicsview.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QTimer>
#include <QScreen>
#include <QDebug>

ComponentCardWidget::ComponentCardWidget(const QString& componentName, 
                                       const QString& description,
                                       QWidget* parent)
    : QWidget(parent)
    , m_componentName(componentName)
    , m_description(description)
{
    setupUI();
    createComponentIcon();
    
    // Set up visual properties
    m_backgroundColor = QColor("#FFFFFF");
    m_borderColor = QColor("#E0E0E0");
    m_hoverColor = QColor("#F5F5F5");
    
    // Enable mouse tracking for hover effects
    setMouseTracking(true);
    
    // Set fixed size
    setFixedSize(m_cardWidth, m_cardHeight);
    
    // Set up drag and drop
    setAcceptDrops(false);
    
    // Set up hover timer for preview
    m_hoverTimer = new QTimer(this);
    m_hoverTimer->setSingleShot(true);
    m_hoverTimer->setInterval(HOVER_DELAY);
    connect(m_hoverTimer, &QTimer::timeout, this, &ComponentCardWidget::showPreview);
}

void ComponentCardWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(12);
    
    // Icon label
    m_iconLabel = new QLabel(this);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(m_iconSize, m_iconSize);
    m_mainLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    
    // Name label with fixed padding from preview
    m_nameLabel = new QLabel(m_componentName, this);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setFont(QFont("Tajawal", 13, QFont::Bold));
    m_nameLabel->setStyleSheet("color: #333333; padding: 8px;");
    m_nameLabel->setWordWrap(true);
    m_mainLayout->addWidget(m_nameLabel);
    
    // Remove description label from card - it will be shown in preview
    // m_descriptionLabel is kept for compatibility but not added to layout
    
    // Add stretch to push everything to the top
    m_mainLayout->addStretch();
}

void ComponentCardWidget::createComponentIcon()
{
    // Create a larger, more detailed icon
    QPixmap pixmap(m_iconSize, m_iconSize);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Define colors for different component types
    QColor backgroundColor;
    QColor borderColor;
    QColor textColor = Qt::white;
    
    if (m_componentName == "Transactor") {
        backgroundColor = QColor("#4A90E2");
        borderColor = QColor("#357ABD");
    } else if (m_componentName == "RM") {
        backgroundColor = QColor("#7ED321");
        borderColor = QColor("#5BA817");
    } else if (m_componentName == "Compare") {
        backgroundColor = QColor("#F5A623");
        borderColor = QColor("#D68910");
    } else if (m_componentName == "Driver") {
        backgroundColor = QColor("#D0021B");
        borderColor = QColor("#A0151A");
    } else if (m_componentName == "Stimuler") {
        backgroundColor = QColor("#9013FE");
        borderColor = QColor("#6A0DAD");
    } else if (m_componentName == "Stimuli") {
        backgroundColor = QColor("#50E3C2");
        borderColor = QColor("#3BB5A0");
        textColor = Qt::black;
    } else if (m_componentName == "RTL") {
        backgroundColor = QColor("#B8E986");
        borderColor = QColor("#8BC34A");
        textColor = Qt::black;
    } else {
        backgroundColor = QColor("#637AB9");
        borderColor = QColor("#4A5A8A");
    }
    
    // Draw rounded rectangle background
    QRectF rect(5, 5, m_iconSize - 10, m_iconSize - 10);
    painter.setPen(QPen(borderColor, 3));
    painter.setBrush(backgroundColor);
    painter.drawRoundedRect(rect, 8, 8);
    
    // Draw component-specific icon or symbol
    painter.setPen(QPen(textColor, 2));
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    
    // Draw component-specific symbols
    if (m_componentName == "Transactor") {
        // Draw transaction symbol (T with arrow)
        painter.drawText(rect, Qt::AlignCenter, "T");
        // Draw small arrow
        painter.drawLine(m_iconSize/2 - 5, m_iconSize/2 + 10, m_iconSize/2 + 5, m_iconSize/2 + 10);
        painter.drawLine(m_iconSize/2 + 3, m_iconSize/2 + 8, m_iconSize/2 + 5, m_iconSize/2 + 10);
        painter.drawLine(m_iconSize/2 + 3, m_iconSize/2 + 12, m_iconSize/2 + 5, m_iconSize/2 + 10);
    } else if (m_componentName == "RM") {
        // Draw RM symbol
        painter.drawText(rect, Qt::AlignCenter, "RM");
    } else if (m_componentName == "Compare") {
        // Draw comparison symbol (=)
        painter.drawLine(m_iconSize/2 - 15, m_iconSize/2 - 5, m_iconSize/2 + 15, m_iconSize/2 - 5);
        painter.drawLine(m_iconSize/2 - 15, m_iconSize/2 + 5, m_iconSize/2 + 15, m_iconSize/2 + 5);
    } else if (m_componentName == "Driver") {
        // Draw driver symbol (D with arrow)
        painter.drawText(rect, Qt::AlignCenter, "D");
        painter.drawLine(m_iconSize/2 + 8, m_iconSize/2, m_iconSize/2 + 15, m_iconSize/2);
        painter.drawLine(m_iconSize/2 + 12, m_iconSize/2 - 3, m_iconSize/2 + 15, m_iconSize/2);
        painter.drawLine(m_iconSize/2 + 12, m_iconSize/2 + 3, m_iconSize/2 + 15, m_iconSize/2);
    } else if (m_componentName == "Stimuler") {
        // Draw stimulator symbol (S with wave)
        painter.drawText(rect, Qt::AlignCenter, "S");
        // Draw wave pattern
        for (int i = 0; i < 3; ++i) {
            int x = m_iconSize/2 + 10 + i * 8;
            painter.drawLine(x, m_iconSize/2 - 5, x + 4, m_iconSize/2 + 5);
            painter.drawLine(x + 4, m_iconSize/2 + 5, x + 8, m_iconSize/2 - 5);
        }
    } else if (m_componentName == "Stimuli") {
        // Draw stimuli symbol (multiple lines)
        painter.drawText(rect, Qt::AlignCenter, "ST");
        for (int i = 0; i < 3; ++i) {
            painter.drawLine(m_iconSize/2 - 10, m_iconSize/2 + 8 + i * 3, 
                           m_iconSize/2 + 10, m_iconSize/2 + 8 + i * 3);
        }
    } else if (m_componentName == "RTL") {
        // Draw RTL symbol (RTL text)
        painter.drawText(rect, Qt::AlignCenter, "RTL");
    } else {
        // Default: just show the name
        painter.drawText(rect, Qt::AlignCenter, m_componentName);
    }
    
    painter.end();
    
    m_icon = pixmap;
    m_iconLabel->setPixmap(m_icon);
}

void ComponentCardWidget::setGraphicsView(DragDropGraphicsView* view)
{
    m_graphicsView = view;
}

void ComponentCardWidget::setDescription(const QString& description)
{
    m_description = description;
    m_descriptionLabel->setText(description);
}

void ComponentCardWidget::setCardSize(int width, int height)
{
    m_cardWidth = width;
    m_cardHeight = height;
    setFixedSize(width, height);
}

void ComponentCardWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Draw card background
    QRectF cardRect(1, 1, width() - 2, height() - 2);
    QColor bgColor = m_isHovered ? m_hoverColor : m_backgroundColor;
    
    painter.setPen(QPen(m_borderColor, 1));
    painter.setBrush(bgColor);
    painter.drawRoundedRect(cardRect, 8, 8);
    
    // Draw subtle shadow
    if (m_isHovered) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 20));
        painter.drawRoundedRect(cardRect.adjusted(2, 2, 2, 2), 8, 8);
    }
}

void ComponentCardWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
        m_dragging = false;
    }
    QWidget::mousePressEvent(event);
}

void ComponentCardWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    
    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    
    if (!m_dragging) {
        m_dragging = true;
        startDrag();
    }
}

void ComponentCardWidget::enterEvent(QEnterEvent* event)
{
    m_isHovered = true;
    m_isHovering = true;
    update();
    
    // Start hover timer to show preview
    m_hoverTimer->start();
    
    QWidget::enterEvent(event);
}

void ComponentCardWidget::leaveEvent(QEvent* event)
{
    m_isHovered = false;
    m_isHovering = false;
    update();
    
    // Stop hover timer and hide preview
    m_hoverTimer->stop();
    hidePreview();
    
    QWidget::leaveEvent(event);
}

void ComponentCardWidget::startDrag()
{
    if (!m_graphicsView) return;
    
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;
    
    // Set mime data for ready components
    mimeData->setData("application/x-ready-component", m_componentName.toUtf8());
    mimeData->setText(m_componentName);
    
    drag->setMimeData(mimeData);
    
    // Create drag pixmap
    QPixmap dragPixmap = m_icon.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    drag->setPixmap(dragPixmap);
    drag->setHotSpot(QPoint(30, 30));
    
    // Start drag operation
    Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
    
    qDebug() << "Started drag for component:" << m_componentName;
}

void ComponentCardWidget::showPreview()
{
    if (!m_isHovering || m_dragging) {
        return;
    }
    
    // Create preview widget if it doesn't exist
    if (!m_previewWidget) {
        m_previewWidget = new ComponentPreviewWidget(m_componentName, m_description);
    } else {
        // Update description in case it changed
        m_previewWidget->setDescription(m_description);
    }
    
    // Update preview position and show
    updatePreviewPosition();
    m_previewWidget->show();
}

void ComponentCardWidget::hidePreview()
{
    if (m_previewWidget) {
        m_previewWidget->hide();
    }
}

void ComponentCardWidget::updatePreviewPosition()
{
    if (!m_previewWidget) {
        return;
    }
    
    // Get the global position of this widget
    QPoint globalPos = mapToGlobal(QPoint(0, 0));
    
    // Calculate preview position (to the right of the card)
    int previewX = globalPos.x() + width() + 10;
    int previewY = globalPos.y();
    
    // Ensure preview doesn't go off screen
    QScreen* screen = QApplication::screenAt(globalPos);
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        
        // Adjust horizontal position if it would go off screen
        if (previewX + m_previewWidget->width() > screenGeometry.right()) {
            previewX = globalPos.x() - m_previewWidget->width() - 10;
        }
        
        // Adjust vertical position if it would go off screen
        if (previewY + m_previewWidget->height() > screenGeometry.bottom()) {
            previewY = screenGeometry.bottom() - m_previewWidget->height() - 10;
        }
        
        // Ensure it doesn't go above the screen
        if (previewY < screenGeometry.top()) {
            previewY = screenGeometry.top() + 10;
        }
    }
    
    m_previewWidget->move(previewX, previewY);
}

void ComponentCardWidget::moveEvent(QMoveEvent* event)
{
    QWidget::moveEvent(event);
    
    // Update preview position if it's showing
    if (m_previewWidget && m_previewWidget->isVisible()) {
        updatePreviewPosition();
    }
}
