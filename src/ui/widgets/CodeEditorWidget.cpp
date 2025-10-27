// CodeEditorWidget.cpp
#include "ui/widgets/CodeEditorWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFont>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QFileInfo>
#include <QShortcut>

// Simple syntax highlighter for SystemVerilog/C++/SystemC
class SyntaxHighlighter : public QSyntaxHighlighter
{
public:
    enum Theme {
        Light,
        Monokai
    };

    SyntaxHighlighter(QTextDocument* parent = nullptr)
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
        
        // Keywords (covers C++, SystemC, and Verilog/SystemVerilog)
        keywordFormat.setForeground(keywordColor);
        keywordFormat.setFontWeight(QFont::Bold);
        QStringList keywordPatterns = {
            // C++/SystemC keywords
            "\\bclass\\b", "\\bstruct\\b", "\\bvoid\\b", "\\bint\\b", "\\bbool\\b",
            "\\bif\\b", "\\belse\\b", "\\bfor\\b", "\\bwhile\\b", "\\breturn\\b",
            "\\bpublic\\b", "\\bprivate\\b", "\\bprotected\\b", "\\bvirtual\\b",
            "\\bconst\\b", "\\bstatic\\b", "\\bnamespace\\b", "\\btemplate\\b",
            "\\btypedef\\b", "\\btypename\\b", "\\boperator\\b", "\\bnew\\b",
            "\\bdelete\\b", "\\bthis\\b", "\\btrue\\b", "\\bfalse\\b",
            "\\bSC_MODULE\\b", "\\bSC_CTOR\\b", "\\bsc_in\\b", "\\bsc_out\\b",
            "\\bsc_uint\\b", "\\bsc_int\\b", "\\bsc_signal\\b",
            // Verilog/SystemVerilog keywords
            "\\bmodule\\b", "\\bendmodule\\b", "\\binput\\b", "\\boutput\\b",
            "\\binout\\b", "\\bwire\\b", "\\breg\\b", "\\binteger\\b",
            "\\balways\\b", "\\bassign\\b", "\\bbegin\\b", "\\bend\\b",
            "\\bcase\\b", "\\bendcase\\b", "\\bdefault\\b", "\\bparameter\\b",
            "\\bposedge\\b", "\\bnegedge\\b", "\\bor\\b", "\\band\\b",
            "\\blogic\\b", "\\bbit\\b", "\\bbyte\\b", "\\bshortint\\b",
            "\\bint\\b", "\\blongint\\b", "\\btime\\b"
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
        preprocessorRule.pattern = QRegularExpression("^\\s*[#`][^\n]*");
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

CodeEditorWidget::CodeEditorWidget(const QString& filePath, QWidget* parent)
    : QWidget(parent), m_filePath(filePath), m_modified(false), m_highlighter(nullptr)
{
    setupUI();
    loadCode();
}

void CodeEditorWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);
    
    // Header section
    QHBoxLayout* headerLayout = new QHBoxLayout();
    
    // File path label
    m_filePathLabel = new QLabel(m_filePath);
    m_filePathLabel->setStyleSheet("font-size: 9pt; color: gray; font-family: Tajawal;");
    m_filePathLabel->setWordWrap(true);
    headerLayout->addWidget(m_filePathLabel, 1);
    
    // Theme toggle button
    m_themeButton = new QPushButton("🌙 Light Theme");
    m_themeButton->setStyleSheet("font-family: Tajawal; padding: 5px 15px;");
    m_themeButton->setToolTip("Toggle between Light and Monokai themes");
    connect(m_themeButton, &QPushButton::clicked, this, &CodeEditorWidget::onThemeToggled);
    headerLayout->addWidget(m_themeButton);
    
    // Save button
    m_saveButton = new QPushButton("Save (Ctrl+S)");
    m_saveButton->setStyleSheet("font-family: Tajawal; padding: 5px 15px;");
    m_saveButton->setToolTip("Save changes to file");
    m_saveButton->setEnabled(false);
    connect(m_saveButton, &QPushButton::clicked, this, &CodeEditorWidget::onSaveClicked);
    headerLayout->addWidget(m_saveButton);
    
    mainLayout->addLayout(headerLayout);
    
    // Code editor
    m_codeEditor = new QPlainTextEdit(this);
    applyCodeEditorStyle();
    
    // Add syntax highlighting
    m_highlighter = new SyntaxHighlighter(m_codeEditor->document());
    
    // Track modifications
    connect(m_codeEditor, &QPlainTextEdit::textChanged, this, &CodeEditorWidget::onTextChanged);
    
    // Add Ctrl+S shortcut for saving
    QShortcut* saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, this, &CodeEditorWidget::onSaveClicked);
    
    mainLayout->addWidget(m_codeEditor, 1);
    
    // Status label
    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("font-family: Tajawal; padding: 3px; font-size: 9pt;");
    mainLayout->addWidget(m_statusLabel);
    
    setLayout(mainLayout);
}

void CodeEditorWidget::applyCodeEditorStyle()
{
    // Set monospace font
    QFont font("Courier New", 10);
    font.setStyleHint(QFont::Monospace);
    m_codeEditor->setFont(font);
    
    // Set tab width to 4 spaces
    QFontMetrics metrics(font);
    m_codeEditor->setTabStopDistance(4 * metrics.horizontalAdvance(' '));
    
    // Apply theme-specific styles based on highlighter theme
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

void CodeEditorWidget::onThemeToggled()
{
    // Toggle theme
    m_isLightTheme = !m_isLightTheme;
    
    if (m_isLightTheme) {
        m_themeButton->setText("🌑 Monokai Theme");
        if (m_highlighter) {
            m_highlighter->setTheme(SyntaxHighlighter::Light);
        }
    } else {
        m_themeButton->setText("🌙 Light Theme");
        if (m_highlighter) {
            m_highlighter->setTheme(SyntaxHighlighter::Monokai);
        }
    }
    
    applyCodeEditorStyle();
}

void CodeEditorWidget::loadCode()
{
    qDebug() << "📂 CodeEditorWidget::loadCode() called for file:" << m_filePath;
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_statusLabel->setText("Error: Could not load file");
        m_statusLabel->setStyleSheet("color: red; font-family: Tajawal; padding: 3px; font-size: 9pt;");
        m_codeEditor->setPlainText("// Error: Could not open file\n// " + m_filePath);
        m_codeEditor->setReadOnly(true);
        return;
    }
    
    QTextStream in(&file);
    QString code = in.readAll();
    file.close();
    
    m_codeEditor->setPlainText(code);
    m_codeEditor->setReadOnly(false);  // Ensure editor is editable
    m_modified = false;
    m_statusLabel->setText("File loaded: " + getFileName());
    m_statusLabel->setStyleSheet("color: green; font-family: Tajawal; padding: 3px; font-size: 9pt;");
}

void CodeEditorWidget::saveCode()
{
    qDebug() << "💾 CodeEditorWidget::saveCode() called for file:" << m_filePath;
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to save file: " + m_filePath);
        m_statusLabel->setText("Error: Could not save file");
        m_statusLabel->setStyleSheet("color: red; font-family: Tajawal; padding: 3px; font-size: 9pt;");
        return;
    }
    
    QTextStream out(&file);
    out << m_codeEditor->toPlainText();
    file.close();
    
    m_modified = false;
    m_saveButton->setEnabled(false);
    m_statusLabel->setText("Saved successfully!");
    m_statusLabel->setStyleSheet("color: green; font-family: Tajawal; padding: 3px; font-size: 9pt;");
    
    emit fileModified(false);
    emit fileSaved();
}

void CodeEditorWidget::onSaveClicked()
{
    if (m_modified) {
        saveCode();
    }
}

void CodeEditorWidget::onTextChanged()
{
    if (!m_modified) {
        m_modified = true;
        m_saveButton->setEnabled(true);
        m_statusLabel->setText("Modified - Remember to save!");
        m_statusLabel->setStyleSheet("color: orange; font-family: Tajawal; padding: 3px; font-size: 9pt;");
        emit fileModified(true);
    }
}

QString CodeEditorWidget::getCode() const
{
    return m_codeEditor->toPlainText();
}

bool CodeEditorWidget::isModified() const
{
    return m_modified;
}

QString CodeEditorWidget::getFileName() const
{
    return QFileInfo(m_filePath).fileName();
}
