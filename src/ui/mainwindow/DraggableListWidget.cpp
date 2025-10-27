// DraggableListWidget.cpp
#include "ui/mainwindow/DraggableListWidget.h"
#include "ui/widgets/dragdropgraphicsview.h"
#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <QFont>

DraggableListWidget::DraggableListWidget(QWidget* parent)
    : QListWidget(parent)
{
}

void DraggableListWidget::setGraphicsView(DragDropGraphicsView* view)
{
    m_view = view;
}

void DraggableListWidget::setReadyComponentList(bool isReady)
{
    m_isReadyComponentList = isReady;
}

void DraggableListWidget::startDrag(Qt::DropActions supportedActions)
{
    QList<QListWidgetItem*> items = selectedItems();
    if (items.isEmpty() || !m_view) return;

    QListWidgetItem* item = items.first();
    QMimeData* mimeData = QListWidget::mimeData(items);
    
    // Add a custom mime type to distinguish ready components
    if (m_isReadyComponentList) {
        mimeData->setData("application/x-ready-component", item->text().toUtf8());
    }

    // Get current scale
    double scale = m_view->currentScale();

    // Base size - Transactor is taller
    int baseWidth = 100;
    int baseHeight = 60;
    
    QString componentName = item->text();
    if (componentName == "Transactor") {
        baseWidth = 100;
        baseHeight = 200;  // Much taller for Transactor
    } else {
        baseWidth = 120;
        baseHeight = 80;
    }

    // Scale pixmap to match current zoom
    int dragWidth = qMax(20, static_cast<int>(baseWidth * scale));
    int dragHeight = qMax(20, static_cast<int>(baseHeight * scale));

    QPixmap pixmap(dragWidth, dragHeight);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, false);
    
    // Unified color for all ready components
    if (m_isReadyComponentList) {
        QColor backgroundColor = QColor("#637AB9").lighter(140);
        QColor borderColor = QColor("#637AB9");
        
        painter.setPen(QPen(borderColor, 2));
        painter.setBrush(backgroundColor);
    } else {
        // RTL modules - original white background
        painter.setPen(Qt::black);
        painter.setBrush(Qt::white);
    }
    
    painter.drawRect(0, 0, dragWidth - 1, dragHeight - 1);
    
    // Black text for both (lighter background now)
    painter.setPen(Qt::black);
    painter.setFont(QFont("Tajawal", qMax(6, static_cast<int>(10 * scale)), QFont::Bold));
    painter.drawText(5, static_cast<int>(15 * scale), item->text());
    painter.end();

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(dragWidth / 2, dragHeight / 2));

    drag->exec(supportedActions);
}

