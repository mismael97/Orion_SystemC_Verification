// ComponentLibraryWidget.h
#ifndef COMPONENTLIBRARYWIDGET_H
#define COMPONENTLIBRARYWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QResizeEvent>
#include "ui/widgets/ComponentCardWidget.h"

class DragDropGraphicsView;

/**
 * @brief A widget that displays ready components as a grid of cards
 * 
 * This widget provides a modern card-based library interface for ready components,
 * replacing the traditional list view with a more visually appealing grid layout.
 */
class ComponentLibraryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ComponentLibraryWidget(QWidget* parent = nullptr);
    
    void setGraphicsView(DragDropGraphicsView* view);
    void addComponent(const QString& name, const QString& description = "");
    void clearComponents();
    void setCardSize(int width, int height);
    void setColumns(int columns);
    void setResponsive(bool enabled);
    void setCardSizeConstraints(int minWidth, int maxWidth);
    void setColumnConstraints(int minColumns, int maxColumns);
    
    // Getters for current layout state
    int getCurrentColumns() const { return m_columns; }
    int getCurrentCardWidth() const { return m_cardWidth; }
    int getCurrentCardHeight() const { return m_cardHeight; }
    bool isResponsive() const { return m_isResponsive; }

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void updateLayout();
    void addComponentCard(const QString& name, const QString& description);
    void calculateOptimalLayout();
    int calculateOptimalColumns(int availableWidth);
    int calculateOptimalCardWidth(int availableWidth, int columns);

    QScrollArea* m_scrollArea;
    QWidget* m_scrollContent;
    QGridLayout* m_gridLayout;
    QList<ComponentCardWidget*> m_componentCards;
    
    DragDropGraphicsView* m_graphicsView = nullptr;
    
    // Layout properties
    int m_columns = 2;
    int m_cardWidth = 200;
    int m_cardHeight = 150;
    int m_spacing = 10;
    
    // Responsive layout properties
    int m_minCardWidth = 150;
    int m_maxCardWidth = 300;
    int m_minColumns = 1;
    int m_maxColumns = 4;
    bool m_isResponsive = true;
    
    // Debouncing for resize events
    QTimer* m_resizeTimer = nullptr;
    static constexpr int RESIZE_DEBOUNCE_MS = 100;
};

#endif // COMPONENTLIBRARYWIDGET_H
