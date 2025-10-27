// ComponentPropertiesDialog.h
#ifndef COMPONENTPROPERTIESDIALOG_H
#define COMPONENTPROPERTIESDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include "ui/widgets/ComponentMetadataEditor.h"

class ComponentPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ComponentPropertiesDialog(QWidget *parent = nullptr);
    
    void setComponentId(const QString& componentId);
    void setMetadata(const QJsonObject& metadata);
    QJsonObject getUpdatedMetadata() const;
    
    static ComponentPropertiesDialog* editComponent(const QString& componentId, 
                                                   const QJsonObject& metadata, 
                                                   QWidget* parent = nullptr);

signals:
    void metadataUpdated(const QString& componentId, const QJsonObject& metadata);

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onMetadataChanged(const QString& componentId, const QJsonObject& metadata);

private:
    void setupUI();
    void setupConnections();
    
    QString m_componentId;
    QJsonObject m_originalMetadata;
    QJsonObject m_currentMetadata;
    
    ComponentMetadataEditor* m_metadataEditor;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
    QPushButton* m_applyButton;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;
};

#endif // COMPONENTPROPERTIESDIALOG_H
