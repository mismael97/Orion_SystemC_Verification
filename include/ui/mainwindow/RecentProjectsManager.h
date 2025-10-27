// RecentProjectsManager.h
#ifndef RECENTPROJECTSMANAGER_H
#define RECENTPROJECTSMANAGER_H

#include <QObject>
#include <QString>

class QMenu;
class MainWindow;

class RecentProjectsManager : public QObject
{
    Q_OBJECT

public:
    explicit RecentProjectsManager(MainWindow* mainWindow);
    
    void setupRecentProjectsMenu();
    void updateRecentProjects(const QString& projectPath);
    void refreshRecentProjectsMenu();

private slots:
    void clearRecentProjects();
    void openRecentProject(const QString& projectPath);

private:
    MainWindow* m_mainWindow;
    QMenu* m_recentProjectsMenu;
};

#endif // RECENTPROJECTSMANAGER_H

