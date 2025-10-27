// VerticalToolbar.cpp
#include "ui/widgets/VerticalToolbar.h"
#include <QVBoxLayout>
#include <QPushButton>

VerticalToolbar::VerticalToolbar(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    applyStyles();
}

void VerticalToolbar::setupUI()
{
    // Create vertical layout (no buttons for now)
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addStretch();
    
    // Set widget size to minimal
    setFixedSize(90, 20);
}

void VerticalToolbar::applyStyles()
{
    // Semi-transparent background with rounded corners
    setStyleSheet(
        "VerticalToolbar {"
        "    background-color: rgba(50, 50, 50, 200);"
        "    border-radius: 8px;"
        "    border: 2px solid rgba(100, 100, 100, 150);"
        "}"
        "QPushButton {"
        "    background-color: rgba(80, 80, 80, 200);"
        "    color: white;"
        "    border: 1px solid rgba(120, 120, 120, 150);"
        "    border-radius: 5px;"
        "    font-size: 11px;"
        "    font-weight: bold;"
        "    font-family: 'Tajawal';"
        "    padding: 5px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(100, 100, 100, 220);"
        "    border: 2px solid rgba(150, 150, 200, 200);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(60, 60, 120, 250);"
        "}"
    );
}

