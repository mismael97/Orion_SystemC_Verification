// CodeEditorDialog.h
#ifndef CODEEDITORDIALOG_H
#define CODEEDITORDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>

// Forward declaration
class CppHighlighter;

class CodeEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CodeEditorDialog(const QString& componentId, const QString& componentType, 
                             const QString& filePath, QWidget* parent = nullptr);
    
    QString getCode() const;
    bool isModified() const;

private slots:
    void onSaveClicked();
    void onSaveAndCloseClicked();
    void onCancelClicked();
    void onThemeToggled();

private:
    QString m_componentId;
    QString m_componentType;
    QString m_filePath;
    QPlainTextEdit* m_codeEditor;
    QPushButton* m_saveButton;
    QPushButton* m_saveAndCloseButton;
    QPushButton* m_cancelButton;
    QPushButton* m_themeButton;
    QLabel* m_statusLabel;
    bool m_modified;
    bool m_isLightTheme = false;
    CppHighlighter* m_highlighter;
    
    void setupUI();
    void loadCode();
    void saveCode();
    void applyCodeEditorStyle();
};

#endif // CODEEDITORDIALOG_H

