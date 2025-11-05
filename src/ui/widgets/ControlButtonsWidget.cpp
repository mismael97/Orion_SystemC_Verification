// ControlButtonsWidget.cpp
#include "ui/widgets/ControlButtonsWidget.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QIcon>
#include <QStyle>
#include <QPainter>
#include <QPixmap>
#include <QDebug>

ControlButtonsWidget::ControlButtonsWidget(QWidget *parent)
    : QWidget(parent)
    , m_runButton(nullptr)
    , m_stopButton(nullptr)
    , m_pauseButton(nullptr)
    , m_state(Stopped)
{
    setupUI();
    applyStyles();
    updateButtonStates();
}

void ControlButtonsWidget::setupUI()
{
    // Create horizontal layout for buttons
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(8, 8, 8, 8);
    
    // Create icons for buttons
    QIcon runIcon = createRunIcon();
    QIcon pauseIcon = createPauseIcon();
    QIcon stopIcon = createStopIcon();
    
    // Create run button with icon (smaller size)
    m_runButton = new QPushButton(this);
    m_runButton->setIcon(runIcon);
    m_runButton->setToolTip("Run");
    m_runButton->setFixedSize(32, 32);
    m_runButton->setIconSize(QSize(18, 18));
    connect(m_runButton, &QPushButton::clicked, this, &ControlButtonsWidget::onRunButtonClicked);
    
    // Create pause button with icon (smaller size)
    m_pauseButton = new QPushButton(this);
    m_pauseButton->setIcon(pauseIcon);
    m_pauseButton->setToolTip("Pause");
    m_pauseButton->setFixedSize(32, 32);
    m_pauseButton->setIconSize(QSize(18, 18));
    connect(m_pauseButton, &QPushButton::clicked, this, &ControlButtonsWidget::onPauseButtonClicked);
    
    // Create stop button with icon (smaller size)
    m_stopButton = new QPushButton(this);
    m_stopButton->setIcon(stopIcon);
    m_stopButton->setToolTip("Stop");
    m_stopButton->setFixedSize(32, 32);
    m_stopButton->setIconSize(QSize(18, 18));
    connect(m_stopButton, &QPushButton::clicked, this, &ControlButtonsWidget::onStopButtonClicked);
    
    // Add spacer to push buttons to the right
    layout->addStretch();
    
    // Add buttons to layout
    layout->addWidget(m_runButton);
    layout->addWidget(m_pauseButton);
    layout->addWidget(m_stopButton);
    
    setLayout(layout);
    
    // Set size policy for layout integration
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    // Set minimum size hint based on button sizes and layout (smaller buttons)
    setMinimumSize(32 * 3 + 5 * 2 + 16, 32 + 16); // 3 buttons + 2 spacing + margins
}

void ControlButtonsWidget::applyStyles()
{
    // Styling for toolbar integration - clean and modern
    setStyleSheet(
        "ControlButtonsWidget {"
        "    background-color: transparent;"
        "    border: none;"
        "}"
        "QPushButton {"
        "    background-color: #4A4A4A;"
        "    color: white;"
        "    border: 1px solid #5A5A5A;"
        "    border-radius: 4px;"
        "    min-width: 32px;"
        "    min-height: 32px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #5A5A5A;"
        "    border: 1px solid #6A6A6A;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3A3A3A;"
        "    border: 1px solid #4A4A4A;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #2A2A2A;"
        "    color: #666666;"
        "    border: 1px solid #3A3A3A;"
        "}"
    );
}

void ControlButtonsWidget::setState(State state)
{
    if (m_state != state) {
        m_state = state;
        updateButtonStates();
    }
}

void ControlButtonsWidget::updateButtonStates()
{
    switch (m_state) {
        case Stopped:
            m_runButton->setEnabled(true);
            m_pauseButton->setEnabled(false);
            m_stopButton->setEnabled(false);
            break;
        case Running:
            m_runButton->setEnabled(false);
            m_pauseButton->setEnabled(true);
            m_stopButton->setEnabled(true);
            break;
        case Paused:
            m_runButton->setEnabled(true);
            m_pauseButton->setEnabled(false);
            m_stopButton->setEnabled(true);
            break;
    }
}

void ControlButtonsWidget::onRunButtonClicked()
{
    qDebug() << "▶ Run button clicked";
    setState(Running);
    emit runClicked();
}

void ControlButtonsWidget::onStopButtonClicked()
{
    qDebug() << "⏹ Stop button clicked";
    setState(Stopped);
    emit stopClicked();
}

void ControlButtonsWidget::onPauseButtonClicked()
{
    qDebug() << "⏸ Pause button clicked";
    if (m_state == Running) {
        setState(Paused);
    } else if (m_state == Paused) {
        setState(Running);
    }
    emit pauseClicked();
}

QIcon ControlButtonsWidget::createRunIcon()
{
    QPixmap pixmap(18, 18);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Draw play triangle (scaled for smaller icon)
    QPolygon triangle;
    triangle << QPoint(5, 4) << QPoint(5, 14) << QPoint(13, 9);
    painter.setBrush(QColor(76, 175, 80)); // Green color
    painter.setPen(QPen(QColor(56, 142, 60), 1));
    painter.drawPolygon(triangle);
    
    return QIcon(pixmap);
}

QIcon ControlButtonsWidget::createPauseIcon()
{
    QPixmap pixmap(18, 18);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Draw two vertical rectangles for pause (scaled for smaller icon)
    painter.setBrush(QColor(255, 193, 7)); // Orange/Amber color
    painter.setPen(QPen(QColor(255, 152, 0), 1));
    
    // Left rectangle
    painter.drawRoundedRect(5, 4, 3, 10, 1, 1);
    // Right rectangle
    painter.drawRoundedRect(10, 4, 3, 10, 1, 1);
    
    return QIcon(pixmap);
}

QIcon ControlButtonsWidget::createStopIcon()
{
    QPixmap pixmap(18, 18);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Draw square for stop (scaled for smaller icon)
    QRectF rect(5, 5, 8, 8);
    painter.setBrush(QColor(244, 67, 54)); // Red color
    painter.setPen(QPen(QColor(198, 40, 40), 1));
    painter.drawRoundedRect(rect, 2, 2);
    
    return QIcon(pixmap);
}

