// TabManager.h
#ifndef TABMANAGER_H
#define TABMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>

class QTabWidget;
class MainWindow;

class TabManager : public QObject
{
    Q_OBJECT

public:
    explicit TabManager(MainWindow* mainWindow, QTabWidget* tabWidget);
    
    void setupTabWidget();
    void openFileInTab(const QString& filePath);

private slots:
    void onTabCloseRequested(int index);
    void onTabChanged(int index);
    void onComponentFileSaved(const QString& filePath);

private:
    MainWindow* m_mainWindow;
    QTabWidget* m_tabWidget;
    QMap<QString, int> m_openFileTabs;  // Maps file path to tab index
};

#endif // TABMANAGER_H

