// ComponentMetadataEditor.cpp
#include "ui/widgets/ComponentMetadataEditor.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QColorDialog>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QMessageBox>
#include <QDebug>

ComponentMetadataEditor::ComponentMetadataEditor(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupConnections();
    setReadOnly(false);
}

void ComponentMetadataEditor::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Basic Information Group
    m_basicGroup = new QGroupBox("Component Information", this);
    m_basicLayout = new QFormLayout(m_basicGroup);
    
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Component name");
    
    m_typeEdit = new QLineEdit(this);
    m_typeEdit->setPlaceholderText("Component type");
    m_typeEdit->setReadOnly(true);
    
    m_authorEdit = new QLineEdit(this);
    m_authorEdit->setPlaceholderText("Author name");
    
    m_versionEdit = new QLineEdit(this);
    m_versionEdit->setPlaceholderText("Version (e.g., 1.0.0)");
    
    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setPlaceholderText("Component description");
    m_descriptionEdit->setMaximumHeight(80);
    
    m_tagsEdit = new QLineEdit(this);
    m_tagsEdit->setPlaceholderText("Tags (comma-separated)");
    
    m_basicLayout->addRow("Name:", m_nameEdit);
    m_basicLayout->addRow("Type:", m_typeEdit);
    m_basicLayout->addRow("Author:", m_authorEdit);
    m_basicLayout->addRow("Version:", m_versionEdit);
    m_basicLayout->addRow("Description:", m_descriptionEdit);
    m_basicLayout->addRow("Tags:", m_tagsEdit);
    
    // Geometry Group
    m_geometryGroup = new QGroupBox("Geometry", this);
    m_geometryLayout = new QFormLayout(m_geometryGroup);
    
    m_xPosEdit = new QLineEdit(this);
    m_xPosEdit->setValidator(new QDoubleValidator(-9999, 9999, 2, this));
    
    m_yPosEdit = new QLineEdit(this);
    m_yPosEdit->setValidator(new QDoubleValidator(-9999, 9999, 2, this));
    
    m_widthEdit = new QLineEdit(this);
    m_widthEdit->setValidator(new QDoubleValidator(10, 1000, 2, this));
    
    m_heightEdit = new QLineEdit(this);
    m_heightEdit->setValidator(new QDoubleValidator(10, 1000, 2, this));
    
    m_rotationEdit = new QLineEdit(this);
    m_rotationEdit->setValidator(new QDoubleValidator(-360, 360, 2, this));
    
    m_geometryLayout->addRow("X Position:", m_xPosEdit);
    m_geometryLayout->addRow("Y Position:", m_yPosEdit);
    m_geometryLayout->addRow("Width:", m_widthEdit);
    m_geometryLayout->addRow("Height:", m_heightEdit);
    m_geometryLayout->addRow("Rotation:", m_rotationEdit);
    
    // Appearance Group
    m_appearanceGroup = new QGroupBox("Appearance", this);
    m_appearanceLayout = new QFormLayout(m_appearanceGroup);
    
    m_colorEdit = new QLineEdit(this);
    m_colorEdit->setPlaceholderText("#RRGGBB");
    
    m_opacityEdit = new QLineEdit(this);
    m_opacityEdit->setValidator(new QDoubleValidator(0.0, 1.0, 2, this));
    m_opacityEdit->setText("1.0");
    
    m_visibleCheck = new QCheckBox("Visible", this);
    m_visibleCheck->setChecked(true);
    
    m_enabledCheck = new QCheckBox("Enabled", this);
    m_enabledCheck->setChecked(true);
    
    m_editableCheck = new QCheckBox("Editable", this);
    m_editableCheck->setChecked(true);
    
    m_lockedCheck = new QCheckBox("Locked", this);
    m_lockedCheck->setChecked(false);
    
    m_appearanceLayout->addRow("Color:", m_colorEdit);
    m_appearanceLayout->addRow("Opacity:", m_opacityEdit);
    m_appearanceLayout->addRow("", m_visibleCheck);
    m_appearanceLayout->addRow("", m_enabledCheck);
    m_appearanceLayout->addRow("", m_editableCheck);
    m_appearanceLayout->addRow("", m_lockedCheck);
    
    // Buttons
    m_buttonLayout = new QHBoxLayout();
    
    m_saveButton = new QPushButton("Save Changes", this);
    m_saveButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px; border-radius: 4px; }");
    
    m_cancelButton = new QPushButton("Cancel", this);
    m_cancelButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 8px; border-radius: 4px; }");
    
    m_previewButton = new QPushButton("Preview JSON", this);
    m_previewButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px; border-radius: 4px; }");
    
    m_buttonLayout->addWidget(m_saveButton);
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addWidget(m_previewButton);
    m_buttonLayout->addStretch();
    
    // Add to main layout
    m_mainLayout->addWidget(m_basicGroup);
    m_mainLayout->addWidget(m_geometryGroup);
    m_mainLayout->addWidget(m_appearanceGroup);
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Set font for Arabic text support
    setStyleSheet("font-family: 'Tajawal', sans-serif;");
}

void ComponentMetadataEditor::setupConnections()
{
    connect(m_saveButton, &QPushButton::clicked, this, &ComponentMetadataEditor::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &ComponentMetadataEditor::onCancelClicked);
    connect(m_previewButton, &QPushButton::clicked, this, &ComponentMetadataEditor::updatePreview);
    
    // Connect field changes
    connect(m_nameEdit, &QLineEdit::textChanged, this, &ComponentMetadataEditor::onFieldChanged);
    connect(m_authorEdit, &QLineEdit::textChanged, this, &ComponentMetadataEditor::onFieldChanged);
    connect(m_versionEdit, &QLineEdit::textChanged, this, &ComponentMetadataEditor::onFieldChanged);
    connect(m_descriptionEdit, &QTextEdit::textChanged, this, &ComponentMetadataEditor::onFieldChanged);
    connect(m_tagsEdit, &QLineEdit::textChanged, this, &ComponentMetadataEditor::onFieldChanged);
    
    connect(m_xPosEdit, &QLineEdit::textChanged, this, &ComponentMetadataEditor::onFieldChanged);
    connect(m_yPosEdit, &QLineEdit::textChanged, this, &ComponentMetadataEditor::onFieldChanged);
    connect(m_widthEdit, &QLineEdit::textChanged, this, &ComponentMetadataEditor::onFieldChanged);
    connect(m_heightEdit, &QLineEdit::textChanged, this, &ComponentMetadataEditor::onFieldChanged);
    connect(m_rotationEdit, &QLineEdit::textChanged, this, &ComponentMetadataEditor::onFieldChanged);
    
    connect(m_colorEdit, &QLineEdit::textChanged, this, &ComponentMetadataEditor::onFieldChanged);
    connect(m_opacityEdit, &QLineEdit::textChanged, this, &ComponentMetadataEditor::onFieldChanged);
    
    connect(m_visibleCheck, &QCheckBox::toggled, this, &ComponentMetadataEditor::onFieldChanged);
    connect(m_enabledCheck, &QCheckBox::toggled, this, &ComponentMetadataEditor::onFieldChanged);
    connect(m_editableCheck, &QCheckBox::toggled, this, &ComponentMetadataEditor::onFieldChanged);
    connect(m_lockedCheck, &QCheckBox::toggled, this, &ComponentMetadataEditor::onFieldChanged);
}

void ComponentMetadataEditor::setComponentId(const QString& componentId)
{
    m_componentId = componentId;
    setWindowTitle(QString("Component Metadata Editor - %1").arg(componentId));
}

void ComponentMetadataEditor::setMetadata(const QJsonObject& metadata)
{
    m_metadata = metadata;
    loadMetadataToUI();
}

QJsonObject ComponentMetadataEditor::getMetadata() const
{
    return m_metadata;
}

void ComponentMetadataEditor::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
    
    m_nameEdit->setReadOnly(readOnly);
    m_authorEdit->setReadOnly(readOnly);
    m_versionEdit->setReadOnly(readOnly);
    m_descriptionEdit->setReadOnly(readOnly);
    m_tagsEdit->setReadOnly(readOnly);
    
    m_xPosEdit->setReadOnly(readOnly);
    m_yPosEdit->setReadOnly(readOnly);
    m_widthEdit->setReadOnly(readOnly);
    m_heightEdit->setReadOnly(readOnly);
    m_rotationEdit->setReadOnly(readOnly);
    
    m_colorEdit->setReadOnly(readOnly);
    m_opacityEdit->setReadOnly(readOnly);
    
    m_visibleCheck->setEnabled(!readOnly);
    m_enabledCheck->setEnabled(!readOnly);
    m_editableCheck->setEnabled(!readOnly);
    m_lockedCheck->setEnabled(!readOnly);
    
    m_saveButton->setVisible(!readOnly);
}

void ComponentMetadataEditor::loadMetadataToUI()
{
    if (m_isUpdating) return;
    m_isUpdating = true;
    
    // Basic information
    m_nameEdit->setText(m_metadata["id"].toString());
    m_typeEdit->setText(m_metadata["type"].toString());
    
    // Component details
    QJsonObject details = m_metadata["componentDetails"].toObject();
    m_authorEdit->setText(details["author"].toString());
    m_versionEdit->setText(details["version"].toString());
    m_descriptionEdit->setPlainText(details["description"].toString());
    
    // Tags
    QJsonArray tags = details["tags"].toArray();
    QStringList tagList;
    for (const QJsonValue& tag : tags) {
        tagList << tag.toString();
    }
    m_tagsEdit->setText(tagList.join(", "));
    
    // Geometry
    QJsonObject geometry = m_metadata["geometry"].toObject();
    QJsonObject position = geometry["position"].toObject();
    QJsonObject size = geometry["size"].toObject();
    
    m_xPosEdit->setText(QString::number(position["x"].toDouble()));
    m_yPosEdit->setText(QString::number(position["y"].toDouble()));
    m_widthEdit->setText(QString::number(size["width"].toDouble()));
    m_heightEdit->setText(QString::number(size["height"].toDouble()));
    m_rotationEdit->setText(QString::number(geometry["rotation"].toDouble()));
    
    // Appearance
    QJsonObject appearance = m_metadata["appearance"].toObject();
    m_colorEdit->setText(appearance["color"].toString());
    m_opacityEdit->setText(QString::number(appearance["opacity"].toDouble()));
    m_visibleCheck->setChecked(appearance["visible"].toBool());
    
    // Properties
    QJsonObject properties = m_metadata["properties"].toObject();
    m_enabledCheck->setChecked(properties["enabled"].toBool());
    m_editableCheck->setChecked(details["editable"].toBool());
    m_lockedCheck->setChecked(properties["locked"].toBool());
    
    m_isUpdating = false;
}

void ComponentMetadataEditor::updateMetadataFromUI()
{
    // Update basic information
    m_metadata["id"] = m_nameEdit->text();
    m_metadata["type"] = m_typeEdit->text();
    
    // Update component details
    QJsonObject details = m_metadata["componentDetails"].toObject();
    details["author"] = m_authorEdit->text();
    details["version"] = m_versionEdit->text();
    details["description"] = m_descriptionEdit->toPlainText();
    
    // Update tags
    QStringList tagList = m_tagsEdit->text().split(",", Qt::SkipEmptyParts);
    QJsonArray tags;
    for (const QString& tag : tagList) {
        tags.append(tag.trimmed());
    }
    details["tags"] = tags;
    details["editable"] = m_editableCheck->isChecked();
    m_metadata["componentDetails"] = details;
    
    // Update geometry
    QJsonObject geometry = m_metadata["geometry"].toObject();
    QJsonObject position;
    position["x"] = m_xPosEdit->text().toDouble();
    position["y"] = m_yPosEdit->text().toDouble();
    geometry["position"] = position;
    
    QJsonObject size;
    size["width"] = m_widthEdit->text().toDouble();
    size["height"] = m_heightEdit->text().toDouble();
    geometry["size"] = size;
    geometry["rotation"] = m_rotationEdit->text().toDouble();
    m_metadata["geometry"] = geometry;
    
    // Update appearance
    QJsonObject appearance = m_metadata["appearance"].toObject();
    appearance["color"] = m_colorEdit->text();
    appearance["opacity"] = m_opacityEdit->text().toDouble();
    appearance["visible"] = m_visibleCheck->isChecked();
    m_metadata["appearance"] = appearance;
    
    // Update properties
    QJsonObject properties = m_metadata["properties"].toObject();
    properties["enabled"] = m_enabledCheck->isChecked();
    properties["locked"] = m_lockedCheck->isChecked();
    m_metadata["properties"] = properties;
    
    // Update modification timestamp
    m_metadata["modified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
}

void ComponentMetadataEditor::onSaveClicked()
{
    updateMetadataFromUI();
    
    // Validate required fields
    if (m_nameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Component name is required.");
        return;
    }
    
    // Validate color format
    QString colorText = m_colorEdit->text();
    if (!colorText.isEmpty() && !QColor::isValidColorName(colorText) && !colorText.startsWith("#")) {
        QMessageBox::warning(this, "Validation Error", "Invalid color format. Use color name or #RRGGBB format.");
        return;
    }
    
    emit metadataChanged(m_componentId, m_metadata);
    emit saveRequested(m_componentId);
    
    qDebug() << "Component metadata saved for:" << m_componentId;
}

void ComponentMetadataEditor::onCancelClicked()
{
    loadMetadataToUI(); // Reset to original values
    emit cancelRequested();
}

void ComponentMetadataEditor::onFieldChanged()
{
    if (m_isUpdating) return;
    updateMetadataFromUI();
}

void ComponentMetadataEditor::updatePreview()
{
    updateMetadataFromUI();
    
    QJsonDocument doc(m_metadata);
    QString jsonString = doc.toJson(QJsonDocument::Indented);
    
    QMessageBox::information(this, "Metadata Preview", 
        QString("Component: %1\n\nMetadata JSON:\n%2").arg(m_componentId, jsonString));
}
