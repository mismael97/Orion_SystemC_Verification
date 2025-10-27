// MinimapWidget.h
#ifndef MINIMAPWIDGET_H
#define MINIMAPWIDGET_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>
#include <QVBoxLayout>
#include <QComboBox>
#include <QHBoxLayout>

/**
 * @brief A minimap widget showing an overview of the entire schematic
 * 
 * Displays a scaled-down view of the entire scene with a viewport rectangle
 * that can be dragged to navigate the main view.
 */
class MinimapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MinimapWidget(QWidget *parent = nullptr);
    
    /**
     * @brief Sets the scene to display in the minimap
     */
    void setScene(QGraphicsScene* scene);
    
    /**
     * @brief Sets the main graphics view to work with
     */
    void setMainView(QGraphicsView* view);
    
    /**
     * @brief Updates the viewport rectangle position based on main view
     */
    void updateViewportRect();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

signals:
    /**
     * @brief Emitted when the user drags the viewport rectangle
     * @param scenePos The new center position in scene coordinates
     */
    void viewportMoved(const QPointF& scenePos);
    
    /**
     * @brief Emitted when the user clicks the focus components button
     * @param boundingRect The bounding rectangle of all components in scene coordinates
     */
    void focusOnComponents(const QRectF& boundingRect);
    
    /**
     * @brief Emitted when the user changes the zoom level
     * @param zoomLevel The new zoom level (0.5 for 50%, 1.0 for 100%, 1.5 for 150%, etc.)
     */
    void zoomChanged(qreal zoomLevel);

private slots:
    /**
     * @brief Handles the focus components button click
     */
    void onFocusComponentsClicked();
    
    /**
     * @brief Handles zoom level changes from the combo box
     */
    void onZoomLevelChanged();

private:
    QGraphicsScene* m_scene = nullptr;
    QGraphicsView* m_mainView = nullptr;
    QRectF m_viewportRect;
    bool m_isDragging = false;
    QPointF m_dragStartPos;
    
    // UI elements
    QPushButton* m_focusButton = nullptr;
    QComboBox* m_zoomComboBox = nullptr;
    QVBoxLayout* m_layout = nullptr;
    QHBoxLayout* m_controlsLayout = nullptr;
    
    // Minimap dimensions and margins
    static constexpr int MARGIN = 5;
    static constexpr int MIN_WIDTH = 120;
    static constexpr int MIN_HEIGHT = 120;
    
    /**
     * @brief Calculates the transform from scene to minimap coordinates
     */
    QTransform getSceneToMinimapTransform() const;
    
    /**
     * @brief Calculates the viewport rectangle in minimap coordinates
     */
    QRectF getViewportRectInMinimapCoords() const;
    
    /**
     * @brief Converts minimap coordinates to scene coordinates
     */
    QPointF minimapToScene(const QPointF& minimapPos) const;
    
    /**
     * @brief Gets the drawable area of the minimap (excluding margins)
     */
    QRectF getDrawableRect() const;
    
    /**
     * @brief Calculates the bounding rectangle of all components in the scene
     */
    QRectF getComponentsBoundingRect() const;
};

#endif // MINIMAPWIDGET_H

