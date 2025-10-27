// ComponentLibraryWidget.cpp
#include "ui/widgets/ComponentLibraryWidget.h"
#include "ui/widgets/dragdropgraphicsview.h"
#include <QScrollArea>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QTimer>
#include <QDebug>

ComponentLibraryWidget::ComponentLibraryWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void ComponentLibraryWidget::setupUI()
{
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Create scroll area
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    
    // Create scroll content widget
    m_scrollContent = new QWidget();
    m_scrollContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Create grid layout for cards
    m_gridLayout = new QGridLayout(m_scrollContent);
    m_gridLayout->setContentsMargins(m_spacing, m_spacing, m_spacing, m_spacing);
    m_gridLayout->setSpacing(m_spacing);
    m_gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    // Set scroll area content
    m_scrollArea->setWidget(m_scrollContent);
    
    // Add scroll area to main layout
    mainLayout->addWidget(m_scrollArea);
    
    // Set up initial components
    addComponent("Transactor", "Transaction-level modeling component for high-level verification");
    addComponent("RM", "Reference Model for verification and comparison");
    addComponent("Compare", "Comparison component for checking results");
    addComponent("Driver", "Test driver for generating stimulus");
    addComponent("Stimuler", "Stimulus generator for test scenarios");
    addComponent("Stimuli", "Test stimuli and data patterns");
    addComponent("RTL", "Register Transfer Level design components");
    
    // Set up resize debouncing timer
    m_resizeTimer = new QTimer(this);
    m_resizeTimer->setSingleShot(true);
    m_resizeTimer->setInterval(RESIZE_DEBOUNCE_MS);
    connect(m_resizeTimer, &QTimer::timeout, this, &ComponentLibraryWidget::calculateOptimalLayout);
    
    // Calculate initial optimal layout
    QTimer::singleShot(0, this, [this]() {
        calculateOptimalLayout();
    });
}

void ComponentLibraryWidget::setGraphicsView(DragDropGraphicsView* view)
{
    m_graphicsView = view;
    
    // Set graphics view for all existing cards
    for (ComponentCardWidget* card : m_componentCards) {
        card->setGraphicsView(view);
    }
}

void ComponentLibraryWidget::addComponent(const QString& name, const QString& description)
{
    addComponentCard(name, description);
    updateLayout();
}

void ComponentLibraryWidget::addComponentCard(const QString& name, const QString& description)
{
    ComponentCardWidget* card = new ComponentCardWidget(name, description, this);
    card->setGraphicsView(m_graphicsView);
    card->setCardSize(m_cardWidth, m_cardHeight);
    
    m_componentCards.append(card);
}

void ComponentLibraryWidget::clearComponents()
{
    // Remove all cards from layout
    for (ComponentCardWidget* card : m_componentCards) {
        m_gridLayout->removeWidget(card);
        card->deleteLater();
    }
    m_componentCards.clear();
}

void ComponentLibraryWidget::setCardSize(int width, int height)
{
    m_cardWidth = width;
    m_cardHeight = height;
    
    // Update size for all existing cards
    for (ComponentCardWidget* card : m_componentCards) {
        card->setCardSize(width, height);
    }
    
    updateLayout();
}

void ComponentLibraryWidget::setColumns(int columns)
{
    m_columns = columns;
    updateLayout();
}

void ComponentLibraryWidget::updateLayout()
{
    // Clear existing layout
    for (ComponentCardWidget* card : m_componentCards) {
        m_gridLayout->removeWidget(card);
    }
    
    // Add cards to grid layout
    for (int i = 0; i < m_componentCards.size(); ++i) {
        int row = i / m_columns;
        int col = i % m_columns;
        m_gridLayout->addWidget(m_componentCards[i], row, col);
    }
    
    // Update scroll content size
    m_scrollContent->adjustSize();
    
    qDebug() << "Updated component library layout with" << m_componentCards.size() 
             << "components in" << m_columns << "columns, card size:" << m_cardWidth << "x" << m_cardHeight;
}

void ComponentLibraryWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    
    if (m_isResponsive && event->size().width() != event->oldSize().width()) {
        // Use debounced timer to avoid too frequent recalculations
        m_resizeTimer->start();
    }
}

void ComponentLibraryWidget::calculateOptimalLayout()
{
    if (!m_isResponsive || m_componentCards.isEmpty()) {
        return;
    }
    
    // Calculate available width (accounting for scrollbar and margins)
    int availableWidth = width() - m_spacing * 2 - 20; // 20px for potential scrollbar
    
    // Calculate optimal number of columns
    int optimalColumns = calculateOptimalColumns(availableWidth);
    
    // Calculate optimal card width
    int optimalCardWidth = calculateOptimalCardWidth(availableWidth, optimalColumns);
    
    // Update layout if values changed
    bool layoutChanged = false;
    
    if (optimalColumns != m_columns) {
        m_columns = optimalColumns;
        layoutChanged = true;
        qDebug() << "Updated columns to:" << m_columns;
    }
    
    if (optimalCardWidth != m_cardWidth) {
        m_cardWidth = optimalCardWidth;
        layoutChanged = true;
        qDebug() << "Updated card width to:" << m_cardWidth;
    }
    
    if (layoutChanged) {
        // Update all card sizes
        for (ComponentCardWidget* card : m_componentCards) {
            card->setCardSize(m_cardWidth, m_cardHeight);
        }
        
        // Update layout
        updateLayout();
    }
}

int ComponentLibraryWidget::calculateOptimalColumns(int availableWidth)
{
    // Calculate how many columns can fit with minimum card width
    int maxPossibleColumns = (availableWidth + m_spacing) / (m_minCardWidth + m_spacing);
    maxPossibleColumns = qMin(maxPossibleColumns, m_maxColumns);
    maxPossibleColumns = qMax(maxPossibleColumns, m_minColumns);
    
    // Calculate how many columns can fit with maximum card width
    int minPossibleColumns = (availableWidth + m_spacing) / (m_maxCardWidth + m_spacing);
    minPossibleColumns = qMax(minPossibleColumns, m_minColumns);
    minPossibleColumns = qMin(minPossibleColumns, m_maxColumns);
    
    // Choose the optimal number of columns based on available space
    int optimalColumns = maxPossibleColumns;
    
    // For very narrow widths, prefer fewer columns with larger cards
    if (availableWidth < 400) {
        optimalColumns = qMin(optimalColumns, 2);
    }
    // For medium widths, allow up to 3 columns
    else if (availableWidth < 600) {
        optimalColumns = qMin(optimalColumns, 3);
    }
    // For wide widths, allow up to maximum columns
    else {
        optimalColumns = qMin(optimalColumns, m_maxColumns);
    }
    
    // Ensure we don't have too many columns that make cards too small
    int cardWidthWithOptimalColumns = (availableWidth - (optimalColumns - 1) * m_spacing) / optimalColumns;
    if (cardWidthWithOptimalColumns < m_minCardWidth) {
        optimalColumns = minPossibleColumns;
    }
    
    return optimalColumns;
}

int ComponentLibraryWidget::calculateOptimalCardWidth(int availableWidth, int columns)
{
    if (columns <= 0) {
        return m_cardWidth;
    }
    
    // Calculate card width based on available space and number of columns
    int calculatedWidth = (availableWidth - (columns - 1) * m_spacing) / columns;
    
    // Apply constraints
    int optimalWidth = qMax(calculatedWidth, m_minCardWidth);
    optimalWidth = qMin(optimalWidth, m_maxCardWidth);
    
    // Adjust card height proportionally to maintain good aspect ratio
    // Keep aspect ratio between 1.2:1 and 1.5:1 for better visual appearance
    float aspectRatio = 1.3f; // Default aspect ratio
    int newCardHeight = static_cast<int>(optimalWidth / aspectRatio);
    
    // Ensure height is within reasonable bounds
    newCardHeight = qMax(newCardHeight, 120);
    newCardHeight = qMin(newCardHeight, 200);
    
    // Update card height if it changed significantly
    if (qAbs(newCardHeight - m_cardHeight) > 10) {
        m_cardHeight = newCardHeight;
    }
    
    return optimalWidth;
}

void ComponentLibraryWidget::setResponsive(bool enabled)
{
    m_isResponsive = enabled;
    if (enabled) {
        calculateOptimalLayout();
    }
}

void ComponentLibraryWidget::setCardSizeConstraints(int minWidth, int maxWidth)
{
    m_minCardWidth = qMax(minWidth, 100); // Minimum reasonable width
    m_maxCardWidth = qMin(maxWidth, 500); // Maximum reasonable width
    
    if (m_isResponsive) {
        calculateOptimalLayout();
    }
}

void ComponentLibraryWidget::setColumnConstraints(int minColumns, int maxColumns)
{
    m_minColumns = qMax(minColumns, 1);
    m_maxColumns = qMin(maxColumns, 6); // Maximum reasonable columns
    
    if (m_isResponsive) {
        calculateOptimalLayout();
    }
}
