// CodeEditorWidget.h
#ifndef CODEEDITORWIDGET_H
#define CODEEDITORWIDGET_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>

// Forward declaration
class SyntaxHighlighter;

class CodeEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CodeEditorWidget(const QString& filePath, QWidget* parent = nullptr);
    
    QString getCode() const;
    bool isModified() const;
    QString getFilePath() const { return m_filePath; }
    QString getFileName() const;

public slots:
    void onSaveClicked();
    void onThemeToggled();

signals:
    void fileModified(bool modified);
    void fileSaved();

private slots:
    void onTextChanged();

private:
    QString m_filePath;
    QPlainTextEdit* m_codeEditor;
    QPushButton* m_saveButton;
    QPushButton* m_themeButton;
    QLabel* m_statusLabel;
    QLabel* m_filePathLabel;
    bool m_modified;
    bool m_isLightTheme = false;
    SyntaxHighlighter* m_highlighter;
    
    void setupUI();
    void loadCode();
    void saveCode();
    void applyCodeEditorStyle();
};

#endif // CODEEDITORWIDGET_H
