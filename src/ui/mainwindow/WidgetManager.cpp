// WidgetManager.cpp
#include "ui/mainwindow/WidgetManager.h"
#include "ui/MainWindow.h"
#include "ui/widgets/MinimapWidget.h"
#include "ui/widgets/VerticalToolbar.h"
#include "ui/widgets/EditComponentWidget.h"
#include "utils/PersistenceManager.h"
#include "scene/SchematicScene.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QScrollBar>
#include <QFont>
#include <QDebug>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QSplitter>

WidgetManager::WidgetManager(MainWindow* mainWindow, QGraphicsView* graphicsView)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
    , m_graphicsView(graphicsView)
    , m_minimap(nullptr)
    , m_verticalToolbar(nullptr)
    , m_editComponentWidget(nullptr)
{
}

void WidgetManager::setupMinimap()
{
    // Get scene from graphics view
    QGraphicsScene* scene = m_graphicsView->scene();
    if (!scene) {
        qWarning() << "Cannot setup minimap: scene is null";
        return;
    }
    
    // Create minimap widget
    m_minimap = new MinimapWidget(m_graphicsView);
    m_minimap->setScene(scene);
    m_minimap->setMainView(m_graphicsView);
    
    // Set minimap properties - adjusted size for zoom controls
    m_minimap->setFixedSize(180, 180);
    m_minimap->raise(); // Bring to front
    m_minimap->show();
    
    // Position it in the bottom-right corner
    updateMinimapPosition();
    
    // Connect minimap viewport movement to main view
    connect(m_minimap, &MinimapWidget::viewportMoved, this, [this](const QPointF& scenePos) {
        m_graphicsView->centerOn(scenePos);
        m_minimap->updateViewportRect();
    });
    
    // Connect minimap focus components to main view
    connect(m_minimap, &MinimapWidget::focusOnComponents, this, [this](const QRectF& boundingRect) {
        if (!boundingRect.isEmpty()) {
            m_graphicsView->fitInView(boundingRect, Qt::KeepAspectRatio);
            m_minimap->updateViewportRect();
        }
    });
    
    // Connect minimap zoom changes to main view
    connect(m_minimap, &MinimapWidget::zoomChanged, this, [this](qreal zoomLevel) {
        // Apply zoom to the main graphics view
        QTransform transform;
        transform.scale(zoomLevel, zoomLevel);
        m_graphicsView->setTransform(transform);
        m_minimap->updateViewportRect();
    });
    
    // Connect view changes to update minimap
    connect(m_graphicsView->horizontalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
        m_minimap->updateViewportRect();
    });
    connect(m_graphicsView->verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
        m_minimap->updateViewportRect();
    });
    
    // Connect scene changes to update minimap
    connect(scene, &QGraphicsScene::changed, this, [this]() {
        m_minimap->update();
    });
}

void WidgetManager::updateMinimapPosition()
{
    if (!m_minimap || !m_minimap->isVisible()) return;
    
    // Position minimap with fixed padding from bottom-right corner
    int padding = 15;
    QSize minimapSize = m_minimap->size();
    QSize viewSize = m_graphicsView->size();
    
    // Calculate position - 15px padding from right and bottom edges
    int x = viewSize.width() - minimapSize.width() - padding;
    int y = viewSize.height() - minimapSize.height() - padding;
    
    m_minimap->move(x, y);
    m_minimap->raise();
}


void WidgetManager::setupSchematicOverlays()
{
    // Create vertical toolbar
    m_verticalToolbar = new VerticalToolbar(m_graphicsView);
    m_verticalToolbar->raise();
    m_verticalToolbar->show();
    
    // Position the overlays
    updateSchematicOverlaysPosition();
}

void WidgetManager::updateSchematicOverlaysPosition()
{
    if (!m_verticalToolbar) return;
    
    // Get the graphics view's size
    QSize viewSize = m_graphicsView->size();
    
    // Position vertical toolbar at top-left corner with padding
    int padding = 15;
    int toolbarX = padding;
    int toolbarY = padding;
    m_verticalToolbar->move(toolbarX, toolbarY);
    m_verticalToolbar->raise();
}

void WidgetManager::setCurrentRtlDirectory(const QString& directory)
{
    m_currentRtlDirectory = directory;
}

void WidgetManager::setupEditComponentWidget()
{
    // Find the existing widgetEditComponent in the UI
    QWidget* existingWidget = m_mainWindow->findChild<QWidget*>("widgetEditComponent");
    if (!existingWidget) {
        qWarning() << "widgetEditComponent not found in UI";
        return;
    }
    
    // Create the edit component widget
    m_editComponentWidget = new EditComponentWidget();
    
    // Set up layout for the existing widget
    if (!existingWidget->layout()) {
        QVBoxLayout* layout = new QVBoxLayout(existingWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        existingWidget->setLayout(layout);
    }
    
    // Add our edit component widget to the layout
    existingWidget->layout()->addWidget(m_editComponentWidget);
    
    // Set size constraints for the editor panel
    existingWidget->setMinimumWidth(250);  // Minimum when visible
    existingWidget->setMaximumWidth(16777215);  // No maximum (QWIDGETSIZE_MAX)
    
    // Set size policy to control expansion
    existingWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    
    // Initially hide the widget
    existingWidget->hide();
    m_editComponentWidget->hide();
    
    // Connect signals
    connect(m_editComponentWidget, &EditComponentWidget::componentSaved, this, [this](const QString& filePath) {
        qDebug() << "Component saved, refreshing:" << filePath;
        m_mainWindow->refreshComponent(filePath);
    });
    
    connect(m_editComponentWidget, &EditComponentWidget::editorClosed, this, [this]() {
        // Hide the editor panel using the splitter
        QSplitter* splitter = m_mainWindow->findChild<QSplitter*>("mainHorizontalSplitter");
        QWidget* parentWidget = m_mainWindow->findChild<QWidget*>("widgetEditComponent");
        
        if (splitter && parentWidget) {
            // Set the editor size to 0 in the splitter
            QList<int> sizes = splitter->sizes();
            if (sizes.size() >= 3) {
                sizes[2] = 0;  // Editor is the 3rd widget
                splitter->setSizes(sizes);
            }
            parentWidget->hide();
        }
    });
    
    qDebug() << "Edit component widget setup complete";
}

