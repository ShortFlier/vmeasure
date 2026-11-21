#include "imagelabel.h"

#include <QFontMetricsF>
#include <QPainter>
#include <algorithm>
#include <cmath>
#include <utility>

namespace {
QRectF rotatedBounds(const QRectF &rect, int angle, qreal thickness)
{
    QTransform transform;
    transform.rotate(angle);
    const qreal margin = thickness * 0.7071067811865476 + 1.0;
    return transform.mapRect(rect).adjusted(-margin, -margin, margin, margin);
}

QRectF centeredRect(const QRectF &rect)
{
    return QRectF(-rect.width() / 2.0, -rect.height() / 2.0, rect.width(), rect.height());
}

qreal scaledThickness(qreal base, qreal scale)
{
    return base / (scale > 0.0 ? scale : 1.0);
}
}

ImagePointLabel::ImagePointLabel(QPointF point, int size, QColor color, int thickness, int angle,
                                 QGraphicsItem *parent)
    : ImageLabel(parent), _point(point), _size(size), _color(color),
      _thickness(thickness), _angle(angle)
{
    setFlag(ItemIgnoresTransformations);
}

QRectF ImagePointLabel::boundingRect() const
{
    const qreal margin = _thickness / 2.0 + 1.0;
    const qreal radius = _size / 2.0 + margin;
    return QRectF(-radius, -radius, 2 * radius, 2 * radius);
}

void ImagePointLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(QPen(_color, _thickness));
    painter->rotate(_angle);
    const qreal half = _size / 2.0;
    painter->drawLine(QPointF(-half, 0), QPointF(half, 0));
    painter->drawLine(QPointF(0, -half), QPointF(0, half));
}

ImageTextLabel::ImageTextLabel(QPointF point, QString text, QColor color, QFont font,
                               QGraphicsItem *parent)
    : ImageLabel(parent), _point(point), _text(std::move(text)), _color(color), _font(std::move(font))
{
    setFlag(ItemIgnoresTransformations);
}

QRectF ImageTextLabel::boundingRect() const
{
    return QFontMetricsF(_font).boundingRect(_text);
}

void ImageTextLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(_color);
    painter->setFont(_font);
    painter->drawText(QPointF(0, 0), _text);
}

ImageRotatedRectLabel::ImageRotatedRectLabel(QRect rect, int angle, QColor color, int thickness,
                                             QGraphicsItem *parent)
    : ImageLabel(parent), _rect(rect), _angle(angle), _color(color),
      _baseThickness(std::max(qreal(0.1), qreal(thickness))), _thickness(_baseThickness)
{
}

ImageRotatedRectLabel::ImageRotatedRectLabel(int w, int h, QPoint center, int angle, QColor color,
                                             int thickness, QGraphicsItem *parent)
    : ImageLabel(parent), _rect(center.x() - w / 2.0, center.y() - h / 2.0, w, h),
      _angle(angle), _color(color), _baseThickness(std::max(qreal(0.1), qreal(thickness))),
      _thickness(_baseThickness)
{
}

QRectF ImageRotatedRectLabel::boundingRect() const
{
    return rotatedBounds(centeredRect(_rect), _angle, _thickness);
}

void ImageRotatedRectLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen(_color);
    pen.setWidthF(_thickness);
    painter->setPen(pen);
    painter->rotate(_angle);
    painter->drawRect(centeredRect(_rect));
}

void ImageRotatedRectLabel::onViewScaleChanged(qreal viewScale)
{
    const qreal thickness = scaledThickness(_baseThickness, viewScale);
    if (!qFuzzyCompare(thickness, _thickness)) {
        prepareGeometryChange();
        _thickness = thickness;
        update();
    }
}

ImageEllipseLabel::ImageEllipseLabel(QPoint center, int radius, QColor color, int thickness,
                                     QGraphicsItem *parent)
    : ImageEllipseLabel(center, radius, radius, 0, color, thickness, parent)
{
}

ImageEllipseLabel::ImageEllipseLabel(QRect rect, int angle, QColor color, int thickness,
                                     QGraphicsItem *parent)
    : ImageLabel(parent), _rect(rect), _angle(angle), _color(color),
      _baseThickness(std::max(qreal(0.1), qreal(thickness))), _thickness(_baseThickness)
{
}

ImageEllipseLabel::ImageEllipseLabel(QPoint center, int rx, int ry, int angle, QColor color,
                                     int thickness, QGraphicsItem *parent)
    : ImageLabel(parent), _rect(center.x() - rx, center.y() - ry, 2.0 * rx, 2.0 * ry),
      _angle(angle), _color(color), _baseThickness(std::max(qreal(0.1), qreal(thickness))),
      _thickness(_baseThickness)
{
}

QRectF ImageEllipseLabel::boundingRect() const
{
    return rotatedBounds(centeredRect(_rect), _angle, _thickness);
}

void ImageEllipseLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen(_color);
    pen.setWidthF(_thickness);
    painter->setPen(pen);
    painter->rotate(_angle);
    painter->drawEllipse(centeredRect(_rect));
}

void ImageEllipseLabel::onViewScaleChanged(qreal viewScale)
{
    const qreal thickness = scaledThickness(_baseThickness, viewScale);
    if (!qFuzzyCompare(thickness, _thickness)) {
        prepareGeometryChange();
        _thickness = thickness;
        update();
    }
}

ImageArrowLabel::ImageArrowLabel(QPoint start, QPoint end, QColor color, int thickness,
                                 int arrowSize, QGraphicsItem *parent)
    : ImageLabel(parent), _start(start), _end(end), _color(color),
      _baseThickness(std::max(qreal(0.1), qreal(thickness))), _thickness(_baseThickness),
      _baseArrowSize(std::max(qreal(0.1), qreal(arrowSize))), _arrowSize(_baseArrowSize)
{
}

QPointF ImageArrowLabel::imagePos() const
{
    return (_start + _end) / 2.0;
}

QRectF ImageArrowLabel::boundingRect() const
{
    const QPointF center = imagePos();
    const qreal margin = _arrowSize + _thickness / 2.0 + 1.0;
    return QRectF(_start - center, _end - center).normalized()
        .adjusted(-margin, -margin, margin, margin);
}

void ImageArrowLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const QPointF start = _start - imagePos();
    const QPointF end = _end - imagePos();
    QPen pen(_color);
    pen.setWidthF(_thickness);
    painter->setPen(pen);
    painter->drawLine(start, end);
    const qreal angle = std::atan2(end.y() - start.y(), end.x() - start.x());
    const qreal offset = 3.14159265358979323846 / 6.0;
    QPolygonF head;
    head << end
         << end - QPointF(std::cos(angle + offset) * _arrowSize, std::sin(angle + offset) * _arrowSize)
         << end - QPointF(std::cos(angle - offset) * _arrowSize, std::sin(angle - offset) * _arrowSize);
    painter->setBrush(_color);
    painter->drawPolygon(head);
}

void ImageArrowLabel::onViewScaleChanged(qreal viewScale)
{
    const qreal thickness = scaledThickness(_baseThickness, viewScale);
    const qreal arrowSize = scaledThickness(_baseArrowSize, viewScale);
    if (!qFuzzyCompare(thickness, _thickness) || !qFuzzyCompare(arrowSize, _arrowSize)) {
        prepareGeometryChange();
        _thickness = thickness;
        _arrowSize = arrowSize;
        update();
    }
}
