// TextItemManager.cpp
#include "ui/mainwindow/TextItemManager.h"
#include "ui/MainWindow.h"
#include "graphics/TextGraphicsItem.h"
#include "scene/SchematicScene.h"
#include "utils/PersistenceManager.h"
#include "ui/widgets/dragdropgraphicsview.h"
#include <QTimer>
#include <QStatusBar>
#include <QDebug>

TextItemManager::TextItemManager(MainWindow* mainWindow, SchematicScene* scene)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
    , m_scene(scene)
{
}

void TextItemManager::connectTextItemSignals(TextGraphicsItem* textItem)
{
    // Track the original state for updates
    struct TextItemState {
        QString lastText;
        QPointF lastPosition;
    };
    
    auto state = new TextItemState{textItem->getText(), textItem->pos()};
    
    // Connect text editing finished signal
    connect(textItem, &TextGraphicsItem::textEditingFinished, this, [this, textItem, state]() {
        // Update the text item in persistence with both current text and position
        PersistenceManager::instance().updateTextItem(
            state->lastText,        // Old text for finding the item
            state->lastPosition,    // Old position for finding the item
            textItem->getText(),    // New text content
            textItem->pos(),        // Current position (might have changed)
            textItem->getTextColor(),
            textItem->getTextFont()
        );
        
        // Update tracking state
        state->lastText = textItem->getText();
        state->lastPosition = textItem->pos();
        
        qDebug() << "Text updated:" << textItem->getText() << "at position" << textItem->pos();
    });
    
    // Connect position changed signal
    connect(textItem, &TextGraphicsItem::positionChanged, this, [this, textItem, state](const QPointF& newPos) {
        // Update the text item position in persistence
        PersistenceManager::instance().updateTextItem(
            state->lastText,        // Current text for finding the item
            state->lastPosition,    // Old position for finding the item
            textItem->getText(),    // Keep same text
            newPos,                 // New position
            textItem->getTextColor(),
            textItem->getTextFont()
        );
        
        // Update tracking state
        state->lastPosition = newPos;
        
        qDebug() << "Position updated for" << textItem->getText() << "to" << newPos;
    });
    
    // Connect destroyed signal
    connect(textItem, &QObject::destroyed, this, [this, state, textItem]() {
        // Remove the text item from persistence when deleted
        PersistenceManager::instance().removeTextItem(
            state->lastText,
            state->lastPosition
        );
        
        qDebug() << "Text item deleted:" << state->lastText;
        
        // Clean up state tracking
        delete state;
    });
}

void TextItemManager::onAddText()
{
    qDebug() << "🆕 Adding new text item...";
    
    // Get the graphics view from MainWindow
    DragDropGraphicsView* view = m_mainWindow->findChild<DragDropGraphicsView*>("graphicsView");
    if (!view) {
        qWarning() << "Graphics view not found!";
        return;
    }
    
    // Get the center of the current view
    QPointF centerPos = view->mapToScene(view->viewport()->rect().center());
    
    qDebug() << "📍 Center position:" << centerPos;
    
    // Create a new text item with default text
    TextGraphicsItem* textItem = new TextGraphicsItem("New Text");
    textItem->setPos(centerPos);
    m_scene->addItem(textItem);
    
    qDebug() << "➕ Text item added to scene | Scene item count:" << m_scene->items().count()
             << "| In scene:" << (textItem->scene() != nullptr)
             << "| Visible:" << textItem->isVisible();
    
    // Connect signals first (so updates are tracked)
    connectTextItemSignals(textItem);
    
    // Save the text item immediately to schematic.json with initial position
    PersistenceManager::instance().saveTextItem(
        textItem->getText(),
        centerPos,
        textItem->getTextColor(),
        textItem->getTextFont()
    );
    
    qDebug() << "💾 Saved new text item to schematic.json:" << textItem->getText() 
             << "at position" << centerPos;
    
    // Show rename dialog immediately for user to customize text
    QTimer::singleShot(100, textItem, &TextGraphicsItem::showRenameDialog);
    
    m_mainWindow->statusBar()->showMessage(tr("Text saved to schematic - customize in dialog"), 2000);
}

void TextItemManager::onAddTextAtPosition(const QPointF& position)
{
    qDebug() << "🆕 Adding new text item at position:" << position;
    
    // Create a new text item with default text at the specified position
    TextGraphicsItem* textItem = new TextGraphicsItem("New Text");
    textItem->setPos(position);
    m_scene->addItem(textItem);
    
    qDebug() << "➕ Text item added to scene at position" << position 
             << "| Scene item count:" << m_scene->items().count()
             << "| In scene:" << (textItem->scene() != nullptr)
             << "| Visible:" << textItem->isVisible();
    
    // Connect signals first (so updates are tracked)
    connectTextItemSignals(textItem);
    
    // Save the text item immediately to schematic.json with the specified position
    PersistenceManager::instance().saveTextItem(
        textItem->getText(),
        position,
        textItem->getTextColor(),
        textItem->getTextFont()
    );
    
    qDebug() << "💾 Saved new text item to schematic.json:" << textItem->getText() 
             << "at position" << position;
    
    // Show rename dialog immediately for user to customize text
    QTimer::singleShot(100, textItem, &TextGraphicsItem::showRenameDialog);
    
    m_mainWindow->statusBar()->showMessage(tr("Text added at cursor position - customize in dialog"), 2000);
}

