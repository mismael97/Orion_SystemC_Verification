// TextGraphicsItem.h
#ifndef TEXTGRAPHICSITEM_H
#define TEXTGRAPHICSITEM_H

#include <QGraphicsTextItem>
#include <QFont>

class TextGraphicsItem : public QGraphicsTextItem
{
    Q_OBJECT

public:
    explicit TextGraphicsItem(const QString& text = "", QGraphicsItem* parent = nullptr);
    
    // Getters
    QString getText() const { return toPlainText(); }
    QColor getTextColor() const { return m_textColor; }
    QFont getTextFont() const { return font(); }
    QString getOriginalText() const { return m_originalText; }
    QPointF getOriginalPosition() const { return m_originalPosition; }
    
    // Setters
    void setText(const QString& text);
    void setTextColor(const QColor& color);
    void setTextFont(const QFont& font);
    
    // Rename functionality
    void showRenameDialog();
    
protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

signals:
    void textChanged(const QString& newText);
    void textEditingFinished();
    void positionChanged(const QPointF& newPosition);

private:
    QColor m_textColor;
    bool m_isEditing;
    QString m_originalText;
    QPointF m_originalPosition;
};

#endif // TEXTGRAPHICSITEM_H

