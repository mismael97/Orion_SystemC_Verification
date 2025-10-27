// WidgetManager.h
#ifndef WIDGETMANAGER_H
#define WIDGETMANAGER_H

#include <QObject>

class MainWindow;
class MinimapWidget;
class VerticalToolbar;
class EditComponentWidget;
class QGraphicsView;

class WidgetManager : public QObject
{
    Q_OBJECT

public:
    explicit WidgetManager(MainWindow* mainWindow, QGraphicsView* graphicsView);
    
    void setupMinimap();
    void setupSchematicOverlays();
    void setupEditComponentWidget();
    void updateMinimapPosition();
    void updateSchematicOverlaysPosition();
    void setCurrentRtlDirectory(const QString& directory);

    MinimapWidget* minimap() const { return m_minimap; }
    VerticalToolbar* verticalToolbar() const { return m_verticalToolbar; }
    EditComponentWidget* editComponentWidget() const { return m_editComponentWidget; }

private:
    MainWindow* m_mainWindow;
    QGraphicsView* m_graphicsView;
    MinimapWidget* m_minimap;
    VerticalToolbar* m_verticalToolbar;
    EditComponentWidget* m_editComponentWidget;
    QString m_currentRtlDirectory;
};

#endif // WIDGETMANAGER_H

