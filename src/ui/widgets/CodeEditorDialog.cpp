// CodeEditorDialog.cpp
#include "ui/widgets/CodeEditorDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFont>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

// Simple syntax highlighter for C++/SystemC
class CppHighlighter : public QSyntaxHighlighter
{
public:
    enum Theme {
        Light,
        Monokai
    };

    CppHighlighter(QTextDocument* parent = nullptr)
        : QSyntaxHighlighter(parent), m_theme(Monokai)
    {
        updateTheme();
    }

    void setTheme(Theme theme)
    {
        m_theme = theme;
        updateTheme();
        rehighlight();
    }

private:
    void updateTheme()
    {
        highlightingRules.clear();
        
        // Define color schemes based on theme
        QColor keywordColor, preprocessorColor, commentColor, stringColor, numberColor;
        
        if (m_theme == Light) {
            // Light theme colors
            keywordColor = QColor(0, 0, 255);           // Blue
            preprocessorColor = QColor(128, 0, 128);    // Purple
            commentColor = QColor(0, 128, 0);           // Green
            stringColor = QColor(163, 21, 21);          // Dark Red
            numberColor = QColor(9, 134, 88);           // Teal
        } else {
            // Monokai theme colors
            keywordColor = QColor(249, 38, 114);        // Pink
            preprocessorColor = QColor(166, 226, 46);   // Light Green
            commentColor = QColor(117, 113, 94);        // Gray
            stringColor = QColor(230, 219, 116);        // Yellow
            numberColor = QColor(174, 129, 255);        // Purple
        }
        
        // Keywords
        keywordFormat.setForeground(keywordColor);
        keywordFormat.setFontWeight(QFont::Bold);
        QStringList keywordPatterns = {
            "\\bclass\\b", "\\bstruct\\b", "\\bvoid\\b", "\\bint\\b", "\\bbool\\b",
            "\\bif\\b", "\\belse\\b", "\\bfor\\b", "\\bwhile\\b", "\\breturn\\b",
            "\\bpublic\\b", "\\bprivate\\b", "\\bprotected\\b", "\\bvirtual\\b",
            "\\bconst\\b", "\\bstatic\\b", "\\bnamespace\\b", "\\btemplate\\b",
            "\\btypedef\\b", "\\btypename\\b", "\\boperator\\b", "\\bnew\\b",
            "\\bdelete\\b", "\\bthis\\b", "\\btrue\\b", "\\bfalse\\b",
            "\\bSC_MODULE\\b", "\\bSC_CTOR\\b", "\\bsc_in\\b", "\\bsc_out\\b",
            "\\bsc_uint\\b", "\\bsc_int\\b", "\\bsc_signal\\b"
        };
        
        for (const QString& pattern : keywordPatterns) {
            HighlightingRule rule;
            rule.pattern = QRegularExpression(pattern);
            rule.format = keywordFormat;
            highlightingRules.append(rule);
        }
        
        // Preprocessor
        preprocessorFormat.setForeground(preprocessorColor);
        HighlightingRule preprocessorRule;
        preprocessorRule.pattern = QRegularExpression("^\\s*#[^\n]*");
        preprocessorRule.format = preprocessorFormat;
        highlightingRules.append(preprocessorRule);
        
        // Single line comments
        singleLineCommentFormat.setForeground(commentColor);
        HighlightingRule commentRule;
        commentRule.pattern = QRegularExpression("//[^\n]*");
        commentRule.format = singleLineCommentFormat;
        highlightingRules.append(commentRule);
        
        // Strings
        quotationFormat.setForeground(stringColor);
        HighlightingRule stringRule;
        stringRule.pattern = QRegularExpression("\".*\"");
        stringRule.format = quotationFormat;
        highlightingRules.append(stringRule);
        
        // Numbers
        numberFormat.setForeground(numberColor);
        HighlightingRule numberRule;
        numberRule.pattern = QRegularExpression("\\b[0-9]+(\\.[0-9]+)?\\b");
        numberRule.format = numberFormat;
        highlightingRules.append(numberRule);
    }

protected:
    void highlightBlock(const QString& text) override
    {
        for (const HighlightingRule& rule : highlightingRules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }

    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
    
    QTextCharFormat keywordFormat;
    QTextCharFormat preprocessorFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat numberFormat;
    Theme m_theme;
};

CodeEditorDialog::CodeEditorDialog(const QString& componentId, const QString& componentType,
                                   const QString& filePath, QWidget* parent)
    : QDialog(parent), m_componentId(componentId), m_componentType(componentType),
      m_filePath(filePath), m_modified(false), m_highlighter(nullptr)
{
    setupUI();
    loadCode();
    
    setWindowTitle("Edit Component: " + componentId);
    resize(800, 600);
}

void CodeEditorDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Header section
    QHBoxLayout* headerLayout = new QHBoxLayout();
    
    QLabel* headerLabel = new QLabel("SystemC Code Editor - " + m_componentType);
    headerLabel->setStyleSheet("font-size: 14pt; font-weight: bold; font-family: Tajawal; padding: 10px;");
    headerLayout->addWidget(headerLabel, 1);
    
    // Theme toggle button
    m_themeButton = new QPushButton("🌙 Light Theme");
    m_themeButton->setStyleSheet("font-family: Tajawal; padding: 5px 15px;");
    m_themeButton->setToolTip("Toggle between Light and Monokai themes");
    connect(m_themeButton, &QPushButton::clicked, this, &CodeEditorDialog::onThemeToggled);
    headerLayout->addWidget(m_themeButton);
    
    mainLayout->addLayout(headerLayout);
    
    // Code editor
    m_codeEditor = new QPlainTextEdit(this);
    applyCodeEditorStyle();
    
    // Add syntax highlighting
    m_highlighter = new CppHighlighter(m_codeEditor->document());
    
    // Track modifications
    connect(m_codeEditor, &QPlainTextEdit::textChanged, this, [this]() {
        m_modified = true;
        m_statusLabel->setText("Modified - Remember to save!");
        m_statusLabel->setStyleSheet("color: orange; font-family: Tajawal;");
    });
    
    mainLayout->addWidget(m_codeEditor);
    
    // Status label
    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("font-family: Tajawal; padding: 5px;");
    mainLayout->addWidget(m_statusLabel);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_saveButton = new QPushButton("Save");
    m_saveButton->setStyleSheet("font-family: Tajawal;");
    m_saveButton->setToolTip("Save changes to file");
    connect(m_saveButton, &QPushButton::clicked, this, &CodeEditorDialog::onSaveClicked);
    buttonLayout->addWidget(m_saveButton);
    
    m_saveAndCloseButton = new QPushButton("Save && Close");
    m_saveAndCloseButton->setStyleSheet("font-family: Tajawal;");
    m_saveAndCloseButton->setToolTip("Save changes and close editor");
    connect(m_saveAndCloseButton, &QPushButton::clicked, this, &CodeEditorDialog::onSaveAndCloseClicked);
    buttonLayout->addWidget(m_saveAndCloseButton);
    
    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setStyleSheet("font-family: Tajawal;");
    m_cancelButton->setToolTip("Close without saving");
    connect(m_cancelButton, &QPushButton::clicked, this, &CodeEditorDialog::onCancelClicked);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
}

void CodeEditorDialog::applyCodeEditorStyle()
{
    // Set monospace font
    QFont font("Courier New", 10);
    font.setStyleHint(QFont::Monospace);
    m_codeEditor->setFont(font);
    
    // Set tab width to 4 spaces
    QFontMetrics metrics(font);
    m_codeEditor->setTabStopDistance(4 * metrics.horizontalAdvance(' '));
    
    // Apply theme-specific styles
    QString styleSheet;
    if (m_isLightTheme) {
        // Light theme
        styleSheet =
            "QPlainTextEdit {"
            "    background-color: #ffffff;"
            "    color: #000000;"
            "    border: 1px solid #cccccc;"
            "    border-radius: 4px;"
            "    padding: 5px;"
            "}";
    } else {
        // Monokai theme
        styleSheet =
            "QPlainTextEdit {"
            "    background-color: #272822;"
            "    color: #f8f8f2;"
            "    border: 1px solid #3c3c3c;"
            "    border-radius: 4px;"
            "    padding: 5px;"
            "}";
    }
    
    m_codeEditor->setStyleSheet(styleSheet);
}

void CodeEditorDialog::onThemeToggled()
{
    // Toggle theme
    m_isLightTheme = !m_isLightTheme;
    
    if (m_isLightTheme) {
        m_themeButton->setText("🌑 Monokai Theme");
        if (m_highlighter) {
            m_highlighter->setTheme(CppHighlighter::Light);
        }
    } else {
        m_themeButton->setText("🌙 Light Theme");
        if (m_highlighter) {
            m_highlighter->setTheme(CppHighlighter::Monokai);
        }
    }
    
    applyCodeEditorStyle();
}

void CodeEditorDialog::loadCode()
{
    qDebug() << "📂 CodeEditorDialog::loadCode() called for file:" << m_filePath;
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Failed to open file: " + m_filePath);
        m_statusLabel->setText("Error: Could not load file");
        m_statusLabel->setStyleSheet("color: red; font-family: Tajawal;");
        return;
    }
    
    QTextStream in(&file);
    QString code = in.readAll();
    file.close();
    
    m_codeEditor->setPlainText(code);
    m_codeEditor->setReadOnly(false);  // Ensure editor is editable
    m_modified = false;
    m_statusLabel->setText("File loaded: " + m_filePath);
    m_statusLabel->setStyleSheet("color: green; font-family: Tajawal;");
}

void CodeEditorDialog::saveCode()
{
    qDebug() << "💾 CodeEditorDialog::saveCode() called for file:" << m_filePath;
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to save file: " + m_filePath);
        m_statusLabel->setText("Error: Could not save file");
        m_statusLabel->setStyleSheet("color: red; font-family: Tajawal;");
        return;
    }
    
    QTextStream out(&file);
    out << m_codeEditor->toPlainText();
    file.close();
    
    m_modified = false;
    m_statusLabel->setText("Saved successfully!");
    m_statusLabel->setStyleSheet("color: green; font-family: Tajawal;");
}

void CodeEditorDialog::onSaveClicked()
{
    saveCode();
}

void CodeEditorDialog::onSaveAndCloseClicked()
{
    saveCode();
    accept();
}

void CodeEditorDialog::onCancelClicked()
{
    if (m_modified) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Unsaved Changes",
            "You have unsaved changes. Are you sure you want to close?",
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::No) {
            return;
        }
    }
    
    reject();
}

QString CodeEditorDialog::getCode() const
{
    return m_codeEditor->toPlainText();
}

bool CodeEditorDialog::isModified() const
{
    return m_modified;
}

