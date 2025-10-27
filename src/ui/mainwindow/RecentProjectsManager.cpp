// RecentProjectsManager.cpp
#include "ui/mainwindow/RecentProjectsManager.h"
#include "ui/MainWindow.h"
#include "utils/PersistenceManager.h"
#include "graphics/TextGraphicsItem.h"
#include "scene/SchematicScene.h"
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QSettings>
#include <QDir>
#include <QMessageBox>
#include <QStatusBar>
#include <QDebug>

RecentProjectsManager::RecentProjectsManager(MainWindow* mainWindow)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
    , m_recentProjectsMenu(nullptr)
{
}

void RecentProjectsManager::setupRecentProjectsMenu()
{
    // Find the File menu
    QMenu* fileMenu = nullptr;
    foreach (QAction* action, m_mainWindow->menuBar()->actions()) {
        if (action->text().contains("File")) {
            fileMenu = action->menu();
            break;
        }
    }
    
    if (!fileMenu) {
        qWarning() << "File menu not found, cannot create recent projects menu";
        return;
    }
    
    // Create the "Open Recent" submenu
    m_recentProjectsMenu = new QMenu(tr("Open Recent"), m_mainWindow);
    
    // Insert the submenu after "Open File" action
    QAction* openFileAction = m_mainWindow->findChild<QAction*>("actionOpen_File");
    if (openFileAction) {
        // Find the position of Open File action in the menu
        QList<QAction*> actions = fileMenu->actions();
        int insertPos = actions.indexOf(openFileAction) + 1;
        
        if (insertPos > 0 && insertPos < actions.size()) {
            fileMenu->insertMenu(actions.at(insertPos), m_recentProjectsMenu);
        } else {
            // If not found, just add it to the menu
            fileMenu->addMenu(m_recentProjectsMenu);
        }
    } else {
        // If Open File action not found, just add to the menu
        fileMenu->addMenu(m_recentProjectsMenu);
    }
    
    // Populate the menu with recent projects
    refreshRecentProjectsMenu();
}

void RecentProjectsManager::updateRecentProjects(const QString& projectPath)
{
    if (projectPath.isEmpty()) return;
    
    // Normalize the path
    QString normalizedPath = QDir(projectPath).absolutePath();
    
    // Load current recent projects
    QSettings settings("SCV_Project", "RecentProjects");
    QStringList recentProjects = settings.value("projects").toStringList();
    
    // Remove if already exists (we'll add it to the front)
    recentProjects.removeAll(normalizedPath);
    
    // Add to front
    recentProjects.prepend(normalizedPath);
    
    // Keep only last 10
    while (recentProjects.size() > 10) {
        recentProjects.removeLast();
    }
    
    // Save back to settings
    settings.setValue("projects", recentProjects);
    
    // Refresh the menu display
    refreshRecentProjectsMenu();
}

void RecentProjectsManager::refreshRecentProjectsMenu()
{
    if (!m_recentProjectsMenu) return;
    
    // Clear the menu
    m_recentProjectsMenu->clear();
    
    // Load recent projects from settings
    QSettings settings("SCV_Project", "RecentProjects");
    QStringList recentProjects = settings.value("projects").toStringList();
    
    // Filter out non-existent directories and add to menu
    int count = 0;
    for (const QString& projectPath : recentProjects) {
        if (count >= 10) break;
        
        if (QDir(projectPath).exists()) {
            QAction* projectAction = m_recentProjectsMenu->addAction(projectPath);
            connect(projectAction, &QAction::triggered, this, [this, projectPath]() {
                openRecentProject(projectPath);
            });
            count++;
        }
    }
    
    // If no valid projects, show placeholder
    if (count == 0) {
        QAction* noProjectsAction = m_recentProjectsMenu->addAction(tr("No Recent Projects"));
        noProjectsAction->setEnabled(false);
    }
    
    // Add separator and clear action
    m_recentProjectsMenu->addSeparator();
    QAction* clearAction = new QAction(tr("Clear Recent Projects"), this);
    connect(clearAction, &QAction::triggered, this, &RecentProjectsManager::clearRecentProjects);
    m_recentProjectsMenu->addAction(clearAction);
}

void RecentProjectsManager::clearRecentProjects()
{
    // Clear settings
    QSettings settings("SCV_Project", "RecentProjects");
    settings.remove("projects");
    
    // Refresh menu to show empty state
    refreshRecentProjectsMenu();
    
    m_mainWindow->statusBar()->showMessage(tr("Recent projects cleared"), 2000);
}

void RecentProjectsManager::openRecentProject(const QString& projectPath)
{
    qDebug() << "📂 RecentProjectsManager::openRecentProject() called for project:" << projectPath;
    if (!QDir(projectPath).exists()) {
        QMessageBox::warning(
            m_mainWindow,
            tr("Project Not Found"),
            tr("The project directory no longer exists:\n%1").arg(projectPath)
        );
        
        // Remove from recent projects
        QSettings settings("SCV_Project", "RecentProjects");
        QStringList recentProjects = settings.value("projects").toStringList();
        recentProjects.removeAll(projectPath);
        settings.setValue("projects", recentProjects);
        
        // Refresh menu to remove the deleted project
        refreshRecentProjectsMenu();
        return;
    }
    
    // Call MainWindow's public method to load the project
    m_mainWindow->loadProject(projectPath);
}

