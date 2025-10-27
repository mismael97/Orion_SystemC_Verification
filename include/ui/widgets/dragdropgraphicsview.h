// DragDropGraphicsView.h
#ifndef DRAGDROPGRAPHICSVIEW_H
#define DRAGDROPGRAPHICSVIEW_H

#include <QGraphicsView>

class DragDropGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit DragDropGraphicsView(QWidget *parent = nullptr);
    void setSharedScene(QGraphicsScene* scene);
    double currentScale() const { return m_currentScale; }
    
    // View navigation methods
    void panUp(int amount = 100);
    void panDown(int amount = 100);
    void panLeft(int amount = 100);
    void panRight(int amount = 100);

signals:
    void componentDropped(const QString& componentId, const QString& componentName, const QPointF& position);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QGraphicsScene* m_scene = nullptr;
    bool m_isPanning = false;
    QPoint m_lastPanPoint;
    double m_currentScale = 1.0;   // start at 100%
    const double MIN_SCALE = 0.2;  // 20%
    const double MAX_SCALE = 4.0;  // 400%

private slots:
    void updateScene();
};

#endif // DRAGDROPGRAPHICSVIEW_H

