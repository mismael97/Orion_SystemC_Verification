// TextGraphicsItem.cpp
#include "graphics/TextGraphicsItem.h"
#include "utils/PersistenceManager.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QTextCursor>
#include <QTextDocument>
#include <QPainter>
#include <QMenu>
#include <QInputDialog>
#include <QApplication>

TextGraphicsItem::TextGraphicsItem(const QString& text, QGraphicsItem* parent)
    : QGraphicsTextItem(parent)
    , m_textColor(Qt::black)
    , m_isEditing(false)
    , m_originalText(text)
    , m_originalPosition(QPointF(0, 0))
{
    // Set the text - empty string shows no text, allowing immediate typing
    setPlainText(text);
    
    // Set default font - use Tajawal for Arabic text
    QFont defaultFont("Tajawal", 14, QFont::Bold);
    setFont(defaultFont);
    
    setDefaultTextColor(m_textColor);
    
    // Make it movable and deletable
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    
    // Make sure it's visible
    setVisible(true);
    setOpacity(1.0);
    
    // Not editable by default
    setTextInteractionFlags(Qt::NoTextInteraction);
    
    qDebug() << "📝 TextGraphicsItem created with text:" << text << "| Visible:" << isVisible() << "| Opacity:" << opacity();
}

void TextGraphicsItem::setText(const QString& text)
{
    setPlainText(text);
}

void TextGraphicsItem::setTextColor(const QColor& color)
{
    m_textColor = color;
    setDefaultTextColor(color);
}

void TextGraphicsItem::setTextFont(const QFont& font)
{
    setFont(font);
}

void TextGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    // Show rename dialog instead of inline editing
    event->accept();
    showRenameDialog();
}

void TextGraphicsItem::focusOutEvent(QFocusEvent* event)
{
    // Disable editing when focus is lost
    m_isEditing = false;
    setTextInteractionFlags(Qt::NoTextInteraction);
    
    // Update original text after editing
    if (m_originalText != toPlainText()) {
        m_originalText = toPlainText();
    }
    
    // Emit signal that editing finished
    emit textEditingFinished();
    emit textChanged(toPlainText());
    
    QGraphicsTextItem::focusOutEvent(event);
}

void TextGraphicsItem::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        // Cancel editing on Escape
        clearFocus();
        return;
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        // Finish editing on Enter (unless Shift is pressed for multi-line)
        if (!(event->modifiers() & Qt::ShiftModifier)) {
            clearFocus();
            return;
        }
    } else if (event->key() == Qt::Key_Delete && !m_isEditing) {
        // Delete the text item when Delete key is pressed (and not editing)
        if (scene()) {
            scene()->removeItem(this);
            deleteLater();
            return;
        }
    }
    
    QGraphicsTextItem::keyPressEvent(event);
    
    // Emit signal that text changed
    emit textChanged(toPlainText());
}

QVariant TextGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged) {
        emit positionChanged(value.toPointF());
    }
    
    return QGraphicsTextItem::itemChange(change, value);
}

void TextGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    
    // Add rename action
    QAction* renameAction = menu.addAction("Rename");
    menu.addSeparator();
    
    // Add delete action
    QAction* deleteAction = menu.addAction("Delete");
    
    // Execute menu and handle selection
    QAction* selectedAction = menu.exec(event->screenPos());
    
    if (selectedAction == renameAction) {
        showRenameDialog();
    } else if (selectedAction == deleteAction) {
        // Delete the text item
        if (scene()) {
            scene()->removeItem(this);
            deleteLater();
        }
    }
    
    event->accept();
}

void TextGraphicsItem::showRenameDialog()
{
    bool ok;
    QString currentText = toPlainText();
    if (currentText.isEmpty()) {
        currentText = "";
    }
    
    // Create input dialog
    QInputDialog dialog;
    dialog.setWindowTitle("Rename Text");
    dialog.setLabelText("Enter new text:");
    dialog.setTextValue(currentText);
    dialog.setInputMode(QInputDialog::TextInput);
    
    // Set font to Tajawal for Arabic support
    QFont dialogFont("Tajawal", 10);
    dialog.setFont(dialogFont);
    
    // Position dialog in the center of the screen
    if (QApplication::activeWindow()) {
        dialog.move(QApplication::activeWindow()->geometry().center() - dialog.rect().center());
    }
    
    // Show dialog and get result
    ok = dialog.exec();
    QString newText = dialog.textValue();
    
    if (ok && !newText.isEmpty()) {
        // Update the text
        setPlainText(newText);
        
        // Update original tracking values to current state
        m_originalText = newText;
        m_originalPosition = pos();
        
        // Emit signals to trigger persistence update
        emit textChanged(newText);
        emit textEditingFinished();
    }
}

