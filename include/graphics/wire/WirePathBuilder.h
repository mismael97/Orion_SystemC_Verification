// WirePathBuilder.h
#ifndef WIREPATHBUILDER_H
#define WIREPATHBUILDER_H

#include <QPainterPath>
#include <QPointF>
#include <QVector>

/**
 * @brief Handles path creation for wire routing
 * 
 * Supports different routing modes:
 * - Straight: Direct line between points
 * - Orthogonal: Manhattan routing with right angles
 * - Bezier: Smooth curved paths
 */
class WirePathBuilder
{
public:
    enum RoutingMode {
        Straight,
        Orthogonal,
        Bezier
    };

    /**
     * @brief Creates a path based on routing mode
     * @param start Starting point
     * @param end Ending point
     * @param mode Routing mode to use
     * @param offset Orthogonal offset (for parallel wires)
     * @return Generated painter path
     */
    static QPainterPath createPath(const QPointF& start, const QPointF& end, 
                                   RoutingMode mode, qreal offset = 0.0);

    /**
     * @brief Creates a path through control points
     * @param start Starting point
     * @param end Ending point
     * @param controlPoints Intermediate control points
     * @return Generated painter path
     */
    static QPainterPath createPathWithControlPoints(const QPointF& start, 
                                                     const QPointF& end,
                                                     const QVector<QPointF>& controlPoints);

    /**
     * @brief Creates a straight line path
     */
    static QPainterPath createStraightPath(const QPointF& start, const QPointF& end);

    /**
     * @brief Creates an orthogonal (Manhattan) path
     */
    static QPainterPath createOrthogonalPath(const QPointF& start, const QPointF& end, 
                                             qreal offset = 0.0);

    /**
     * @brief Creates a Bezier curve path
     */
    static QPainterPath createBezierPath(const QPointF& start, const QPointF& end);

private:
    static constexpr qreal PORT_SPACING = 20.0;
};

#endif // WIREPATHBUILDER_H
