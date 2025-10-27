// DraggableListWidget.h
#ifndef DRAGGABLELISTWIDGET_H
#define DRAGGABLELISTWIDGET_H

#include <QListWidget>

class DragDropGraphicsView;

class DraggableListWidget : public QListWidget {
    Q_OBJECT

public:
    explicit DraggableListWidget(QWidget* parent = nullptr);
    
    void setGraphicsView(DragDropGraphicsView* view);
    void setReadyComponentList(bool isReady);

protected:
    void startDrag(Qt::DropActions supportedActions) override;

private:
    DragDropGraphicsView* m_view = nullptr;
    bool m_isReadyComponentList = false;
};

#endif // DRAGGABLELISTWIDGET_H

