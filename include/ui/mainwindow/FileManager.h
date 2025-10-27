// FileManager.h
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QString>

class QListWidget;
class QTreeWidget;
class MainWindow;
class QFileSystemWatcher;
class FileExplorerTreeWidget;

class FileManager : public QObject
{
    Q_OBJECT

public:
    explicit FileManager(MainWindow* mainWindow, QListWidget* componentList);
    explicit FileManager(MainWindow* mainWindow, FileExplorerTreeWidget* treeWidget);
    
    void setFileWatcher(QFileSystemWatcher* watcher);
    void loadRtlFilesFromDirectory(const QString& directoryPath);
    void refreshModuleView(const QString& filePath);
    void initializeReadyComponents(QListWidget* readyComponentList);
    
    // New methods for tree widget
    void setTreeWidget(FileExplorerTreeWidget* treeWidget);
    void setComponentList(QListWidget* componentList);

private:
    void addModuleToList(const QString& filePath, const QString& moduleName);
    void addFileToList(const QString& filePath);
    void addDirectoryToList(const QString& dirPath, const QString& dirName);

    MainWindow* m_mainWindow;
    QListWidget* m_componentList;
    FileExplorerTreeWidget* m_treeWidget;
    QFileSystemWatcher* m_fileWatcher;
};

#endif // FILEMANAGER_H

