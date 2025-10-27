// MinimapWidget.cpp
#include "ui/widgets/MinimapWidget.h"
#include "graphics/ReadyComponentGraphicsItem.h"
#include "graphics/ModuleGraphicsItem.h"
#include <QPainter>
#include <QMouseEvent>
#include <QGraphicsItem>
#include <QPainterPath>
#include <QScrollBar>
#include <QFont>
#include <QComboBox>
#include <QHBoxLayout>
#include <QInputDialog>

MinimapWidget::MinimapWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(MIN_WIDTH, MIN_HEIGHT);
    setMaximumSize(280, 200); // Increased width to accommodate zoom controls
    setAttribute(Qt::WA_TranslucentBackground, false);
    setMouseTracking(true);
    
    // Set style for lighter background
    setStyleSheet("background-color: rgba(240, 240, 245, 200); border: 1px solid #888; border-radius: 4px;");
    
    // Create main layout
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(2, 2, 2, 2);
    m_layout->setSpacing(2);
    
    // Create controls layout (horizontal)
    m_controlsLayout = new QHBoxLayout();
    m_controlsLayout->setContentsMargins(0, 0, 0, 0);
    m_controlsLayout->setSpacing(2);
    
    // Create zoom combo box
    m_zoomComboBox = new QComboBox(this);
    m_zoomComboBox->setFixedSize(60, 20);
    m_zoomComboBox->setToolTip("Zoom level");
    m_zoomComboBox->addItem("50%");
    m_zoomComboBox->addItem("100%");
    m_zoomComboBox->addItem("150%");
    m_zoomComboBox->addItem("Custom");
    m_zoomComboBox->setCurrentText("100%"); // Default to 100%
    m_zoomComboBox->setStyleSheet(
        "QComboBox { "
        "background-color: rgba(255, 255, 255, 200); "
        "border: 1px solid #888; "
        "border-radius: 3px; "
        "font-size: 10px; "
        "font-family: 'Tajawal'; "
        "color: #666666; "
        "padding: 1px 3px; "
        "}"
        "QComboBox:hover { "
        "background-color: rgba(255, 255, 255, 240); "
        "border: 1px solid #666; "
        "color: #555555; "
        "}"
        "QComboBox::drop-down { "
        "border: none; "
        "width: 12px; "
        "}"
        "QComboBox::down-arrow { "
        "image: none; "
        "border-left: 4px solid transparent; "
        "border-right: 4px solid transparent; "
        "border-top: 4px solid #666; "
        "margin-right: 2px; "
        "}"
        "QComboBox QAbstractItemView { "
        "background-color: rgba(255, 255, 255, 240); "
        "border: 1px solid #888; "
        "border-radius: 3px; "
        "selection-background-color: rgba(100, 150, 200, 150); "
        "color: #666666; "
        "}"
        "QComboBox QAbstractItemView::item { "
        "padding: 2px 5px; "
        "color: #666666; "
        "}"
        "QComboBox QAbstractItemView::item:hover { "
        "background-color: rgba(100, 150, 200, 100); "
        "color: #555555; "
        "}"
        "QComboBox QAbstractItemView::item:selected { "
        "background-color: rgba(100, 150, 200, 150); "
        "color: #444444; "
        "}"
    );
    
    // Create focus components button
    m_focusButton = new QPushButton("🔍", this);
    m_focusButton->setFixedSize(24, 20);
    m_focusButton->setToolTip("Focus view on all components");
    m_focusButton->setStyleSheet(
        "QPushButton { "
        "background-color: rgba(255, 255, 255, 200); "
        "border: 1px solid #888; "
        "border-radius: 3px; "
        "font-size: 10px; "
        "}"
        "QPushButton:hover { "
        "background-color: rgba(255, 255, 255, 240); "
        "border: 1px solid #666; "
        "}"
        "QPushButton:pressed { "
        "background-color: rgba(200, 200, 200, 240); "
        "}"
    );
    
    // Add controls to horizontal layout
    m_controlsLayout->addWidget(m_zoomComboBox);
    m_controlsLayout->addWidget(m_focusButton);
    m_controlsLayout->addStretch(); // Push controls to the left
    
    // Add controls layout to main layout
    m_layout->addLayout(m_controlsLayout);
    m_layout->addStretch(); // Push controls to top
    
    // Connect signals
    connect(m_focusButton, &QPushButton::clicked, this, &MinimapWidget::onFocusComponentsClicked);
    connect(m_zoomComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MinimapWidget::onZoomLevelChanged);
}

void MinimapWidget::setScene(QGraphicsScene* scene)
{
    m_scene = scene;
    update();
}

void MinimapWidget::setMainView(QGraphicsView* view)
{
    m_mainView = view;
    update();
}

void MinimapWidget::updateViewportRect()
{
    if (!m_mainView || !m_scene) {
        return;
    }
    
    // Get the visible rect in scene coordinates
    m_viewportRect = m_mainView->mapToScene(m_mainView->viewport()->rect()).boundingRect();
    update();
}

QRectF MinimapWidget::getDrawableRect() const
{
    return rect().adjusted(MARGIN, MARGIN, -MARGIN, -MARGIN);
}

QTransform MinimapWidget::getSceneToMinimapTransform() const
{
    if (!m_scene) {
        return QTransform();
    }
    
    QRectF sceneRect = m_scene->sceneRect();
    if (sceneRect.isEmpty()) {
        // If scene rect is empty, calculate from items
        sceneRect = m_scene->itemsBoundingRect();
        if (sceneRect.isEmpty()) {
            sceneRect = QRectF(-1000, -1000, 2000, 2000); // Default size
        }
    }
    
    // Add some padding around the scene content
    qreal padding = 100;
    sceneRect.adjust(-padding, -padding, padding, padding);
    
    QRectF drawableRect = getDrawableRect();
    
    // Calculate scale to fit scene in drawable area
    qreal scaleX = drawableRect.width() / sceneRect.width();
    qreal scaleY = drawableRect.height() / sceneRect.height();
    qreal scale = qMin(scaleX, scaleY);
    
    // Center the minimap content
    QTransform transform;
    transform.translate(drawableRect.center().x(), drawableRect.center().y());
    transform.scale(scale, scale);
    transform.translate(-sceneRect.center().x(), -sceneRect.center().y());
    
    return transform;
}

QRectF MinimapWidget::getViewportRectInMinimapCoords() const
{
    if (!m_mainView || m_viewportRect.isEmpty()) {
        return QRectF();
    }
    
    QTransform transform = getSceneToMinimapTransform();
    return transform.mapRect(m_viewportRect);
}

QPointF MinimapWidget::minimapToScene(const QPointF& minimapPos) const
{
    QTransform transform = getSceneToMinimapTransform();
    bool invertible;
    QTransform inverted = transform.inverted(&invertible);
    
    if (invertible) {
        return inverted.map(minimapPos);
    }
    
    return QPointF();
}

void MinimapWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Draw lighter background with rounded corners
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(245, 245, 250, 220));
    painter.drawRoundedRect(rect(), 4, 4);
    
    // Draw border
    painter.setPen(QPen(QColor(150, 150, 160, 180), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 4, 4);
    
    if (!m_scene) {
        // Draw placeholder text
        painter.setPen(QColor(120, 120, 130));
        QFont font = painter.font();
        font.setPointSize(9);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, "Minimap");
        return;
    }
    
    QRectF drawableRect = getDrawableRect();
    QTransform transform = getSceneToMinimapTransform();
    
    // Save painter state
    painter.save();
    
    // Clip to drawable area
    painter.setClipRect(drawableRect);
    
    // Set transform
    painter.setTransform(transform, false);
    
    // Draw scene items in simplified form
    painter.setPen(Qt::NoPen);
    QList<QGraphicsItem*> items = m_scene->items();
    
    for (QGraphicsItem* item : items) {
        if (!item->isVisible()) {
            continue;
        }
        
        // Draw simplified representation
        QRectF itemRect = item->boundingRect();
        QRectF sceneRect = item->mapRectToScene(itemRect);
        
        // Different colors for different item types - lighter colors
        QColor itemColor = QColor(100, 150, 220, 160);
        
        painter.setBrush(itemColor);
        painter.setPen(QPen(itemColor.darker(120), 0.5));
        painter.drawRect(sceneRect);
    }
    
    // Restore painter state
    painter.restore();
    
    // Draw viewport rectangle
    QRectF viewportInMinimap = getViewportRectInMinimapCoords();
    if (!viewportInMinimap.isEmpty()) {
        // Draw semi-transparent overlay outside viewport - lighter
        QPainterPath fullPath;
        fullPath.addRoundedRect(drawableRect, 3, 3);
        
        QPainterPath viewportPath;
        viewportPath.addRect(viewportInMinimap);
        
        QPainterPath overlayPath = fullPath.subtracted(viewportPath);
        painter.fillPath(overlayPath, QColor(200, 200, 210, 80));
        
        // Draw viewport border - more subtle
        painter.setPen(QPen(QColor(70, 130, 255), 2));
        painter.setBrush(QColor(70, 130, 255, 30));
        painter.drawRect(viewportInMinimap);
    }
    
    // Draw minimap border again on top
    painter.setPen(QPen(QColor(150, 150, 160, 180), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 4, 4);
}

void MinimapWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QRectF viewportInMinimap = getViewportRectInMinimapCoords();
        
        if (viewportInMinimap.contains(event->pos())) {
            // Start dragging viewport
            m_isDragging = true;
            m_dragStartPos = event->pos();
        } else {
            // Click outside viewport - center on clicked position
            QPointF scenePos = minimapToScene(event->pos());
            emit viewportMoved(scenePos);
        }
        
        event->accept();
    }
}

void MinimapWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging) {
        QPointF delta = event->pos() - m_dragStartPos;
        m_dragStartPos = event->pos();
        
        // Calculate new viewport center in scene coordinates
        QRectF viewportInMinimap = getViewportRectInMinimapCoords();
        QPointF newCenter = viewportInMinimap.center() + delta;
        QPointF sceneCenterPos = minimapToScene(newCenter);
        
        emit viewportMoved(sceneCenterPos);
        event->accept();
    } else {
        // Update cursor based on position
        QRectF viewportInMinimap = getViewportRectInMinimapCoords();
        if (viewportInMinimap.contains(event->pos())) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void MinimapWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    }
}

void MinimapWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update();
}

void MinimapWidget::onFocusComponentsClicked()
{
    if (!m_scene) {
        return;
    }
    
    QRectF componentsRect = getComponentsBoundingRect();
    if (!componentsRect.isEmpty()) {
        emit focusOnComponents(componentsRect);
    }
}

QRectF MinimapWidget::getComponentsBoundingRect() const
{
    if (!m_scene) {
        return QRectF();
    }
    
    qreal minX = 0, maxX = 0, minY = 0, maxY = 0;
    bool firstItem = true;
    
    QList<QGraphicsItem*> items = m_scene->items();
    for (QGraphicsItem* item : items) {
        if (!item->isVisible()) {
            continue;
        }
        
        // Check if item is a component (ReadyComponentGraphicsItem or ModuleGraphicsItem)
        ReadyComponentGraphicsItem* readyComponent = qgraphicsitem_cast<ReadyComponentGraphicsItem*>(item);
        ModuleGraphicsItem* moduleComponent = qgraphicsitem_cast<ModuleGraphicsItem*>(item);
        
        if (readyComponent || moduleComponent) {
            QRectF itemRect = item->mapRectToScene(item->boundingRect());
            
            if (firstItem) {
                // Initialize with first component's bounds
                minX = itemRect.left();
                maxX = itemRect.right();
                minY = itemRect.top();
                maxY = itemRect.bottom();
                firstItem = false;
            } else {
                // Expand bounds to include this component
                minX = qMin(minX, itemRect.left());
                maxX = qMax(maxX, itemRect.right());
                minY = qMin(minY, itemRect.top());
                maxY = qMax(maxY, itemRect.bottom());
            }
        }
    }
    
    if (firstItem) {
        return QRectF(); // No components found
    }
    
    // Create bounding rectangle from min/max coordinates
    // Width = max_x - min_x, Height = max_y - min_y
    QRectF boundingRect(minX, minY, maxX - minX, maxY - minY);
    
    // Add exactly 10 pixels padding around the components
    qreal padding = 10;
    boundingRect.adjust(-padding, -padding, padding, padding);
    
    return boundingRect;
}

void MinimapWidget::onZoomLevelChanged()
{
    QString currentText = m_zoomComboBox->currentText();
    qreal zoomLevel = 1.0; // Default to 100%
    
    if (currentText == "50%") {
        zoomLevel = 0.5;
    } else if (currentText == "100%") {
        zoomLevel = 1.0;
    } else if (currentText == "150%") {
        zoomLevel = 1.5;
    } else if (currentText == "Custom") {
        // Show input dialog for custom zoom level
        bool ok;
        int customZoom = QInputDialog::getInt(this, 
            "Custom Zoom", 
            "Enter zoom percentage (25-500):", 
            100, 25, 500, 1, &ok);
        
        if (ok) {
            zoomLevel = customZoom / 100.0;
            // Update combo box to show the custom value
            m_zoomComboBox->setItemText(3, QString("%1%").arg(customZoom));
        } else {
            // User cancelled, revert to previous selection
            m_zoomComboBox->setCurrentText("100%");
            return;
        }
    }
    
    // Emit the zoom changed signal
    emit zoomChanged(zoomLevel);
}


