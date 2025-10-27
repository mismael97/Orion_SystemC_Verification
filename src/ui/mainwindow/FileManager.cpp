// FileManager.cpp
#include "ui/mainwindow/FileManager.h"
#include "ui/MainWindow.h"
#include "ui/widgets/FileExplorerTreeWidget.h"
#include "parsers/SvParser.h"
#include "utils/PersistenceManager.h"
#include "graphics/ModuleGraphicsItem.h"
#include "scene/SchematicScene.h"
#include <QListWidget>
#include <QTreeWidget>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QMessageBox>
#include <QFileSystemWatcher>
#include <QStatusBar>
#include <QDebug>

FileManager::FileManager(MainWindow* mainWindow, QListWidget* componentList)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
    , m_componentList(componentList)
    , m_treeWidget(nullptr)
    , m_fileWatcher(nullptr)
{
}

FileManager::FileManager(MainWindow* mainWindow, FileExplorerTreeWidget* treeWidget)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
    , m_componentList(nullptr)
    , m_treeWidget(treeWidget)
    , m_fileWatcher(nullptr)
{
}

void FileManager::setFileWatcher(QFileSystemWatcher* watcher)
{
    m_fileWatcher = watcher;
}

void FileManager::setTreeWidget(FileExplorerTreeWidget* treeWidget)
{
    m_treeWidget = treeWidget;
}

void FileManager::setComponentList(QListWidget* componentList)
{
    m_componentList = componentList;
}

void FileManager::loadRtlFilesFromDirectory(const QString& directoryPath)
{
    qDebug() << "🔍 FileManager::loadRtlFilesFromDirectory() called with path:" << directoryPath;
    
    // Use tree widget if available, otherwise fall back to list widget
    if (m_treeWidget) {
        m_treeWidget->setRootDirectory(directoryPath);
        return;
    }
    
    // Clear the component list (legacy support)
    if (m_componentList) {
        m_componentList->clear();
    }
    
    // Clear existing file watchers
    if (m_fileWatcher && !m_fileWatcher->files().isEmpty()) {
        m_fileWatcher->removePaths(m_fileWatcher->files());
    }

    QDir dir(directoryPath);
    
    // Files to exclude
    QStringList excludedFiles = {
        "connections.json",
        "rtl_placements.json"
    };
    // List all files in the directory and print their names
    QFileInfoList entryList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    qDebug() << "Files in directory:" << directoryPath;
    for (const QFileInfo& entry : entryList) {
        qDebug() << "  -" << entry.fileName();
    }
    // Get all entries (files and directories)
    QFileInfoList allEntries = dir.entryInfoList(
        QDir::AllEntries | QDir::NoDotAndDotDot,
        QDir::DirsFirst | QDir::Name
    );

    if (allEntries.isEmpty()) {
        QMessageBox::information(m_mainWindow, tr("Empty Directory"), 
                                tr("No files or directories found in the selected directory."));
        return;
    }

    int itemCount = 0;
    bool topModuleFound = false;
    QString topFilePath;
    QStringList filesToWatch;
    
    for (const QFileInfo& fileInfo : allEntries) {
        QString fileName = fileInfo.fileName();
        QString filePath = fileInfo.absoluteFilePath();
        
        // Skip excluded files
        if (excludedFiles.contains(fileName)) {
            continue;
        }
        
        if (fileInfo.isDir()) {
            // Add directory to list
            addDirectoryToList(filePath, fileName);
            itemCount++;
        } else if (fileInfo.isFile()) {
            // Check if it's a Verilog/SystemVerilog file with modules
            if (fileName.endsWith(".sv") || fileName.endsWith(".v")) {
                // Add to file watcher
                filesToWatch << filePath;
                
                // Try to parse modules
                QFile file(filePath);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    QString content = in.readAll();
                    file.close();

                    // Find all module declarations in the file
                    QRegularExpression moduleRegex(R"(^\s*module\s+(\w+))", QRegularExpression::MultilineOption);
                    QRegularExpressionMatchIterator i = moduleRegex.globalMatch(content);
                    
                    bool hasModules = false;
                    while (i.hasNext()) {
                        QRegularExpressionMatch match = i.next();
                        QString moduleName = match.captured(1);
                        if (!moduleName.isEmpty()) {
                            addModuleToList(filePath, moduleName);
                            itemCount++;
                            hasModules = true;
                            
                            // Check if this is the top module
                            if (fileName.toLower() == "top.sv" || fileName.toLower() == "top.v" || 
                                moduleName.toLower() == "top") {
                                topModuleFound = true;
                                topFilePath = filePath;
                            }
                        }
                    }
                    
                    // If no modules found, add as regular file
                    if (!hasModules) {
                        addFileToList(filePath);
                        itemCount++;
                    }
                } else {
                    // Can't open file, just add it
                    addFileToList(filePath);
                    itemCount++;
                }
            } else {
                // Non-Verilog file, add as regular file
                addFileToList(filePath);
                itemCount++;
            }
        }
    }
    
    // Add all files to watcher
    if (!filesToWatch.isEmpty() && m_fileWatcher) {
        m_fileWatcher->addPaths(filesToWatch);
        qDebug() << "Watching" << filesToWatch.size() << "RTL files for changes";
    }

    if (itemCount == 0) {
        m_mainWindow->statusBar()->showMessage(tr("No items to display (all files excluded)"), 3000);
    } else {
        m_mainWindow->statusBar()->showMessage(tr("Loaded %1 item(s) from %2").arg(itemCount).arg(directoryPath), 3000);
    }
}

void FileManager::addModuleToList(const QString& filePath, const QString& moduleName)
{
    QListWidgetItem* item = new QListWidgetItem(moduleName);
    
    // Store the file path in the item's data for later use
    item->setData(Qt::UserRole, filePath);
    
    // Create icon for the module
    QPixmap pixmap(100, 60);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    painter.drawRect(0, 0, 99, 59);
    painter.setPen(Qt::black);
    painter.setFont(QFont("Tajawal", 10));
    painter.drawText(5, 15, moduleName);
    painter.end();
    
    item->setIcon(QIcon(pixmap));
    item->setToolTip(QString("%1\n%2\nDouble-click to edit").arg(moduleName).arg(filePath));
    
    m_componentList->addItem(item);
}

void FileManager::addFileToList(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    
    QListWidgetItem* item = new QListWidgetItem(fileName);
    
    // Store the file path in the item's data for later use
    item->setData(Qt::UserRole, filePath);
    
    // Create icon for the file (different color to distinguish from modules)
    QPixmap pixmap(100, 60);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Qt::darkGray);
    painter.setBrush(QColor(240, 240, 240));
    painter.drawRect(0, 0, 99, 59);
    painter.setPen(Qt::darkGray);
    painter.setFont(QFont("Tajawal", 8));
    painter.drawText(5, 15, fileName);
    painter.end();
    
    item->setIcon(QIcon(pixmap));
    item->setToolTip(QString("File: %1\nDouble-click to edit").arg(filePath));
    
    m_componentList->addItem(item);
}

void FileManager::addDirectoryToList(const QString& dirPath, const QString& dirName)
{
    QListWidgetItem* item = new QListWidgetItem(dirName);
    
    // Store the directory path in the item's data for later use
    item->setData(Qt::UserRole, dirPath);
    item->setData(Qt::UserRole + 1, "directory"); // Mark as directory
    
    // Create icon for directory (folder icon with different color)
    QPixmap pixmap(100, 60);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, false);
    
    // Draw folder shape
    painter.setPen(QPen(QColor(255, 193, 7), 2)); // Orange/amber color
    painter.setBrush(QColor(255, 235, 59, 100)); // Light yellow with transparency
    
    // Folder tab
    painter.drawRect(5, 15, 30, 10);
    // Folder body
    painter.drawRect(5, 25, 90, 30);
    
    painter.setPen(Qt::darkGray);
    painter.setFont(QFont("Tajawal", 8, QFont::Bold));
    painter.drawText(5, 45, "📁 " + dirName);
    painter.end();
    
    item->setIcon(QIcon(pixmap));
    item->setToolTip(QString("Directory: %1\nDouble-click to navigate").arg(dirPath));
    
    m_componentList->addItem(item);
}

void FileManager::refreshModuleView(const QString& filePath)
{
    // Get the scene from MainWindow - we'll need to add a getter method
    // For now, this is a placeholder that will be properly connected
    // Re-parse the file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    // Find modules in the file
    QRegularExpression moduleRegex(R"(^\s*module\s+(\w+))", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator i = moduleRegex.globalMatch(content);
    
    PersistenceManager& pm = PersistenceManager::instance();
    
    // We'll need to get the scene from MainWindow
    // This will be properly connected in the MainWindow
    qDebug() << "Module refresh requested for:" << filePath;
}

void FileManager::initializeReadyComponents(QListWidget* readyComponentList)
{
    qDebug() << "initializeReadyComponents() called";
    
    // Check if readyComponentList exists
    if (!readyComponentList) {
        qDebug() << "ERROR: readyComponentList is null!";
        return;
    }
    
    qDebug() << "readyComponentList found, clearing...";
    
    // Clear the ready component list first
    readyComponentList->clear();
    
    // List of ready components
    QStringList readyComponents = {"Transactor", "RM", "Compare", "Driver", "Stimuler", "Stimuli", "RTL"};
    
    qDebug() << "Adding" << readyComponents.size() << "ready components";
    
    for (const QString& componentName : readyComponents) {
        QListWidgetItem* item = new QListWidgetItem(componentName);
        
        // Create icon for the ready component with correct size
        int iconWidth = 100;
        int iconHeight = 60;
        
        if (componentName == "Transactor") {
            iconWidth = 50;   // Scale down for icon
            iconHeight = 100;  // But keep the proportion taller
        } else {
            iconWidth = 100;
            iconHeight = 60;
        }
        
        QPixmap pixmap(iconWidth, iconHeight);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, false);
        
        // Unified color for all ready components
        QColor backgroundColor = QColor("#637AB9").lighter(140);
        QColor borderColor = QColor("#637AB9");
        
        painter.setPen(QPen(borderColor, 2));
        painter.setBrush(backgroundColor);
        painter.drawRect(0, 0, iconWidth - 1, iconHeight - 1);
        painter.setPen(Qt::black); // Black text for lighter background
        painter.setFont(QFont("Tajawal", 8, QFont::Bold));
        
        // Draw text vertically centered
        QRectF textRect(2, 2, iconWidth - 4, iconHeight - 4);
        painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, componentName);
        painter.end();
        
        item->setIcon(QIcon(pixmap));
        item->setToolTip("Ready-to-use component: " + componentName);
        
        readyComponentList->addItem(item);
        qDebug() << "Added component:" << componentName;
    }
    
    qDebug() << "Total items in readyComponentList:" << readyComponentList->count();
}

