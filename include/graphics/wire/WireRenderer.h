// WireRenderer.h
#ifndef WIRERENDERER_H
#define WIRERENDERER_H

#include <QPainter>
#include <QPainterPath>
#include <QColor>

/**
 * @brief Handles visual rendering of wires
 * 
 * Responsible for painting wires with various effects:
 * - Neon glow effects
 * - Different line styles (solid, dashed, dotted)
 * - Wire states (normal, active, error, locked)
 * - Arrows at endpoints
 * - Custom colors
 */
class WireRenderer
{
public:
    enum LineStyle {
        Solid,
        Dashed,
        Dotted
    };

    enum WireState {
        Normal,
        Active,
        Error,
        Locked
    };

    WireRenderer();

    /**
     * @brief Paints the wire with all effects
     * @param painter The painter to use
     * @param path The wire path to draw
     * @param isSelected Whether wire is selected
     * @param isTemporary Whether wire is being drawn
     */
    void paint(QPainter* painter, const QPainterPath& path, bool isSelected, bool isTemporary);

    /**
     * @brief Draws an arrow at the end of the wire
     */
    void drawArrow(QPainter* painter, const QPainterPath& path, bool isInverted);

    /**
     * @brief Draws locked indicator
     */
    void drawLockedIndicator(QPainter* painter, const QPainterPath& path);

    // Setters
    void setLineStyle(LineStyle style) { m_lineStyle = style; }
    void setWireState(WireState state) { m_wireState = state; }
    void setWireThickness(int thickness) { m_wireThickness = thickness; }
    void setNeonColor(const QColor& color) { m_neonColor = color; }
    void setCustomColor(const QColor& color) { m_customColor = color; m_useCustomColor = true; }
    void clearCustomColor() { m_useCustomColor = false; }
    void setLocked(bool locked) { m_isLocked = locked; }

    // Getters
    LineStyle getLineStyle() const { return m_lineStyle; }
    WireState getWireState() const { return m_wireState; }
    int getWireThickness() const { return m_wireThickness; }
    QColor getWireColor() const;
    bool isLocked() const { return m_isLocked; }
    bool hasCustomColor() const { return m_useCustomColor; }

    /**
     * @brief Generates a neon color for the wire
     */
    static QColor generateNeonColor();

private:
    LineStyle m_lineStyle = Solid;
    WireState m_wireState = Normal;
    int m_wireThickness = 3;
    QColor m_neonColor;
    QColor m_customColor;
    bool m_useCustomColor = false;
    bool m_isLocked = false;

    Qt::PenStyle getPenStyle() const;
    void paintNeonEffect(QPainter* painter, const QPainterPath& path, const QColor& color, int width);
};

#endif // WIRERENDERER_H
