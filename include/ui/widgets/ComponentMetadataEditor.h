// ComponentMetadataEditor.h
#ifndef COMPONENTMETADATAEDITOR_H
#define COMPONENTMETADATAEDITOR_H

#include <QWidget>
#include <QJsonObject>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QGroupBox>

class ComponentMetadataEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ComponentMetadataEditor(QWidget *parent = nullptr);
    
    void setComponentId(const QString& componentId);
    void setMetadata(const QJsonObject& metadata);
    QJsonObject getMetadata() const;
    
    void setReadOnly(bool readOnly);
    bool isReadOnly() const { return m_readOnly; }

signals:
    void metadataChanged(const QString& componentId, const QJsonObject& metadata);
    void saveRequested(const QString& componentId);
    void cancelRequested();

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onFieldChanged();
    void updatePreview();

private:
    void setupUI();
    void loadMetadataToUI();
    void updateMetadataFromUI();
    void setupConnections();
    
    QString m_componentId;
    QJsonObject m_metadata;
    bool m_readOnly = false;
    bool m_isUpdating = false;
    
    // UI Elements
    QLineEdit* m_nameEdit;
    QLineEdit* m_typeEdit;
    QLineEdit* m_authorEdit;
    QLineEdit* m_versionEdit;
    QTextEdit* m_descriptionEdit;
    QLineEdit* m_tagsEdit;
    QCheckBox* m_editableCheck;
    QCheckBox* m_visibleCheck;
    QCheckBox* m_enabledCheck;
    QCheckBox* m_lockedCheck;
    
    // Position and Size
    QLineEdit* m_xPosEdit;
    QLineEdit* m_yPosEdit;
    QLineEdit* m_widthEdit;
    QLineEdit* m_heightEdit;
    QLineEdit* m_rotationEdit;
    
    // Appearance
    QLineEdit* m_colorEdit;
    QLineEdit* m_opacityEdit;
    
    // Buttons
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
    QPushButton* m_previewButton;
    
    // Layouts
    QVBoxLayout* m_mainLayout;
    QFormLayout* m_basicLayout;
    QFormLayout* m_geometryLayout;
    QFormLayout* m_appearanceLayout;
    QHBoxLayout* m_buttonLayout;
    
    // Group boxes
    QGroupBox* m_basicGroup;
    QGroupBox* m_geometryGroup;
    QGroupBox* m_appearanceGroup;
};

#endif // COMPONENTMETADATAEDITOR_H
