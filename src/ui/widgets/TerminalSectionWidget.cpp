/**
 * @file TerminalSectionWidget.cpp
 * @brief Implementation of VS Code-style terminal section widget
 */

#include "ui/widgets/TerminalSectionWidget.h"
#include <QApplication>
#include <QFont>
#include <QColor>
#include <QPalette>
#include <QScrollBar>
#include <QTimer>
#include <QMessageBox>
#include <QInputDialog>
#include <QClipboard>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QComboBox>
#include <QDateTime>
#include <QThread>
#include <QFileInfo>
#include <QSettings>
#include <QRegularExpression>
#include <QDir>
// #include <QTermWidget> // Commented out for now - requires external dependencies

// TerminalSession Implementation
TerminalSession::TerminalSession(QWidget* parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_terminal(nullptr)
    , m_statusLayout(nullptr)
    , m_statusLabel(nullptr)
    , m_workingDirLabel(nullptr)
    , m_shellLabel(nullptr)
    , m_processStatusLabel(nullptr)
    , m_process(nullptr)
    , m_isActive(false)
    , m_hasUnsavedChanges(false)
    , m_historyIndex(-1)
    , m_promptPosition(0)
    , m_ansiMode(true)
    , m_contextMenu(nullptr)
{
    setupUI();
    setupContextMenu();
    setupShell();
}

TerminalSession::~TerminalSession()
{
    closeTerminal();
}

void TerminalSession::setSessionName(const QString& name)
{
    m_sessionName = name;
    updateTitle();
}

QString TerminalSession::sessionName() const
{
    return m_sessionName;
}

void TerminalSession::startTerminal()
{
    if (!m_process) {
        m_process = new QProcess(this);
        setupShell();
        
        // Connect process signals
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &TerminalSession::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred, this, &TerminalSession::onProcessError);
        connect(m_process, &QProcess::readyReadStandardOutput,
                this, &TerminalSession::onProcessReadyReadStandardOutput);
        connect(m_process, &QProcess::readyReadStandardError,
                this, &TerminalSession::onProcessReadyReadStandardError);
    }
    
    // If process exists but is not running, restart it
    if (m_process->state() != QProcess::Running) {
        // Set environment
        m_process->setProcessEnvironment(m_environment);
        m_process->setWorkingDirectory(m_workingDirectory);
        
        // Start the shell with appropriate arguments
        QStringList arguments;
#ifdef Q_OS_WIN
        if (m_shellPath.contains("bash.exe")) {
            // Git Bash arguments for interactive mode
            arguments << "--login" << "-i";
        } else if (m_shellPath.contains("wsl.exe")) {
            // WSL arguments for interactive mode
            arguments << "--exec" << "bash" << "-l";
        } else if (m_shellPath.contains("powershell")) {
            // PowerShell arguments for interactive mode
            arguments << "-NoExit" << "-Command" << "& {Write-Host 'PowerShell Ready' -ForegroundColor Green}";
        } else {
            // CMD arguments for interactive mode
            arguments << "/K" << "echo Shell Ready";
        }
#else
        // Bash arguments for interactive mode
        arguments << "-i";
#endif
        
        writeToTerminal(QString("Starting %1...\n").arg(m_shellPath));
        writeToTerminal(QString("Arguments: %1\n").arg(arguments.join(" ")), QColor(150, 150, 150));
        
        m_process->start(m_shellPath, arguments);
        m_isActive = m_process->waitForStarted(5000); // Increased timeout for bash
        
        if (m_isActive) {
            m_processStatusLabel->setText("Running");
            writeToTerminal("Shell started successfully\n", QColor(100, 255, 100));
            
            // For bash, wait a moment for initialization
            if (m_shellPath.contains("bash.exe") || m_shellPath.contains("wsl.exe")) {
                QThread::msleep(500); // Give bash time to initialize
            }
        } else {
            m_processStatusLabel->setText("Failed");
            writeToTerminal(QString("Failed to start shell: %1\n").arg(m_process->errorString()), QColor(255, 100, 100));
            writeToTerminal(QString("Process state: %1\n").arg(m_process->state()), QColor(255, 100, 100));
        }
    }
}

void TerminalSession::closeTerminal()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        if (!m_process->waitForFinished(3000)) {
            m_process->kill();
        }
    }
    
    if (m_terminal) {
        writeToTerminal("\nTerminal session closed\n");
        m_isActive = false;
        m_processStatusLabel->setText("Closed");
    }
}

bool TerminalSession::isActive() const
{
    return m_isActive;
}


void TerminalSession::onContextMenuRequested(const QPoint& pos)
{
    if (m_contextMenu) {
        m_contextMenu->exec(mapToGlobal(pos));
    }
}

void TerminalSession::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    // Create interactive terminal area
    m_terminal = new QTextEdit(this);
    m_terminal->setObjectName("terminal");
    m_terminal->setReadOnly(true); // Make it read-only, handle input via event filter
    m_terminal->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Apply VS Code-style terminal appearance
    QPalette palette = m_terminal->palette();
    palette.setColor(QPalette::Base, QColor(30, 30, 30));
    palette.setColor(QPalette::Text, QColor(212, 212, 212));
    m_terminal->setPalette(palette);
    
    // Set font
    QFont font("Consolas", 10);
    font.setStyleHint(QFont::TypeWriter);
    m_terminal->setFont(font);
    
    // Setup text formats for ANSI support
    m_defaultFormat = m_terminal->currentCharFormat();
    m_defaultFormat.setForeground(QColor(212, 212, 212));
    m_currentFormat = m_defaultFormat;
    
    // Connect signals
    connect(m_terminal, &QTextEdit::customContextMenuRequested, this, &TerminalSession::onContextMenuRequested);
    
    // Install event filter to handle key presses
    m_terminal->installEventFilter(this);
    
    // Create status bar
    m_statusLayout = new QHBoxLayout();
    m_statusLayout->setContentsMargins(5, 2, 5, 2);
    
    m_statusLabel = new QLabel("Ready", this);
    m_workingDirLabel = new QLabel("", this);
    m_shellLabel = new QLabel("", this);
    m_processStatusLabel = new QLabel("Idle", this);
    
    // Style status labels
    QFont statusFont = font;
    statusFont.setPointSize(8);
    m_statusLabel->setFont(statusFont);
    m_workingDirLabel->setFont(statusFont);
    m_shellLabel->setFont(statusFont);
    m_processStatusLabel->setFont(statusFont);
    
    m_statusLabel->setStyleSheet("color: #d4d4d4;");
    m_workingDirLabel->setStyleSheet("color: #569cd6;");
    m_shellLabel->setStyleSheet("color: #4ec9b0;");
    m_processStatusLabel->setStyleSheet("color: #ce9178;");
    
    m_statusLayout->addWidget(m_statusLabel);
    m_statusLayout->addWidget(m_workingDirLabel);
    m_statusLayout->addStretch();
    m_statusLayout->addWidget(m_shellLabel);
    m_statusLayout->addWidget(m_processStatusLabel);
    
    // Add widgets to layout
    m_layout->addWidget(m_terminal);
    m_layout->addLayout(m_statusLayout);
    
    // Set default session name
    m_sessionName = "Terminal";
    updateTitle();
    
    // Initialize working directory
    m_workingDirectory = QDir::currentPath();
    updateStatusBar();
    
    // Add initial prompt
    writeToTerminal("Terminal session started\n");
    writePrompt();
}

void TerminalSession::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    m_renameAction = new QAction("Rename", this);
    m_closeAction = new QAction("Close", this);
    m_clearAction = new QAction("Clear", this);
    m_copyAction = new QAction("Copy", this);
    m_pasteAction = new QAction("Paste", this);
    m_selectAllAction = new QAction("Select All", this);
    m_clearHistoryAction = new QAction("Clear History", this);
    m_testBashAction = new QAction("Test Bash Terminal", this);
    
    connect(m_renameAction, &QAction::triggered, [this]() {
        bool ok;
        QString newName = QInputDialog::getText(this, "Rename Terminal", "Enter new name:", 
                                               QLineEdit::Normal, m_sessionName, &ok);
        if (ok && !newName.isEmpty()) {
            setSessionName(newName);
            emit sessionRenamed(newName);
        }
    });
    
    connect(m_closeAction, &QAction::triggered, [this]() {
        closeTerminal();
        emit sessionClosed();
    });
    
    connect(m_clearAction, &QAction::triggered, [this]() {
        if (m_terminal) {
            m_terminal->setReadOnly(false);
            m_terminal->clear();
            m_terminal->setReadOnly(true);
            writePrompt();
        }
    });
    
    connect(m_copyAction, &QAction::triggered, [this]() {
        if (m_terminal) {
            m_terminal->copy();
        }
    });
    
    connect(m_pasteAction, &QAction::triggered, [this]() {
        if (m_terminal && isAtPrompt()) {
            QString clipboardText = QApplication::clipboard()->text();
            handleUserInput(clipboardText);
        }
    });
    
    connect(m_selectAllAction, &QAction::triggered, [this]() {
        if (m_terminal) {
            m_terminal->selectAll();
        }
    });
    
    connect(m_clearHistoryAction, &QAction::triggered, [this]() {
        m_commandHistory.clear();
        m_historyIndex = -1;
    });
    
    connect(m_testBashAction, &QAction::triggered, [this]() {
        testBashTerminal();
    });
    
    m_contextMenu->addAction(m_copyAction);
    m_contextMenu->addAction(m_pasteAction);
    m_contextMenu->addAction(m_selectAllAction);
    m_contextMenu->addSeparator();
    
    // Add shell selection submenu
    QMenu* shellMenu = m_contextMenu->addMenu("Switch Shell");
    QStringList availableShells = getAvailableShells();
    for (const QString& shell : availableShells) {
        QAction* shellAction = shellMenu->addAction(shell);
        connect(shellAction, &QAction::triggered, [this, shell]() {
            switchShell(shell);
        });
    }
    
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_renameAction);
    m_contextMenu->addAction(m_clearAction);
    m_contextMenu->addAction(m_clearHistoryAction);
    m_contextMenu->addAction(m_testBashAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_closeAction);
}

void TerminalSession::updateTitle()
{
    // Title is managed by the parent TerminalTab
}

// Command execution methods
void TerminalSession::executeCommand(const QString& command)
{
    // If no process exists or process is not running, start the shell
    if (!m_process || m_process->state() != QProcess::Running) {
        writeToTerminal("Starting shell...\n");
        startTerminal();
        
        // Wait a moment for the shell to start
        if (m_process && m_process->waitForStarted(2000)) {
            writeToTerminal("Shell started. Executing command...\n");
        } else {
            writeToTerminal("Error: Failed to start shell process\n", QColor(255, 100, 100));
            return;
        }
    }
    
    addToHistory(command);
    writeToTerminal(command + "\n");
    m_process->write(command.toUtf8() + "\n");
    emit commandExecuted(command);
}

void TerminalSession::sendInput(const QString& input)
{
    if (m_process && m_process->state() == QProcess::Running) {
        m_process->write(input.toUtf8());
    }
}

void TerminalSession::setWorkingDirectory(const QString& dir)
{
    if (QDir(dir).exists()) {
        m_workingDirectory = QDir(dir).absolutePath();
        if (m_process) {
            m_process->setWorkingDirectory(m_workingDirectory);
        }
        updateStatusBar();
        emit workingDirectoryChanged(m_workingDirectory);
    }
}

QString TerminalSession::workingDirectory() const
{
    return m_workingDirectory;
}

// Environment methods
void TerminalSession::setEnvironmentVariable(const QString& name, const QString& value)
{
    m_environment.insert(name, value);
    if (m_process) {
        m_process->setProcessEnvironment(m_environment);
    }
}

void TerminalSession::setEnvironmentVariables(const QProcessEnvironment& env)
{
    m_environment = env;
    if (m_process) {
        m_process->setProcessEnvironment(m_environment);
    }
}

QProcessEnvironment TerminalSession::environment() const
{
    return m_environment;
}

// Session state methods
void TerminalSession::saveSessionState()
{
    // Save current state to settings or file
    QSettings settings;
    settings.beginGroup("TerminalSession_" + m_sessionName);
    settings.setValue("workingDirectory", m_workingDirectory);
    settings.setValue("commandHistory", m_commandHistory);
    settings.endGroup();
    m_hasUnsavedChanges = false;
}

void TerminalSession::restoreSessionState()
{
    QSettings settings;
    settings.beginGroup("TerminalSession_" + m_sessionName);
    QString savedDir = settings.value("workingDirectory", QDir::homePath()).toString();
    QStringList savedHistory = settings.value("commandHistory", QStringList()).toStringList();
    settings.endGroup();
    
    setWorkingDirectory(savedDir);
    m_commandHistory = savedHistory;
    m_historyIndex = m_commandHistory.size();
}

bool TerminalSession::hasUnsavedChanges() const
{
    return m_hasUnsavedChanges;
}

// Process event handlers
void TerminalSession::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_isActive = false;
    m_processStatusLabel->setText("Finished");
    
    if (exitStatus == QProcess::NormalExit) {
        writeToTerminal(QString("\nProcess finished with exit code: %1\n").arg(exitCode));
    } else {
        writeToTerminal("\nProcess crashed\n", QColor(255, 100, 100));
    }
    
    writePrompt();
}

void TerminalSession::onProcessError(QProcess::ProcessError error)
{
    m_isActive = false;
    m_processStatusLabel->setText("Error");
    
    QString errorMsg;
    switch (error) {
        case QProcess::FailedToStart:
            errorMsg = "Failed to start process";
            break;
        case QProcess::Crashed:
            errorMsg = "Process crashed";
            break;
        case QProcess::Timedout:
            errorMsg = "Process timed out";
            break;
        case QProcess::WriteError:
            errorMsg = "Write error";
            break;
        case QProcess::ReadError:
            errorMsg = "Read error";
            break;
        default:
            errorMsg = "Unknown error";
            break;
    }
    
    writeToTerminal(QString("\nError: %1\n").arg(errorMsg), QColor(255, 100, 100));
    emit errorReceived(errorMsg);
}

void TerminalSession::onProcessReadyReadStandardOutput()
{
    QByteArray data = m_process->readAllStandardOutput();
    processOutput(data, false);
}

void TerminalSession::onProcessReadyReadStandardError()
{
    QByteArray data = m_process->readAllStandardError();
    processOutput(data, true);
}

void TerminalSession::onTerminalKeyPress(QKeyEvent* event)
{
    if (!m_terminal) return;
    
    QTextCursor cursor = m_terminal->textCursor();
    int currentPos = cursor.position();
    
    // Check if we're at or after the prompt position
    if (currentPos < m_promptPosition) {
        // Don't allow editing before the prompt
        cursor.setPosition(m_promptPosition);
        m_terminal->setTextCursor(cursor);
        return;
    }
    
    switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter: {
            // Execute the current command
            cursor.movePosition(QTextCursor::End);
            m_terminal->setTextCursor(cursor);
            QString command = m_currentCommand.trimmed();
            if (!command.isEmpty()) {
                processCommand(command);
            } else {
                writePrompt();
            }
            break;
        }
            
        case Qt::Key_Up:
            // Navigate command history
            navigateHistory(-1);
            event->accept();
            return;
            
        case Qt::Key_Down:
            // Navigate command history
            navigateHistory(1);
            event->accept();
            return;
            
        case Qt::Key_Backspace:
            // Only allow backspace if we're not at the prompt
            if (currentPos > m_promptPosition) {
                // Update current command
                m_currentCommand = m_currentCommand.left(m_currentCommand.length() - 1);
                // Remove the character from the display
                m_terminal->setReadOnly(false);
                cursor.deletePreviousChar();
                m_terminal->setTextCursor(cursor);
                m_terminal->setReadOnly(true);
            } else {
                event->ignore();
                return;
            }
            break;
            
        case Qt::Key_Left:
            // Don't allow moving left of the prompt
            if (currentPos <= m_promptPosition) {
                event->ignore();
                return;
            }
            break;
            
        case Qt::Key_Home:
            // Move to prompt position
            cursor.setPosition(m_promptPosition);
            m_terminal->setTextCursor(cursor);
            event->accept();
            return;
            
        default:
            // Handle regular character input
            if (event->text().length() > 0 && event->text().at(0).isPrint()) {
                m_currentCommand += event->text();
                writeToTerminal(event->text());
            }
            break;
    }
    
    event->accept();
}

// Helper methods
void TerminalSession::processOutput(const QByteArray& data, bool isError)
{
    QString text = QString::fromUtf8(data);
    
    if (isError) {
        writeToTerminal(text, QColor(255, 100, 100));
        emit errorReceived(text);
    } else {
        if (m_ansiMode) {
            handleAnsiEscapeSequences(text);
        } else {
            writeToTerminal(text);
        }
        emit outputReceived(text);
    }
}

void TerminalSession::handleAnsiEscapeSequences(const QString& text)
{
    // Simple ANSI escape sequence handling
    // This is a basic implementation - could be enhanced for full ANSI support
    QString processedText = text;
    
    // Remove or convert basic ANSI escape sequences
    QRegularExpression ansiRegex("\x1b\\[[0-9;]*m");
    processedText.remove(ansiRegex);
    
    // Handle color codes (basic implementation)
    if (text.contains("\x1b[31m")) { // Red
        writeToTerminal(processedText, QColor(255, 100, 100));
    } else if (text.contains("\x1b[32m")) { // Green
        writeToTerminal(processedText, QColor(100, 255, 100));
    } else if (text.contains("\x1b[33m")) { // Yellow
        writeToTerminal(processedText, QColor(255, 255, 100));
    } else if (text.contains("\x1b[34m")) { // Blue
        writeToTerminal(processedText, QColor(100, 100, 255));
    } else {
        writeToTerminal(processedText);
    }
}

void TerminalSession::addToHistory(const QString& command)
{
    if (!command.isEmpty() && (m_commandHistory.isEmpty() || m_commandHistory.last() != command)) {
        m_commandHistory.append(command);
        if (m_commandHistory.size() > 100) { // Limit history size
            m_commandHistory.removeFirst();
        }
        m_historyIndex = m_commandHistory.size();
    }
}

void TerminalSession::navigateHistory(int direction)
{
    if (m_commandHistory.isEmpty()) return;
    
    if (direction > 0 && m_historyIndex < m_commandHistory.size() - 1) {
        m_historyIndex++;
    } else if (direction < 0 && m_historyIndex > 0) {
        m_historyIndex--;
    }
    
    if (m_historyIndex >= 0 && m_historyIndex < m_commandHistory.size()) {
        // Clear current command and replace with history item
        m_terminal->setReadOnly(false);
        QTextCursor cursor = m_terminal->textCursor();
        cursor.setPosition(m_promptPosition);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        
        // Insert the history command
        QString historyCommand = m_commandHistory[m_historyIndex];
        cursor.insertText(historyCommand);
        m_currentCommand = historyCommand;
        
        // Move cursor to end
        cursor.movePosition(QTextCursor::End);
        m_terminal->setTextCursor(cursor);
        m_terminal->setReadOnly(true);
    }
}

void TerminalSession::setupShell()
{
    // Detect shell based on platform
#ifdef Q_OS_WIN
    // Try different shells in order of preference
    QStringList shellCandidates;
    
    // 1. Git Bash (Linux-like commands on Windows)
    QStringList gitBashPaths = {
        "C:\\Program Files\\Git\\bin\\bash.exe",
        "C:\\Program Files (x86)\\Git\\bin\\bash.exe",
        "C:\\Users\\" + QDir::home().dirName() + "\\AppData\\Local\\Programs\\Git\\bin\\bash.exe"
    };
    
    for (const QString& gitBashPath : gitBashPaths) {
        if (QFile::exists(gitBashPath)) {
            shellCandidates.append(gitBashPath);
            break; // Use the first found Git Bash
        }
    }
    
    // 2. WSL (Windows Subsystem for Linux)
    QString wslPath = "wsl.exe";
    QProcess wslTest;
    wslTest.start(wslPath, QStringList() << "--list" << "--quiet");
    if (wslTest.waitForFinished(2000) && wslTest.exitCode() == 0) {
        shellCandidates.append(wslPath);
    }
    
    // 3. PowerShell
    QString powershellPath = "powershell.exe";
    QProcess psTest;
    psTest.start(powershellPath, QStringList() << "-Command" << "echo test");
    if (psTest.waitForFinished(1000) && psTest.exitCode() == 0) {
        shellCandidates.append(powershellPath);
    }
    
    // 4. CMD as fallback
    shellCandidates.append("cmd.exe");
    
    // Use the first available shell
    if (!shellCandidates.isEmpty()) {
        m_shellPath = shellCandidates.first();
        
        // Set appropriate label
        if (m_shellPath.contains("bash.exe")) {
            m_shellLabel->setText("Git Bash");
        } else if (m_shellPath.contains("wsl.exe")) {
            m_shellLabel->setText("WSL");
        } else if (m_shellPath.contains("powershell")) {
            m_shellLabel->setText("PowerShell");
        } else {
            m_shellLabel->setText("CMD");
        }
    } else {
        m_shellPath = "cmd.exe";
        m_shellLabel->setText("CMD");
    }
#else
    m_shellPath = "/bin/bash";
    m_shellLabel->setText("Bash");
#endif
    
    // Set up environment
    m_environment = QProcessEnvironment::systemEnvironment();
    
    // Add some useful environment variables
    m_environment.insert("TERM", "xterm-256color");
    m_environment.insert("COLORTERM", "truecolor");
    
    // For Git Bash, ensure proper PATH
    if (m_shellPath.contains("bash.exe")) {
        QString gitPath = QFileInfo(m_shellPath).absolutePath();
        QString currentPath = m_environment.value("PATH");
        if (!currentPath.contains(gitPath)) {
            m_environment.insert("PATH", gitPath + ";" + currentPath);
        }
        // Set MSYS environment
        m_environment.insert("MSYSTEM", "MINGW64");
        m_environment.insert("MSYS", "winsymlinks:nativestrict");
    }
    
    // Debug information
    writeToTerminal(QString("Shell configured: %1\n").arg(m_shellPath));
}

void TerminalSession::switchShell(const QString& shellType)
{
    // Close current terminal if running
    if (m_process && m_process->state() == QProcess::Running) {
        closeTerminal();
    }
    
    // Set new shell based on type
    if (shellType == "Git Bash") {
        // Find Git Bash path
        QStringList gitBashPaths = {
            "C:\\Program Files\\Git\\bin\\bash.exe",
            "C:\\Program Files (x86)\\Git\\bin\\bash.exe",
            "C:\\Users\\" + QDir::home().dirName() + "\\AppData\\Local\\Programs\\Git\\bin\\bash.exe"
        };
        
        bool found = false;
        for (const QString& gitBashPath : gitBashPaths) {
            if (QFile::exists(gitBashPath)) {
                m_shellPath = gitBashPath;
                found = true;
                break;
            }
        }
        
        if (!found) {
            writeToTerminal("Git Bash not found. Please install Git for Windows.\n", QColor(255, 100, 100));
            return;
        }
        m_shellLabel->setText("Git Bash");
    } else if (shellType == "WSL") {
        m_shellPath = "wsl.exe";
        m_shellLabel->setText("WSL");
    } else if (shellType == "PowerShell") {
        m_shellPath = "powershell.exe";
        m_shellLabel->setText("PowerShell");
    } else if (shellType == "CMD") {
        m_shellPath = "cmd.exe";
        m_shellLabel->setText("CMD");
    } else {
        writeToTerminal(QString("Unknown shell type: %1\n").arg(shellType), QColor(255, 100, 100));
        return;
    }
    
    writeToTerminal(QString("Switched to %1\n").arg(shellType));
    writePrompt();
}

QStringList TerminalSession::getAvailableShells() const
{
    QStringList shells;
    
#ifdef Q_OS_WIN
    // Check Git Bash
    QStringList gitBashPaths = {
        "C:\\Program Files\\Git\\bin\\bash.exe",
        "C:\\Program Files (x86)\\Git\\bin\\bash.exe",
        "C:\\Users\\" + QDir::home().dirName() + "\\AppData\\Local\\Programs\\Git\\bin\\bash.exe"
    };
    
    bool gitBashFound = false;
    for (const QString& gitBashPath : gitBashPaths) {
        if (QFile::exists(gitBashPath)) {
            gitBashFound = true;
            break;
        }
    }
    
    if (gitBashFound) {
        shells.append("Git Bash");
    }
    
    // Check WSL
    QProcess wslTest;
    wslTest.start("wsl.exe", QStringList() << "--list" << "--quiet");
    if (wslTest.waitForFinished(2000) && wslTest.exitCode() == 0) {
        shells.append("WSL");
    }
    
    // Check PowerShell
    QProcess psTest;
    psTest.start("powershell.exe", QStringList() << "-Command" << "echo test");
    if (psTest.waitForFinished(1000) && psTest.exitCode() == 0) {
        shells.append("PowerShell");
    }
    
    // CMD is always available
    shells.append("CMD");
#else
    shells.append("Bash");
#endif
    
    return shells;
}

void TerminalSession::showLinuxHelp()
{
    writeToTerminal("\n=== Linux Commands Available ===\n", QColor(100, 200, 255));
    
    QStringList commands = {
        "File Operations:",
        "  ls -l          List files with details",
        "  pwd            Show current directory",
        "  cd <dir>       Change directory",
        "  mkdir <dir>    Create directory",
        "  rm <file>      Remove file",
        "  cp <src> <dst> Copy file",
        "  mv <src> <dst> Move/rename file",
        "",
        "Text Operations:",
        "  cat <file>     Display file contents",
        "  grep <pattern> Search text in files",
        "  head <file>    Show first lines",
        "  tail <file>    Show last lines",
        "  nano <file>    Edit file",
        "",
        "System Information:",
        "  ps             Show running processes",
        "  top            Show system resources",
        "  df -h          Show disk usage",
        "  free -h        Show memory usage",
        "  uname -a       Show system info",
        "",
        "Git Commands:",
        "  git status     Show git status",
        "  git add <file> Stage file",
        "  git commit -m  Commit changes",
        "  git push       Push to remote",
        "  git pull       Pull from remote",
        "",
        "Network:",
        "  ping <host>    Test connectivity",
        "  curl <url>     Download/request",
        "  wget <url>     Download file",
        "",
        "Built-in Commands:",
        "  help           Show this help",
        "  linux-help     Show this help",
        "  test-bash      Test bash terminal functionality",
        "  bash-test      Test bash terminal functionality",
        ""
    };
    
    for (const QString& cmd : commands) {
        if (cmd.isEmpty()) {
            writeToTerminal("\n");
        } else if (cmd.endsWith(":")) {
            writeToTerminal(cmd + "\n", QColor(255, 200, 100));
        } else {
            writeToTerminal(cmd + "\n");
        }
    }
    
    writeToTerminal("Note: Available commands depend on your shell (Git Bash, WSL, etc.)\n", QColor(200, 200, 200));
}

void TerminalSession::testBashTerminal()
{
    writeToTerminal("\n=== Bash Terminal Test ===\n", QColor(100, 200, 255));
    
    if (m_shellPath.contains("bash.exe")) {
        writeToTerminal("Testing Git Bash terminal...\n");
        writeToTerminal("Current shell: " + m_shellPath + "\n");
        writeToTerminal("Environment variables:\n");
        writeToTerminal("  PATH: " + m_environment.value("PATH").left(100) + "...\n");
        writeToTerminal("  TERM: " + m_environment.value("TERM") + "\n");
        writeToTerminal("  MSYSTEM: " + m_environment.value("MSYSTEM") + "\n");
        
        // Test basic bash commands
        writeToTerminal("\nTesting basic commands:\n");
        executeCommand("echo 'Bash test successful'");
        executeCommand("pwd");
        executeCommand("ls -la");
    } else if (m_shellPath.contains("wsl.exe")) {
        writeToTerminal("Testing WSL terminal...\n");
        writeToTerminal("Current shell: " + m_shellPath + "\n");
        executeCommand("echo 'WSL test successful'");
        executeCommand("pwd");
        executeCommand("ls -la");
    } else {
        writeToTerminal("Not using bash terminal. Current shell: " + m_shellPath + "\n", QColor(255, 200, 100));
    }
}

void TerminalSession::writeToTerminal(const QString& text, const QColor& color)
{
    if (!m_terminal) return;
    
    // Temporarily make terminal writable to insert text
    bool wasReadOnly = m_terminal->isReadOnly();
    m_terminal->setReadOnly(false);
    
    QTextCursor cursor = m_terminal->textCursor();
    cursor.movePosition(QTextCursor::End);
    
    if (color.isValid()) {
        QTextCharFormat format = m_currentFormat;
        format.setForeground(color);
        cursor.setCharFormat(format);
    } else {
        cursor.setCharFormat(m_defaultFormat);
    }
    
    cursor.insertText(text);
    m_terminal->setTextCursor(cursor);
    
    // Restore read-only state
    m_terminal->setReadOnly(wasReadOnly);
    
    // Auto-scroll to bottom
    QScrollBar* scrollBar = m_terminal->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void TerminalSession::updateStatusBar()
{
    if (m_workingDirLabel) {
        QString shortPath = m_workingDirectory;
        if (shortPath.length() > 50) {
            shortPath = "..." + shortPath.right(47);
        }
        m_workingDirLabel->setText(shortPath);
    }
}

void TerminalSession::handleUserInput(const QString& input)
{
    m_currentCommand += input;
    writeToTerminal(input);
}

void TerminalSession::processCommand(const QString& command)
{
    if (command.isEmpty()) {
        writePrompt();
        return;
    }
    
    // Handle built-in commands
    if (command == "help" || command == "linux-help") {
        showLinuxHelp();
        writePrompt();
        return;
    } else if (command == "test-bash" || command == "bash-test") {
        testBashTerminal();
        writePrompt();
        return;
    }
    
    // Add to history
    addToHistory(command);
    
    // Execute the command
    executeCommand(command);
    
    // Clear current command
    m_currentCommand.clear();
}

bool TerminalSession::isAtPrompt() const
{
    if (!m_terminal) return false;
    QTextCursor cursor = m_terminal->textCursor();
    return cursor.position() >= m_promptPosition;
}

void TerminalSession::writePrompt()
{
    writeToTerminal("$ ");
    m_promptPosition = m_terminal->textCursor().position();
    m_currentCommand.clear();
}

bool TerminalSession::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_terminal && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        onTerminalKeyPress(keyEvent);
        return true; // Event handled
    }
    return QWidget::eventFilter(obj, event);
}

// TerminalTab Implementation
TerminalTab::TerminalTab(QWidget* parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_tabWidget(nullptr)
    , m_contextMenu(nullptr)
{
    setupUI();
    setupContextMenu();
    addPlusTab(); // Add the "+" tab first
    addNewSession(); // Start with one terminal session
}

TerminalTab::~TerminalTab()
{
}

void TerminalTab::addPlusTab()
{
    // Create a dummy widget for the "+" tab
    QWidget* plusWidget = new QWidget();
    plusWidget->setObjectName("plusTab");
    
    // Add the "+" tab
    int plusTabIndex = m_tabWidget->addTab(plusWidget, "+");
    
    // Style the "+" tab
    m_tabWidget->tabBar()->setTabButton(plusTabIndex, QTabBar::RightSide, nullptr);
    m_tabWidget->tabBar()->setTabButton(plusTabIndex, QTabBar::LeftSide, nullptr);
    
    // Set a special style for the "+" tab
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #C0C0C0; }"
        "QTabBar::tab { padding: 8px 12px; }"
        "QTabBar::tab:selected { background-color: #E1E1E1; }"
        "QTabBar::tab:hover { background-color: #F0F0F0; }"
    );
    
    // Connect tab bar click to handle "+" tab clicks
    connect(m_tabWidget->tabBar(), &QTabBar::tabBarClicked, this, &TerminalTab::onTabBarClicked);
}

void TerminalTab::addNewSession()
{
    TerminalSession* session = new TerminalSession(this);
    session->setSessionName(QString("Terminal %1").arg(m_tabWidget->count()));
    
    // Insert before the "+" tab (which is always the last tab)
    int index = m_tabWidget->insertTab(m_tabWidget->count() - 1, session, session->sessionName());
    m_tabWidget->setCurrentIndex(index);
    
    // Connect signals
    connect(session, &TerminalSession::sessionClosed, this, &TerminalTab::onSessionClosed);
    connect(session, &TerminalSession::sessionRenamed, this, &TerminalTab::onSessionRenamed);
    
    updateSessionButtons();
    emit sessionCountChanged(m_tabWidget->count() - 1); // Subtract 1 for the "+" tab
}

void TerminalTab::closeCurrentSession()
{
    int currentIndex = m_tabWidget->currentIndex();
    if (currentIndex >= 0 && m_tabWidget->count() > 1) {
        QWidget* widget = m_tabWidget->widget(currentIndex);
        m_tabWidget->removeTab(currentIndex);
        widget->deleteLater();
        updateSessionButtons();
        emit sessionCountChanged(m_tabWidget->count());
    }
}

void TerminalTab::renameCurrentSession()
{
    int currentIndex = m_tabWidget->currentIndex();
    if (currentIndex >= 0) {
        TerminalSession* session = qobject_cast<TerminalSession*>(m_tabWidget->widget(currentIndex));
        if (session) {
            bool ok;
            QString newName = QInputDialog::getText(this, "Rename Terminal", "Enter new name:", 
                                                   QLineEdit::Normal, session->sessionName(), &ok);
            if (ok && !newName.isEmpty()) {
                session->setSessionName(newName);
                m_tabWidget->setTabText(currentIndex, newName);
            }
        }
    }
}

void TerminalTab::clearCurrentSession()
{
    int currentIndex = m_tabWidget->currentIndex();
    if (currentIndex >= 0) {
        TerminalSession* session = qobject_cast<TerminalSession*>(m_tabWidget->widget(currentIndex));
        if (session && session->m_terminal) {
            session->m_terminal->clear();
        }
    }
}

int TerminalTab::sessionCount() const
{
    return m_tabWidget ? m_tabWidget->count() : 0;
}

TerminalSession* TerminalTab::currentSession() const
{
    int currentIndex = m_tabWidget->currentIndex();
    if (currentIndex >= 0) {
        return qobject_cast<TerminalSession*>(m_tabWidget->widget(currentIndex));
    }
    return nullptr;
}

void TerminalTab::onSessionClosed()
{
    TerminalSession* session = qobject_cast<TerminalSession*>(sender());
    if (session) {
        int index = m_tabWidget->indexOf(session);
        if (index >= 0) {
            m_tabWidget->removeTab(index);
            session->deleteLater();
            updateSessionButtons();
            emit sessionCountChanged(m_tabWidget->count());
        }
    }
}

void TerminalTab::onSessionRenamed(const QString& newName)
{
    TerminalSession* session = qobject_cast<TerminalSession*>(sender());
    if (session) {
        int index = m_tabWidget->indexOf(session);
        if (index >= 0) {
            m_tabWidget->setTabText(index, newName);
        }
    }
}

void TerminalTab::onTabContextMenuRequested(const QPoint& pos)
{
    if (m_contextMenu) {
        m_contextMenu->exec(mapToGlobal(pos));
    }
}

void TerminalTab::onTabChanged(int index)
{
    updateSessionButtons();
}

void TerminalTab::onTabBarClicked(int index)
{
    // If the "+" tab was clicked, add a new session
    if (index == m_tabWidget->count() - 1) {
        addNewSession();
    }
}

void TerminalTab::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    // No button layout needed - using "+" tab instead
    
    // Create tab widget for terminal sessions
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, [this](int index) {
        // Don't allow closing the "+" tab (it's always the last tab)
        if (index == m_tabWidget->count() - 1) {
            return; // This is the "+" tab, don't close it
        }
        
        if (m_tabWidget->count() > 2) { // More than just the "+" tab
            QWidget* widget = m_tabWidget->widget(index);
            m_tabWidget->removeTab(index);
            widget->deleteLater();
            updateSessionButtons();
            emit sessionCountChanged(m_tabWidget->count() - 1); // Subtract 1 for the "+" tab
        }
    });
    
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &TerminalTab::onTabChanged);
    connect(m_tabWidget, &QTabWidget::customContextMenuRequested, this, &TerminalTab::onTabContextMenuRequested);
    
    m_layout->addWidget(m_tabWidget);
}

void TerminalTab::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    m_newSessionAction = new QAction("New Terminal", this);
    m_closeSessionAction = new QAction("Close Terminal", this);
    m_renameSessionAction = new QAction("Rename Terminal", this);
    m_clearSessionAction = new QAction("Clear Terminal", this);
    m_splitTerminalAction = new QAction("Split Terminal", this);
    
    connect(m_newSessionAction, &QAction::triggered, this, &TerminalTab::addNewSession);
    connect(m_closeSessionAction, &QAction::triggered, this, &TerminalTab::closeCurrentSession);
    connect(m_renameSessionAction, &QAction::triggered, this, &TerminalTab::renameCurrentSession);
    connect(m_clearSessionAction, &QAction::triggered, this, &TerminalTab::clearCurrentSession);
    
    m_contextMenu->addAction(m_newSessionAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_renameSessionAction);
    m_contextMenu->addAction(m_clearSessionAction);
    m_contextMenu->addAction(m_splitTerminalAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_closeSessionAction);
}

void TerminalTab::updateTabTitles()
{
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        // Skip the "+" tab (always the last tab)
        if (i == m_tabWidget->count() - 1) {
            continue;
        }
        
        TerminalSession* session = qobject_cast<TerminalSession*>(m_tabWidget->widget(i));
        if (session) {
            m_tabWidget->setTabText(i, session->sessionName());
        }
    }
}

void TerminalTab::updateSessionButtons()
{
    // No buttons to update anymore - using "+" tab instead
    // This method is kept for compatibility but does nothing
}

// ProblemsTab Implementation
ProblemsTab::ProblemsTab(QWidget* parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_filterEdit(nullptr)
    , m_clearButton(nullptr)
    , m_problemsTree(nullptr)
    , m_contextMenu(nullptr)
    , m_problemCount(0)
{
    setupUI();
    setupContextMenu();
}

ProblemsTab::~ProblemsTab()
{
}

void ProblemsTab::addProblem(const QString& file, int line, int column, const QString& message, const QString& severity)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(m_problemsTree);
    item->setText(0, severity);
    item->setText(1, QString::number(line));
    item->setText(2, QString::number(column));
    item->setText(3, QFileInfo(file).fileName());
    item->setText(4, message);
    item->setData(0, Qt::UserRole, file);
    item->setData(0, Qt::UserRole + 1, line);
    item->setData(0, Qt::UserRole + 2, column);
    
    // Set severity color
    QColor color;
    if (severity == "Error") {
        color = QColor(255, 100, 100);
    } else if (severity == "Warning") {
        color = QColor(255, 200, 100);
    } else {
        color = QColor(100, 200, 255);
    }
    item->setForeground(0, color);
    
    m_problemCount++;
    updateProblemCount();
}

void ProblemsTab::clearProblems()
{
    m_problemsTree->clear();
    m_problemCount = 0;
    updateProblemCount();
}

void ProblemsTab::setFilter(const QString& filter)
{
    m_filterEdit->setText(filter);
    onFilterChanged();
}

int ProblemsTab::problemCount() const
{
    return m_problemCount;
}

void ProblemsTab::onProblemDoubleClicked(QTreeWidgetItem* item, int column)
{
    if (item) {
        QString file = item->data(0, Qt::UserRole).toString();
        int line = item->data(0, Qt::UserRole + 1).toInt();
        int column = item->data(0, Qt::UserRole + 2).toInt();
        emit problemDoubleClicked(file, line, column);
    }
}

void ProblemsTab::onContextMenuRequested(const QPoint& pos)
{
    if (m_contextMenu) {
        m_contextMenu->exec(mapToGlobal(pos));
    }
}

void ProblemsTab::onFilterChanged()
{
    QString filter = m_filterEdit->text();
    for (int i = 0; i < m_problemsTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = m_problemsTree->topLevelItem(i);
        bool visible = filter.isEmpty() || 
                      item->text(0).contains(filter, Qt::CaseInsensitive) ||
                      item->text(3).contains(filter, Qt::CaseInsensitive) ||
                      item->text(4).contains(filter, Qt::CaseInsensitive);
        item->setHidden(!visible);
    }
}

void ProblemsTab::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    // Filter layout
    m_filterLayout = new QHBoxLayout();
    m_filterLayout->setContentsMargins(5, 5, 5, 5);
    
    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText("Filter problems...");
    connect(m_filterEdit, &QLineEdit::textChanged, this, &ProblemsTab::onFilterChanged);
    
    m_clearButton = new QPushButton("Clear", this);
    connect(m_clearButton, &QPushButton::clicked, this, &ProblemsTab::clearProblems);
    
    m_filterLayout->addWidget(m_filterEdit);
    m_filterLayout->addWidget(m_clearButton);
    
    // Problems tree
    m_problemsTree = new QTreeWidget(this);
    m_problemsTree->setHeaderLabels({"Severity", "Line", "Column", "File", "Message"});
    m_problemsTree->setRootIsDecorated(false);
    m_problemsTree->setAlternatingRowColors(true);
    m_problemsTree->setSortingEnabled(true);
    m_problemsTree->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Set column widths
    m_problemsTree->setColumnWidth(0, 80);
    m_problemsTree->setColumnWidth(1, 60);
    m_problemsTree->setColumnWidth(2, 60);
    m_problemsTree->setColumnWidth(3, 150);
    
    connect(m_problemsTree, &QTreeWidget::itemDoubleClicked, this, &ProblemsTab::onProblemDoubleClicked);
    connect(m_problemsTree, &QTreeWidget::customContextMenuRequested, this, &ProblemsTab::onContextMenuRequested);
    
    m_layout->addLayout(m_filterLayout);
    m_layout->addWidget(m_problemsTree);
}

void ProblemsTab::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    m_clearAction = new QAction("Clear All", this);
    m_copyAction = new QAction("Copy", this);
    m_goToFileAction = new QAction("Go to File", this);
    
    connect(m_clearAction, &QAction::triggered, this, &ProblemsTab::clearProblems);
    connect(m_copyAction, &QAction::triggered, [this]() {
        QTreeWidgetItem* item = m_problemsTree->currentItem();
        if (item) {
            QString text = QString("%1:%2:%3 - %4").arg(item->text(3), item->text(1), item->text(2), item->text(4));
            QApplication::clipboard()->setText(text);
        }
    });
    connect(m_goToFileAction, &QAction::triggered, [this]() {
        QTreeWidgetItem* item = m_problemsTree->currentItem();
        if (item) {
            QString file = item->data(0, Qt::UserRole).toString();
            int line = item->data(0, Qt::UserRole + 1).toInt();
            int column = item->data(0, Qt::UserRole + 2).toInt();
            emit problemDoubleClicked(file, line, column);
        }
    });
    
    m_contextMenu->addAction(m_copyAction);
    m_contextMenu->addAction(m_goToFileAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_clearAction);
}

void ProblemsTab::updateProblemCount()
{
    // Update title or status if needed
    // This could be connected to a status bar or title update
}

// OutputTab Implementation
OutputTab::OutputTab(QWidget* parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_clearButton(nullptr)
    , m_outputText(nullptr)
    , m_contextMenu(nullptr)
{
    setupUI();
    setupContextMenu();
}

OutputTab::~OutputTab()
{
}

void OutputTab::appendOutput(const QString& text)
{
    m_outputText->append(text);
    // Auto-scroll to bottom
    QScrollBar* scrollBar = m_outputText->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void OutputTab::clearOutput()
{
    m_outputText->clear();
    emit outputCleared();
}

void OutputTab::setOutput(const QString& text)
{
    m_outputText->setPlainText(text);
}

void OutputTab::onContextMenuRequested(const QPoint& pos)
{
    if (m_contextMenu) {
        m_contextMenu->exec(mapToGlobal(pos));
    }
}

void OutputTab::onClearOutput()
{
    clearOutput();
}

void OutputTab::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    // Button layout
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setContentsMargins(5, 5, 5, 5);
    
    m_clearButton = new QPushButton("Clear Output", this);
    connect(m_clearButton, &QPushButton::clicked, this, &OutputTab::onClearOutput);
    
    m_buttonLayout->addWidget(m_clearButton);
    m_buttonLayout->addStretch();
    
    // Output text
    m_outputText = new QTextEdit(this);
    m_outputText->setReadOnly(true);
    m_outputText->setContextMenuPolicy(Qt::CustomContextMenu);
    m_outputText->setFont(QFont("Consolas", 9));
    
    connect(m_outputText, &QTextEdit::customContextMenuRequested, this, &OutputTab::onContextMenuRequested);
    
    m_layout->addLayout(m_buttonLayout);
    m_layout->addWidget(m_outputText);
}

void OutputTab::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    m_clearAction = new QAction("Clear", this);
    m_copyAction = new QAction("Copy", this);
    m_selectAllAction = new QAction("Select All", this);
    
    connect(m_clearAction, &QAction::triggered, this, &OutputTab::onClearOutput);
    connect(m_copyAction, &QAction::triggered, [this]() {
        m_outputText->copy();
    });
    connect(m_selectAllAction, &QAction::triggered, [this]() {
        m_outputText->selectAll();
    });
    
    m_contextMenu->addAction(m_copyAction);
    m_contextMenu->addAction(m_selectAllAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_clearAction);
}

// DebugConsoleTab Implementation
DebugConsoleTab::DebugConsoleTab(QWidget* parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_levelCombo(nullptr)
    , m_clearButton(nullptr)
    , m_debugText(nullptr)
    , m_contextMenu(nullptr)
    , m_currentLevel("ALL")
{
    setupUI();
    setupContextMenu();
}

DebugConsoleTab::~DebugConsoleTab()
{
}

void DebugConsoleTab::appendDebugMessage(const QString& message, const QString& level)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString formattedMessage = QString("[%1] [%2] %3").arg(timestamp, level, message);
    
    m_debugText->append(formattedMessage);
    emit debugMessageAdded(message, level);
    
    // Auto-scroll to bottom
    QScrollBar* scrollBar = m_debugText->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void DebugConsoleTab::clearDebugMessages()
{
    m_debugText->clear();
}

void DebugConsoleTab::setDebugLevel(const QString& level)
{
    m_currentLevel = level;
    m_levelCombo->setCurrentText(level);
    updateLevelFilter();
}

void DebugConsoleTab::onContextMenuRequested(const QPoint& pos)
{
    if (m_contextMenu) {
        m_contextMenu->exec(mapToGlobal(pos));
    }
}

void DebugConsoleTab::onClearDebugMessages()
{
    clearDebugMessages();
}

void DebugConsoleTab::onLevelFilterChanged()
{
    m_currentLevel = m_levelCombo->currentText();
    updateLevelFilter();
}

void DebugConsoleTab::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    // Filter layout
    m_filterLayout = new QHBoxLayout();
    m_filterLayout->setContentsMargins(5, 5, 5, 5);
    
    m_levelCombo = new QComboBox(this);
    m_levelCombo->addItems({"ALL", "DEBUG", "INFO", "WARNING", "ERROR", "FATAL"});
    m_levelCombo->setCurrentText("ALL");
    connect(m_levelCombo, QOverload<const QString&>::of(&QComboBox::currentTextChanged), 
            this, &DebugConsoleTab::onLevelFilterChanged);
    
    m_clearButton = new QPushButton("Clear", this);
    connect(m_clearButton, &QPushButton::clicked, this, &DebugConsoleTab::onClearDebugMessages);
    
    m_filterLayout->addWidget(m_levelCombo);
    m_filterLayout->addWidget(m_clearButton);
    m_filterLayout->addStretch();
    
    // Debug text
    m_debugText = new QTextEdit(this);
    m_debugText->setReadOnly(true);
    m_debugText->setContextMenuPolicy(Qt::CustomContextMenu);
    m_debugText->setFont(QFont("Consolas", 9));
    
    connect(m_debugText, &QTextEdit::customContextMenuRequested, this, &DebugConsoleTab::onContextMenuRequested);
    
    m_layout->addLayout(m_filterLayout);
    m_layout->addWidget(m_debugText);
}

void DebugConsoleTab::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    m_clearAction = new QAction("Clear", this);
    m_copyAction = new QAction("Copy", this);
    m_selectAllAction = new QAction("Select All", this);
    
    connect(m_clearAction, &QAction::triggered, this, &DebugConsoleTab::onClearDebugMessages);
    connect(m_copyAction, &QAction::triggered, [this]() {
        m_debugText->copy();
    });
    connect(m_selectAllAction, &QAction::triggered, [this]() {
        m_debugText->selectAll();
    });
    
    m_contextMenu->addAction(m_copyAction);
    m_contextMenu->addAction(m_selectAllAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_clearAction);
}

void DebugConsoleTab::updateLevelFilter()
{
    // This would filter the displayed messages based on the selected level
    // For now, we'll show all messages
    // In a full implementation, you'd store messages with their levels and filter accordingly
}

// TerminalSectionWidget Implementation
TerminalSectionWidget::TerminalSectionWidget(QWidget* parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_tabWidget(nullptr)
    , m_problemsTab(nullptr)
    , m_outputTab(nullptr)
    , m_terminalTab(nullptr)
    , m_debugConsoleTab(nullptr)
    , m_contextMenu(nullptr)
{
    setupUI();
    setupContextMenu();
    applyVSCodeStyle();
}

TerminalSectionWidget::~TerminalSectionWidget()
{
}

void TerminalSectionWidget::showProblemsTab()
{
    m_tabWidget->setCurrentIndex(0);
}

void TerminalSectionWidget::showOutputTab()
{
    m_tabWidget->setCurrentIndex(1);
}

void TerminalSectionWidget::showTerminalTab()
{
    m_tabWidget->setCurrentIndex(2);
}

void TerminalSectionWidget::showDebugConsoleTab()
{
    m_tabWidget->setCurrentIndex(3);
}

void TerminalSectionWidget::toggleVisibility()
{
    setVisible(!isVisible());
    emit visibilityToggled(isVisible());
}

void TerminalSectionWidget::onTabChanged(int index)
{
    // Handle tab change if needed
    Q_UNUSED(index)
}

void TerminalSectionWidget::onContextMenuRequested(const QPoint& pos)
{
    if (m_contextMenu) {
        m_contextMenu->exec(mapToGlobal(pos));
    }
}

void TerminalSectionWidget::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Create tabs
    m_problemsTab = new ProblemsTab(this);
    m_outputTab = new OutputTab(this);
    m_terminalTab = new TerminalTab(this);
    m_debugConsoleTab = new DebugConsoleTab(this);
    
    // Add tabs
    m_tabWidget->addTab(m_problemsTab, "Problems");
    m_tabWidget->addTab(m_outputTab, "Output");
    m_tabWidget->addTab(m_terminalTab, "Terminal");
    m_tabWidget->addTab(m_debugConsoleTab, "Debug Console");
    
    // Connect signals
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &TerminalSectionWidget::onTabChanged);
    connect(m_tabWidget, &QTabWidget::customContextMenuRequested, this, &TerminalSectionWidget::onContextMenuRequested);
    
    m_layout->addWidget(m_tabWidget);
}

void TerminalSectionWidget::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    m_toggleVisibilityAction = new QAction("Toggle Panel", this);
    m_moveToPanelAction = new QAction("Move to Panel", this);
    m_closePanelAction = new QAction("Close Panel", this);
    
    connect(m_toggleVisibilityAction, &QAction::triggered, this, &TerminalSectionWidget::toggleVisibility);
    connect(m_closePanelAction, &QAction::triggered, [this]() {
        setVisible(false);
        emit visibilityToggled(false);
    });
    
    m_contextMenu->addAction(m_toggleVisibilityAction);
    m_contextMenu->addAction(m_moveToPanelAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_closePanelAction);
}

void TerminalSectionWidget::applyVSCodeStyle()
{
    // Apply VS Code-style colors and fonts
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor(30, 30, 30));
    palette.setColor(QPalette::WindowText, QColor(212, 212, 212));
    palette.setColor(QPalette::Base, QColor(30, 30, 30));
    palette.setColor(QPalette::AlternateBase, QColor(45, 45, 45));
    palette.setColor(QPalette::Text, QColor(212, 212, 212));
    palette.setColor(QPalette::Button, QColor(45, 45, 45));
    palette.setColor(QPalette::ButtonText, QColor(212, 212, 212));
    palette.setColor(QPalette::Highlight, QColor(0, 122, 204));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    
    setPalette(palette);
    
    // Set font
    QFont font("Segoe UI", 9);
    setFont(font);
}
