// EditComponentWidget.cpp
#include "ui/widgets/EditComponentWidget.h"
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <QFont>

EditComponentWidget::EditComponentWidget(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_textEdit(nullptr)
    , m_saveButton(nullptr)
    , m_closeButton(nullptr)
    , m_toolbarWidget(nullptr)
{
    setupUi();
    applyStyles();
}

EditComponentWidget::~EditComponentWidget()
{
}

void EditComponentWidget::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(5);
    
    // Create toolbar with title and buttons
    m_toolbarWidget = new QWidget(this);
    QHBoxLayout* toolbarLayout = new QHBoxLayout(m_toolbarWidget);
    toolbarLayout->setContentsMargins(5, 5, 5, 5);
    toolbarLayout->setSpacing(10);
    
    // Title label
    m_titleLabel = new QLabel("Component Editor", this);
    m_titleLabel->setStyleSheet("font-family: Tajawal; font-size: 12pt; font-weight: bold;");
    toolbarLayout->addWidget(m_titleLabel);
    
    toolbarLayout->addStretch();
    
    // Save button
    m_saveButton = new QPushButton("Save", this);
    m_saveButton->setFixedSize(80, 30);
    connect(m_saveButton, &QPushButton::clicked, this, &EditComponentWidget::onSaveClicked);
    toolbarLayout->addWidget(m_saveButton);
    
    // Close button
    m_closeButton = new QPushButton("Close", this);
    m_closeButton->setFixedSize(80, 30);
    connect(m_closeButton, &QPushButton::clicked, this, &EditComponentWidget::onCloseClicked);
    toolbarLayout->addWidget(m_closeButton);
    
    m_mainLayout->addWidget(m_toolbarWidget);
    
    // Create text editor
    m_textEdit = new QTextEdit(this);
    m_textEdit->setAcceptRichText(false);
    
    // Set monospace font for code editing
    QFont codeFont("Consolas", 10);
    codeFont.setStyleHint(QFont::Monospace);
    m_textEdit->setFont(codeFont);
    
    m_mainLayout->addWidget(m_textEdit);
    
    setLayout(m_mainLayout);
    
    // Initially hide the widget
    hide();
}

void EditComponentWidget::applyStyles()
{
    // Style the widget with a modern look
    setStyleSheet(
        "EditComponentWidget {"
        "    background-color: #2B2B2B;"
        "    border-left: 2px solid #637AB9;"
        "}"
        "QTextEdit {"
        "    background-color: #1E1E1E;"
        "    color: #D4D4D4;"
        "    border: 1px solid #3E3E42;"
        "    border-radius: 4px;"
        "    padding: 5px;"
        "    selection-background-color: #264F78;"
        "}"
        "QPushButton {"
        "    background-color: #637AB9;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-family: Tajawal;"
        "    font-size: 10pt;"
        "    padding: 5px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #7A8FC9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #505F89;"
        "}"
    );
}

void EditComponentWidget::loadComponentFile(const QString& filePath, const QString& componentName)
{
    qDebug() << "📂 EditComponentWidget::loadComponentFile() called for file:" << filePath << "component:" << componentName;
    m_currentFilePath = filePath;
    m_componentName = componentName;
    
    // Update title
    m_titleLabel->setText(QString("Edit: %1").arg(componentName));
    
    // Check if file exists
    QFile file(filePath);
    if (!file.exists()) {
        qWarning() << "File does not exist:" << filePath;
        
        // Create a template file
        QString templateContent = QString(
            "// %1 Component Implementation\n"
            "// Auto-generated file\n\n"
            "#include <systemc.h>\n\n"
            "// TODO: Implement %1 component\n\n"
            "class %1 {\n"
            "public:\n"
            "    %1() {\n"
            "        // Constructor\n"
            "    }\n"
            "    \n"
            "    ~%1() {\n"
            "        // Destructor\n"
            "    }\n"
            "};\n"
        ).arg(componentName);
        
        m_textEdit->setPlainText(templateContent);
        
        QMessageBox::information(this, "File Created", 
            QString("File did not exist. Created template for: %1\n\n"
                   "Click Save to create the file.").arg(componentName));
    } else {
        // Load file content
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Error", 
                QString("Could not open file: %1\n\nError: %2")
                    .arg(filePath)
                    .arg(file.errorString()));
            return;
        }
        
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();
        
        // Set content in editor
        m_textEdit->setPlainText(content);
        
        qDebug() << "✅ Loaded component file:" << filePath;
    }
    
    // Show the widget
    show();
}

void EditComponentWidget::clear()
{
    m_textEdit->clear();
    m_titleLabel->setText("Component Editor");
    m_currentFilePath.clear();
    m_componentName.clear();
    hide();
}

void EditComponentWidget::saveContent()
{
    qDebug() << "💾 EditComponentWidget::saveContent() called for file:" << m_currentFilePath;
    if (m_currentFilePath.isEmpty()) {
        return;
    }
    
    // Save file
    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", 
            QString("Could not save file: %1").arg(m_currentFilePath));
        return;
    }
    
    QTextStream out(&file);
    out << m_textEdit->toPlainText();
    file.close();
    
    qDebug() << "Saved component file:" << m_currentFilePath;
    
    // Emit signal
    emit componentSaved(m_currentFilePath);
    
    QMessageBox::information(this, "Success", 
        QString("Component '%1' saved successfully!").arg(m_componentName));
}

void EditComponentWidget::onSaveClicked()
{
    saveContent();
}

void EditComponentWidget::onCloseClicked()
{
    // Check if there are unsaved changes
    // For now, just close - you can add unsaved changes check later
    clear();
    emit editorClosed();
}

