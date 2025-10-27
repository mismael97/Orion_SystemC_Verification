// EditComponentWidget.h
#ifndef EDITCOMPONENTWIDGET_H
#define EDITCOMPONENTWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>

class EditComponentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EditComponentWidget(QWidget* parent = nullptr);
    ~EditComponentWidget() override;

    // Load and display a component file
    void loadComponentFile(const QString& filePath, const QString& componentName);
    
    // Clear the editor
    void clear();
    
    // Save current content
    void saveContent();
    
    // Check if editor is visible/active
    bool isEditingComponent() const { return !m_currentFilePath.isEmpty(); }
    
    // Get current file path
    QString currentFilePath() const { return m_currentFilePath; }

signals:
    void componentSaved(const QString& filePath);
    void editorClosed();

private slots:
    void onSaveClicked();
    void onCloseClicked();

private:
    void setupUi();
    void applyStyles();

    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QTextEdit* m_textEdit;
    QPushButton* m_saveButton;
    QPushButton* m_closeButton;
    QWidget* m_toolbarWidget;
    
    QString m_currentFilePath;
    QString m_componentName;
};

#endif // EDITCOMPONENTWIDGET_H

