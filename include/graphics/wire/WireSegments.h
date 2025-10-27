// WireSegments.h
#ifndef WIRESEGMENTS_H
#define WIRESEGMENTS_H

#include <QVector>
#include <QPointF>
#include <QPainter>
#include <QPainterPath>

/**
 * @brief Represents a single wire segment
 */
struct WireSegment {
    QPointF start;
    QPointF end;
    int segmentIndex;
    bool isVertical;
    bool isHorizontal;
};

/**
 * @brief Manages wire segments for orthogonal routing
 * 
 * Handles extraction, selection, and manipulation of
 * individual segments in orthogonal wires.
 */
class WireSegments
{
public:
    WireSegments();

    /**
     * @brief Updates segments from a painter path
     */
    void updateFromPath(const QPainterPath& path);

    /**
     * @brief Finds segment at given position
     * @return Segment index, or -1 if none found
     */
    int findSegmentAt(const QPointF& scenePos) const;

    /**
     * @brief Gets all segments
     */
    const QVector<WireSegment>& getSegments() const { return m_segments; }

    /**
     * @brief Gets a specific segment
     */
    const WireSegment& getSegment(int index) const { return m_segments[index]; }

    /**
     * @brief Gets count of segments
     */
    int count() const { return m_segments.size(); }

    /**
     * @brief Checks if segments list is empty
     */
    bool isEmpty() const { return m_segments.isEmpty(); }

    /**
     * @brief Clears all segments
     */
    void clear() { m_segments.clear(); }

    /**
     * @brief Draws adjustment arrows for selected segment
     */
    void drawSegmentArrows(QPainter* painter, int selectedSegmentIndex) const;

    static constexpr qreal SEGMENT_DETECTION_THRESHOLD = 10.0;
    static constexpr qreal ADJUSTMENT_ARROW_SIZE = 10.0;

private:
    QVector<WireSegment> m_segments;
};

#endif // WIRESEGMENTS_H
