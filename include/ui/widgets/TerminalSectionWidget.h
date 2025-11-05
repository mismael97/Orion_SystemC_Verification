/**
 * @file TerminalSectionWidget.h
 * @brief VS Code-style terminal section with multiple tabs and terminal sessions
 * 
 * This widget provides a comprehensive terminal section similar to VS Code,
 * featuring multiple tabs for different types of output and terminal sessions.
 * 
 * Features:
 * - Problems tab for compilation errors and warnings
 * - Output tab for build output and general messages
 * - Terminal tab with multiple terminal sessions
 * - Debug Console tab for debugging output
 * - VS Code-style terminal interface
 * - Multiple terminal session support
 * 
 * @author SCV Project Team
 * @version 1.0
 * @date 2024
 */

#ifndef TERMINALSECTIONWIDGET_H
#define TERMINALSECTIONWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QSplitter>
#include <QListWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QProcess>
#include <QTimer>
#include <QScrollBar>
#include <QFont>
#include <QColor>
#include <QPalette>
#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QProcessEnvironment>
#include <QKeyEvent>

QT_BEGIN_NAMESPACE
class QTextEdit;
QT_END_NAMESPACE

/**
 * @class TerminalSession
 * @brief Individual terminal session widget with real command execution
 */
class TerminalSession : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalSession(QWidget* parent = nullptr);
    ~TerminalSession();

    void setSessionName(const QString& name);
    QString sessionName() const;
    void startTerminal();
    void closeTerminal();
    bool isActive() const;
    
    // Shell management
    void switchShell(const QString& shellType);
    QStringList getAvailableShells() const;
    
    // Command execution
    void executeCommand(const QString& command);
    void sendInput(const QString& input);
    void setWorkingDirectory(const QString& dir);
    QString workingDirectory() const;
    void clearTerminal();
    
    // Environment
    void setEnvironmentVariable(const QString& name, const QString& value);
    void setEnvironmentVariables(const QProcessEnvironment& env);
    QProcessEnvironment environment() const;
    
    // Session state
    void saveSessionState();
    void restoreSessionState();
    bool hasUnsavedChanges() const;

signals:
    void sessionClosed();
    void sessionRenamed(const QString& newName);
    void workingDirectoryChanged(const QString& dir);
    void commandExecuted(const QString& command);
    void outputReceived(const QString& output);
    void errorReceived(const QString& error);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onProcessReadyReadStandardOutput();
    void onProcessReadyReadStandardError();
    void onContextMenuRequested(const QPoint& pos);
    void onTerminalKeyPress(QKeyEvent* event);

private:
    void setupUI();
    void setupContextMenu();
    void updateTitle();
    void updateStatusBar();
    void processOutput(const QByteArray& data, bool isError = false);
    void handleAnsiEscapeSequences(const QString& text);
    void addToHistory(const QString& command);
    void navigateHistory(int direction);
    void setupShell();
    void writeToTerminal(const QString& text, const QColor& color = QColor());
    void handleUserInput(const QString& input);
    void processCommand(const QString& command);
    bool isAtPrompt() const;
    void writePrompt();
    void showLinuxHelp();
    void testBashTerminal();
    
protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

    // UI Components
    QVBoxLayout* m_layout;
    QTextEdit* m_terminal;
    QHBoxLayout* m_statusLayout;
    QLabel* m_statusLabel;
    QLabel* m_workingDirLabel;
    QLabel* m_shellLabel;
    QLabel* m_processStatusLabel;
    
    // Session data
    QString m_sessionName;
    QString m_workingDirectory;
    QString m_shellPath;
    QProcess* m_process;
    QProcessEnvironment m_environment;
    bool m_isActive;
    bool m_hasUnsavedChanges;
    
    // Command history and input
    QStringList m_commandHistory;
    int m_historyIndex;
    QString m_currentInput;
    QString m_currentCommand;
    int m_promptPosition;
    
    // Context menu
    QMenu* m_contextMenu;
    QAction* m_renameAction;
    QAction* m_closeAction;
    QAction* m_clearAction;
    QAction* m_copyAction;
    QAction* m_pasteAction;
    QAction* m_selectAllAction;
    QAction* m_clearHistoryAction;
    QAction* m_testBashAction;
    
    // ANSI support
    QTextCharFormat m_defaultFormat;
    QTextCharFormat m_currentFormat;
    bool m_ansiMode;

    friend class TerminalTab;
};

/**
 * @class TerminalTab
 * @brief Terminal tab with multiple sessions support
 */
class TerminalTab : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalTab(QWidget* parent = nullptr);
    ~TerminalTab();

    void addNewSession();
    void closeCurrentSession();
    void renameCurrentSession();
    void clearCurrentSession();
    int sessionCount() const;
    TerminalSession* currentSession() const;

signals:
    void sessionCountChanged(int count);

private slots:
    void onSessionClosed();
    void onSessionRenamed(const QString& newName);
    void onTabContextMenuRequested(const QPoint& pos);
    void onTabChanged(int index);
    void onTabBarClicked(int index);

private:
    void setupUI();
    void setupContextMenu();
    void updateTabTitles();
    void updateSessionButtons();
    void addPlusTab();

    QVBoxLayout* m_layout;
    QTabWidget* m_tabWidget;
    QMenu* m_contextMenu;
    QAction* m_newSessionAction;
    QAction* m_closeSessionAction;
    QAction* m_renameSessionAction;
    QAction* m_clearSessionAction;
    QAction* m_splitTerminalAction;
};

/**
 * @class ProblemsTab
 * @brief Problems tab for displaying compilation errors and warnings
 */
class ProblemsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ProblemsTab(QWidget* parent = nullptr);
    ~ProblemsTab();

    void addProblem(const QString& file, int line, int column, const QString& message, const QString& severity);
    void clearProblems();
    void setFilter(const QString& filter);
    int problemCount() const;

signals:
    void problemDoubleClicked(const QString& file, int line, int column);

private slots:
    void onProblemDoubleClicked(QTreeWidgetItem* item, int column);
    void onContextMenuRequested(const QPoint& pos);
    void onFilterChanged();

private:
    void setupUI();
    void setupContextMenu();
    void updateProblemCount();

    QVBoxLayout* m_layout;
    QHBoxLayout* m_filterLayout;
    QLineEdit* m_filterEdit;
    QPushButton* m_clearButton;
    QTreeWidget* m_problemsTree;
    QMenu* m_contextMenu;
    QAction* m_clearAction;
    QAction* m_copyAction;
    QAction* m_goToFileAction;
    int m_problemCount;
};

/**
 * @class OutputTab
 * @brief Output tab for build output and general messages
 */
class OutputTab : public QWidget
{
    Q_OBJECT

public:
    explicit OutputTab(QWidget* parent = nullptr);
    ~OutputTab();

    void appendOutput(const QString& text);
    void clearOutput();
    void setOutput(const QString& text);

signals:
    void outputCleared();

private slots:
    void onContextMenuRequested(const QPoint& pos);
    void onClearOutput();

private:
    void setupUI();
    void setupContextMenu();

    QVBoxLayout* m_layout;
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_clearButton;
    QTextEdit* m_outputText;
    QMenu* m_contextMenu;
    QAction* m_clearAction;
    QAction* m_copyAction;
    QAction* m_selectAllAction;
};

/**
 * @class DebugConsoleTab
 * @brief Debug Console tab for debugging output
 */
class DebugConsoleTab : public QWidget
{
    Q_OBJECT

public:
    explicit DebugConsoleTab(QWidget* parent = nullptr);
    ~DebugConsoleTab();

    void appendDebugMessage(const QString& message, const QString& level = "INFO");
    void clearDebugMessages();
    void setDebugLevel(const QString& level);

signals:
    void debugMessageAdded(const QString& message, const QString& level);

private slots:
    void onContextMenuRequested(const QPoint& pos);
    void onClearDebugMessages();
    void onLevelFilterChanged();

private:
    void setupUI();
    void setupContextMenu();
    void updateLevelFilter();

    QVBoxLayout* m_layout;
    QHBoxLayout* m_filterLayout;
    QComboBox* m_levelCombo;
    QPushButton* m_clearButton;
    QTextEdit* m_debugText;
    QMenu* m_contextMenu;
    QAction* m_clearAction;
    QAction* m_copyAction;
    QAction* m_selectAllAction;
    QString m_currentLevel;
};

/**
 * @class TerminalSectionWidget
 * @brief Main terminal section widget with VS Code-style interface
 */
class TerminalSectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalSectionWidget(QWidget* parent = nullptr);
    ~TerminalSectionWidget();

    // Tab access methods
    ProblemsTab* problemsTab() const { return m_problemsTab; }
    OutputTab* outputTab() const { return m_outputTab; }
    TerminalTab* terminalTab() const { return m_terminalTab; }
    DebugConsoleTab* debugConsoleTab() const { return m_debugConsoleTab; }

    // Convenience methods
    void showProblemsTab();
    void showOutputTab();
    void showTerminalTab();
    void showDebugConsoleTab();
    void toggleVisibility();

signals:
    void visibilityToggled(bool visible);

private slots:
    void onTabChanged(int index);
    void onContextMenuRequested(const QPoint& pos);

private:
    void setupUI();
    void setupContextMenu();
    void applyVSCodeStyle();

    QVBoxLayout* m_layout;
    QTabWidget* m_tabWidget;
    ProblemsTab* m_problemsTab;
    OutputTab* m_outputTab;
    TerminalTab* m_terminalTab;
    DebugConsoleTab* m_debugConsoleTab;
    QMenu* m_contextMenu;
    QAction* m_toggleVisibilityAction;
    QAction* m_moveToPanelAction;
    QAction* m_closePanelAction;
};

#endif // TERMINALSECTIONWIDGET_H
