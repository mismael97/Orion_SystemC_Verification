// ComponentPropertiesDialog.cpp
#include "ui/widgets/ComponentPropertiesDialog.h"
#include <QMessageBox>
#include <QApplication>
#include <QScreen>

ComponentPropertiesDialog::ComponentPropertiesDialog(QWidget *parent)
    : QDialog(parent)
    , m_metadataEditor(nullptr)
{
    setupUI();
    setupConnections();
    
    // Set dialog properties
    setModal(true);
    setWindowTitle("Component Properties");
    setMinimumSize(600, 700);
    resize(700, 800);
    
    // Center the dialog on the screen
    if (parent) {
        move(parent->x() + (parent->width() - width()) / 2, 
             parent->y() + (parent->height() - height()) / 2);
    } else {
        // Center on primary screen
        QScreen* screen = QApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->geometry();
            move(screenGeometry.x() + (screenGeometry.width() - width()) / 2,
                 screenGeometry.y() + (screenGeometry.height() - height()) / 2);
        }
    }
}

void ComponentPropertiesDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Create metadata editor
    m_metadataEditor = new ComponentMetadataEditor(this);
    m_mainLayout->addWidget(m_metadataEditor);
    
    // Button layout
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setSpacing(10);
    
    m_saveButton = new QPushButton("Save", this);
    m_saveButton->setStyleSheet(
        "QPushButton { "
        "background-color: #4CAF50; "
        "color: white; "
        "padding: 10px 20px; "
        "border: none; "
        "border-radius: 5px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { background-color: #45a049; } "
        "QPushButton:pressed { background-color: #3d8b40; }"
    );
    
    m_applyButton = new QPushButton("Apply", this);
    m_applyButton->setStyleSheet(
        "QPushButton { "
        "background-color: #2196F3; "
        "color: white; "
        "padding: 10px 20px; "
        "border: none; "
        "border-radius: 5px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { background-color: #1976D2; } "
        "QPushButton:pressed { background-color: #1565C0; }"
    );
    
    m_cancelButton = new QPushButton("Cancel", this);
    m_cancelButton->setStyleSheet(
        "QPushButton { "
        "background-color: #f44336; "
        "color: white; "
        "padding: 10px 20px; "
        "border: none; "
        "border-radius: 5px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { background-color: #da190b; } "
        "QPushButton:pressed { background-color: #c1170b; }"
    );
    
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_saveButton);
    m_buttonLayout->addWidget(m_applyButton);
    m_buttonLayout->addWidget(m_cancelButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Set font for Arabic text support
    setStyleSheet("font-family: 'Tajawal', sans-serif;");
}

void ComponentPropertiesDialog::setupConnections()
{
    connect(m_saveButton, &QPushButton::clicked, this, &ComponentPropertiesDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &ComponentPropertiesDialog::onCancelClicked);
    connect(m_applyButton, &QPushButton::clicked, this, [this]() {
        onSaveClicked();
        // Don't close the dialog on apply
    });
    
    connect(m_metadataEditor, &ComponentMetadataEditor::metadataChanged,
            this, &ComponentPropertiesDialog::onMetadataChanged);
}

void ComponentPropertiesDialog::setComponentId(const QString& componentId)
{
    m_componentId = componentId;
    if (m_metadataEditor) {
        m_metadataEditor->setComponentId(componentId);
    }
    setWindowTitle(QString("Component Properties - %1").arg(componentId));
}

void ComponentPropertiesDialog::setMetadata(const QJsonObject& metadata)
{
    m_originalMetadata = metadata;
    m_currentMetadata = metadata;
    if (m_metadataEditor) {
        m_metadataEditor->setMetadata(metadata);
    }
}

QJsonObject ComponentPropertiesDialog::getUpdatedMetadata() const
{
    return m_currentMetadata;
}

void ComponentPropertiesDialog::onSaveClicked()
{
    if (m_metadataEditor) {
        m_currentMetadata = m_metadataEditor->getMetadata();
    }
    
    emit metadataUpdated(m_componentId, m_currentMetadata);
    accept();
    
    qDebug() << "Component properties saved for:" << m_componentId;
}

void ComponentPropertiesDialog::onCancelClicked()
{
    reject();
}

void ComponentPropertiesDialog::onMetadataChanged(const QString& componentId, const QJsonObject& metadata)
{
    m_currentMetadata = metadata;
    
    // Enable apply button when changes are made
    m_applyButton->setEnabled(true);
    m_saveButton->setEnabled(true);
}

ComponentPropertiesDialog* ComponentPropertiesDialog::editComponent(const QString& componentId, 
                                                                   const QJsonObject& metadata, 
                                                                   QWidget* parent)
{
    ComponentPropertiesDialog* dialog = new ComponentPropertiesDialog(parent);
    dialog->setComponentId(componentId);
    dialog->setMetadata(metadata);
    
    return dialog;
}
