#include "imagelabel.h"

#include <algorithm>
#include <QPainter>

ImagePointLabel::ImagePointLabel(QPoint point, int size, QColor color, int thickness, int angle, QGraphicsItem *parent)
    : ImageLabel(parent), _point(point), _size(size), _color(color), _thickness(thickness), _angle(angle)
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
}


QRectF ImagePointLabel::boundingRect() const
{
    // 返回标签的边界矩形
    return QRectF(-_size/2, -_size/2, _size, _size);
}

void ImagePointLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(QPen(_color, _thickness));
    painter->translate(0, 0); // 将绘制原点移动到标签中心
    painter->rotate(_angle);
    painter->drawLine(-_size/2, 0, _size/2, 0);
    painter->drawLine(0, -_size/2, 0, _size/2);
}

ImageRotatedRectLabel::ImageRotatedRectLabel(QRect rect, int angle, QColor color, int thickness, QGraphicsItem *parent)
    : ImageLabel(parent), _rect(rect), _angle(angle), _color(color), _thickness(thickness)
{
}

ImageRotatedRectLabel::ImageRotatedRectLabel(int w, int h, QPoint center, int angle, QColor color, int thickness, QGraphicsItem *parent)
    : ImageLabel(parent), _rect(center.x() - w/2, center.y() - h/2, w, h), _angle(angle), _color(color), _thickness(thickness)
{
}

void ImageRotatedRectLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    //(0,0)点为矩形中心点
    painter->setPen(QPen(_color, _thickness));
    painter->translate(0, 0);
    painter->rotate(_angle);
    painter->drawRect(QRectF(-_rect.width()/2, -_rect.height()/2, _rect.width(), _rect.height()));
}

QRectF ImageRotatedRectLabel::boundingRect() const{
    // 图元局部坐标以中心点(0,0)为原点，boundingRect 必须与 paint 使用同一坐标系。
    QRectF localRect(-_rect.width() / 2.0,
                     -_rect.height() / 2.0,
                     _rect.width(),
                     _rect.height());

    // 计算旋转后的外接矩形，并额外留出线宽边距，避免高缩放下被裁剪。
    QTransform transform;
    transform.rotate(_angle);
    QRectF rotatedRect = transform.mapRect(localRect);
    const qreal margin = _thickness / 2.0 + 1.0;
    return rotatedRect.adjusted(-margin, -margin, margin, margin);
}


void ImageRotatedRectLabel::onViewScaleChanged(qreal viewScale)
{
    // 根据视图缩放调整标签线条粗细。
    const int newThickness = std::max(1, static_cast<int>(4 / viewScale));
    if (newThickness != _thickness) {
        // 线宽变化会影响 boundingRect，先通知场景更新图元几何范围。
        prepareGeometryChange();
        _thickness = newThickness;
    }
    update(); // 触发重绘
}

ImageEllipseLabel::ImageEllipseLabel(QPoint center, int radius, QColor color, int thickness, QGraphicsItem *parent)
    : ImageLabel(parent), _rect(center.x() - radius, center.y() - radius, radius * 2, radius * 2), _angle(0), _color(color), _thickness(thickness)
{
}

ImageEllipseLabel::ImageEllipseLabel(QRect rect, int angle, QColor color, int thickness, QGraphicsItem *parent)
    : ImageLabel(parent), _rect(rect), _angle(angle), _color(color), _thickness(thickness)
{
}

ImageEllipseLabel::ImageEllipseLabel(QPoint center, int rx, int ry, int angle, QColor color, int thickness, QGraphicsItem *parent)
    : ImageLabel(parent),
      _rect(center.x() - rx, center.y() - ry, rx * 2, ry * 2),
      _angle(angle),
      _color(color),
      _thickness(thickness)
{
}

QRectF ImageEllipseLabel::boundingRect() const
{
    // 图元局部坐标以椭圆中心点(0,0)为原点。
    QRectF localRect(-_rect.width() / 2.0,
                     -_rect.height() / 2.0,
                     _rect.width(),
                     _rect.height());

    // 旋转后的外接矩形。
    QTransform transform;
    transform.rotate(_angle);
    QRectF rotatedRect = transform.mapRect(localRect);

    const qreal margin = _thickness / 2.0 + 1.0;
    return rotatedRect.adjusted(-margin, -margin, margin, margin);
}

void ImageEllipseLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // (0,0)点为椭圆中心点。
    painter->setPen(QPen(_color, _thickness));
    painter->rotate(_angle);
    painter->drawEllipse(QRectF(-_rect.width() / 2.0,
                                -_rect.height() / 2.0,
                                _rect.width(),
                                _rect.height()));
}

void ImageEllipseLabel::onViewScaleChanged(qreal viewScale)
{
    // 根据视图缩放调整标签线条粗细。
    const int newThickness = std::max(1, static_cast<int>(4 / viewScale));
    if (newThickness != _thickness) {
        // 线宽变化会影响 boundingRect，先通知场景更新图元几何范围。
        prepareGeometryChange();
        _thickness = newThickness;
    }
    update(); // 触发重绘
}