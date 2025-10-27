// TextItemManager.h
#ifndef TEXTITEMMANAGER_H
#define TEXTITEMMANAGER_H

#include <QObject>

class MainWindow;
class TextGraphicsItem;
class SchematicScene;

class TextItemManager : public QObject
{
    Q_OBJECT

public:
    explicit TextItemManager(MainWindow* mainWindow, SchematicScene* scene);
    
    void connectTextItemSignals(TextGraphicsItem* textItem);
    void onAddText();
    void onAddTextAtPosition(const QPointF& position);

private:
    MainWindow* m_mainWindow;
    SchematicScene* m_scene;
};

#endif // TEXTITEMMANAGER_H

