// WireControlPoints.h
#ifndef WIRECONTROLPOINTS_H
#define WIRECONTROLPOINTS_H

#include <QVector>
#include <QPointF>
#include <QPainter>
#include <QPainterPath>

/**
 * @brief Manages control points for wire manipulation
 * 
 * Handles adding, removing, finding, and drawing control points
 * that allow users to reshape wires.
 */
class WireControlPoints
{
public:
    WireControlPoints();

    /**
     * @brief Adds a control point
     */
    void addControlPoint(const QPointF& point);

    /**
     * @brief Removes a control point by index
     */
    void removeControlPoint(int index);

    /**
     * @brief Finds control point at given position
     * @return Index of control point, or -1 if none found
     */
    int findControlPointAt(const QPointF& scenePos) const;

    /**
     * @brief Finds nearest point on wire path
     */
    QPointF findNearestPointOnPath(const QPointF& pos, const QPainterPath& path) const;

    /**
     * @brief Updates control point position
     */
    void updateControlPoint(int index, const QPointF& newPos);

    /**
     * @brief Gets all control points
     */
    const QVector<QPointF>& getControlPoints() const { return m_controlPoints; }

    /**
     * @brief Sets all control points
     */
    void setControlPoints(const QVector<QPointF>& points) { m_controlPoints = points; }

    /**
     * @brief Clears all control points
     */
    void clear() { m_controlPoints.clear(); }

    /**
     * @brief Checks if there are any control points
     */
    bool isEmpty() const { return m_controlPoints.isEmpty(); }

    /**
     * @brief Gets count of control points
     */
    int count() const { return m_controlPoints.size(); }

    /**
     * @brief Draws control points on painter
     * @param painter The painter to draw with
     * @param isSelected Whether the wire is selected
     * @param hoveredIndex Index of hovered control point (-1 if none)
     */
    void drawControlPoints(QPainter* painter, bool isSelected, int hoveredIndex) const;

    /**
     * @brief Nudges all control points by offset
     */
    void nudgeAll(const QPointF& offset);

    static constexpr qreal CONTROL_POINT_RADIUS = 6.0;
    static constexpr qreal CONTROL_POINT_DETECTION_RADIUS = 15.0;

private:
    QVector<QPointF> m_controlPoints;
};

#endif // WIRECONTROLPOINTS_H
