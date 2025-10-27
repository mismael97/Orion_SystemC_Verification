// ComponentCardWidget.h
#ifndef COMPONENTCARDWIDGET_H
#define COMPONENTCARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QColor>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QTimer>

class DragDropGraphicsView;
class ComponentPreviewWidget;

/**
 * @brief A card widget that displays a ready component with image, name, and description
 * 
 * This widget provides a modern card-based interface for ready components,
 * making them more visually appealing and easier to understand.
 */
class ComponentCardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ComponentCardWidget(const QString& componentName, 
                                const QString& description = "",
                                QWidget* parent = nullptr);
    
    void setGraphicsView(DragDropGraphicsView* view);
    QString getComponentName() const { return m_componentName; }
    void setDescription(const QString& description);
    void setCardSize(int width, int height);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void moveEvent(QMoveEvent* event) override;

private:
    void setupUI();
    void createComponentIcon();
    void startDrag();
    void showPreview();
    void hidePreview();
    void updatePreviewPosition();

    QString m_componentName;
    QString m_description;
    QPixmap m_icon;
    DragDropGraphicsView* m_graphicsView = nullptr;
    
    // UI elements
    QVBoxLayout* m_mainLayout;
    QLabel* m_iconLabel;
    QLabel* m_nameLabel;
    QLabel* m_descriptionLabel;
    
    // Visual properties
    QColor m_backgroundColor;
    QColor m_borderColor;
    QColor m_hoverColor;
    bool m_isHovered = false;
    
    // Drag properties
    QPoint m_dragStartPosition;
    bool m_dragging = false;
    
    // Card dimensions
    int m_cardWidth = 200;
    int m_cardHeight = 150;
    int m_iconSize = 80;
    
    // Preview functionality
    ComponentPreviewWidget* m_previewWidget = nullptr;
    QTimer* m_hoverTimer = nullptr;
    bool m_isHovering = false;
    static constexpr int HOVER_DELAY = 500; // ms
};

#endif // COMPONENTCARDWIDGET_H
