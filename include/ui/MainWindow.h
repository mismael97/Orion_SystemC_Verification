/**
 * @file MainWindow.h
 * @brief Main application window with modular architecture and comprehensive UI management
 * 
 * This module provides the main application window that serves as the central hub for the
 * SystemVerilog schematic editor. It implements a modular architecture with specialized
 * managers for different aspects of the application functionality.
 * 
 * Architecture:
 * - TabManager: Manages code editor tabs and file operations
 * - FileManager: Handles RTL file discovery and module loading
 * - RecentProjectsManager: Manages recent project history
 * - WidgetManager: Manages specialized UI widgets (minimap, etc.)
 * - TextItemManager: Handles text annotation management
 * 
 * Key Features:
 * - Drag-and-drop component placement
 * - Real-time file system monitoring
 * - Undo/redo functionality
 * - Dark mode support
 * - Project management
 * - Component library integration
 * - Multi-tab code editing
 * 
 * @author SCV Project Team
 * @version 1.0
 * @date 2024
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsScene>
#include <QMainWindow>
#include <QUndoStack>
#include <QFileSystemWatcher>
#include <QListWidgetItem>
#include "scene/SchematicScene.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// Forward declarations for managers
class TabManager;
class FileManager;
class RecentProjectsManager;
class WidgetManager;
class TextItemManager;
#include "ui/widgets/ComponentLibraryWidget.h"
#include "ui/widgets/FileExplorerTreeWidget.h"
#include "ui/widgets/TerminalSectionWidget.h"

/**
 * @class MainWindow
 * @brief Main application window with modular architecture and comprehensive UI management
 * 
 * This class serves as the central hub for the SystemVerilog schematic editor,
 * coordinating between different managers and providing the main user interface.
 * It implements a modular architecture for better maintainability and extensibility.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for MainWindow
     * @param parent Parent widget (optional)
     * 
     * Initializes the main window with all managers and sets up the UI.
     * Configures drag-and-drop functionality, file monitoring, and undo/redo.
     */
    MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     * 
     * Cleans up resources and saves application state.
     */
    ~MainWindow();
    
    QUndoStack* undoStack() { return m_undoStack; }
    SchematicScene* schematicScene() { return scene; }
    QString currentDirectory() const { return currentRtlDirectory; }
    WidgetManager* widgetManager() { return m_widgetManager; }
    
    // Public methods for managers to use
    void openFileInTab(const QString& filePath);
    void loadProject(const QString& projectPath);
    void refreshComponent(const QString& filePath);
    void refreshModuleView(const QString& filePath);

private:
    Ui::MainWindow *ui;
    SchematicScene *scene;
    QString currentRtlDirectory;  // Store the current RTL directory path
    bool m_isLoadingProject;      // Flag to prevent multiple simultaneous loading operations
    
    // Undo/Redo
    QUndoStack *m_undoStack;
    
    // File Watcher for RTL changes
    QFileSystemWatcher *m_fileWatcher;
    
    // Managers
    TabManager *m_tabManager;
    FileManager *m_fileManager;
    RecentProjectsManager *m_recentProjectsManager;
    WidgetManager *m_widgetManager;
    TextItemManager *m_textItemManager;
    
    // UI Components
    ComponentLibraryWidget *m_componentLibrary;
    FileExplorerTreeWidget *m_fileExplorerTree;
    TerminalSectionWidget *m_terminalSection;

    // Setup methods
    void setupUndoRedo();
    void setupFileWatcher();
    void setupManagers();
    void setupResizableEditorPanel();
    void setupActionButtonsLayout();
    void setupControlButtonsWidget();
    void setupTerminalSection();
    void setupTerminalMenuActions();
    void loadProjectInternal(const QString& projectPath);
    
    // Control button actions
    void executeMakeVerilate();
    
protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void on_actionDark_Mode_toggled(bool checked);
    void on_actionOpen_File_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionRefresh_triggered();
    void on_actionToggleTerminal_triggered();
    void onRtlFileChanged(const QString& path);
    void onRtlListDoubleClicked(QListWidgetItem* item);
    
    // File explorer tree widget slots
    void onFileExplorerFileDoubleClicked(const QString& filePath);
    void onFileExplorerDirectoryDoubleClicked(const QString& dirPath);
    void onFileExplorerFileSelected(const QString& filePath);
    void onFileExplorerContextMenuRequested(const QString& filePath, const QPoint& globalPos);
    
    // Navigation handlers
    
    // Add text handler
    void onAddText();

};
#endif // MAINWINDOW_H

