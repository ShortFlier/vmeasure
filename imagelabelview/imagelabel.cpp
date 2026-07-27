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
        : ImageLabel(parent),
            _rect(rect),
            _angle(angle),
            _color(color),
            _baseThickness(std::max(0.1, static_cast<qreal>(thickness))),
            _thickness(std::max(0.1, static_cast<qreal>(thickness)))
{
}

ImageRotatedRectLabel::ImageRotatedRectLabel(int w, int h, QPoint center, int angle, QColor color, int thickness, QGraphicsItem *parent)
        : ImageLabel(parent),
            _rect(center.x() - w/2, center.y() - h/2, w, h),
            _angle(angle),
            _color(color),
            _baseThickness(std::max(0.1, static_cast<qreal>(thickness))),
            _thickness(std::max(0.1, static_cast<qreal>(thickness)))
{
}

void ImageRotatedRectLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    //(0,0)点为矩形中心点
    QPen pen(_color);
    pen.setWidthF(_thickness);
    painter->setPen(pen);
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
    const qreal safeScale = (viewScale > 0.0) ? viewScale : 1.0;
    const qreal newThickness = _baseThickness / safeScale;
    if (!qFuzzyCompare(newThickness, _thickness)) {
        // 线宽变化会影响 boundingRect，先通知场景更新图元几何范围。
        prepareGeometryChange();
        _thickness = newThickness;
    }
    update(); // 触发重绘
}

ImageEllipseLabel::ImageEllipseLabel(QPoint center, int radius, QColor color, int thickness, QGraphicsItem *parent)
        : ImageLabel(parent),
            _rect(center.x() - radius, center.y() - radius, radius * 2, radius * 2),
            _angle(0),
            _color(color),
            _baseThickness(std::max(0.1, static_cast<qreal>(thickness))),
            _thickness(std::max(0.1, static_cast<qreal>(thickness)))
{
}

ImageEllipseLabel::ImageEllipseLabel(QRect rect, int angle, QColor color, int thickness, QGraphicsItem *parent)
        : ImageLabel(parent),
            _rect(rect),
            _angle(angle),
            _color(color),
            _baseThickness(std::max(0.1, static_cast<qreal>(thickness))),
            _thickness(std::max(0.1, static_cast<qreal>(thickness)))
{
}

ImageEllipseLabel::ImageEllipseLabel(QPoint center, int rx, int ry, int angle, QColor color, int thickness, QGraphicsItem *parent)
    : ImageLabel(parent),
      _rect(center.x() - rx, center.y() - ry, rx * 2, ry * 2),
      _angle(angle),
      _color(color),
            _baseThickness(std::max(0.1, static_cast<qreal>(thickness))),
            _thickness(std::max(0.1, static_cast<qreal>(thickness)))
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
    QPen pen(_color);
    pen.setWidthF(_thickness);
    painter->setPen(pen);
    painter->rotate(_angle);
    painter->drawEllipse(QRectF(-_rect.width() / 2.0,
                                -_rect.height() / 2.0,
                                _rect.width(),
                                _rect.height()));
}

void ImageEllipseLabel::onViewScaleChanged(qreal viewScale)
{
    // 根据视图缩放调整标签线条粗细。
    const qreal safeScale = (viewScale > 0.0) ? viewScale : 1.0;
    const qreal newThickness = _baseThickness / safeScale;
    if (!qFuzzyCompare(newThickness, _thickness)) {
        // 线宽变化会影响 boundingRect，先通知场景更新图元几何范围。
        prepareGeometryChange();
        _thickness = newThickness;
    }
    update(); // 触发重绘
}



ImageArrowLabel::ImageArrowLabel(QPoint start, QPoint end, QColor color, int thickness, int arrowSize, QGraphicsItem *parent)
    : ImageLabel(parent),
      _start(start),
      _end(end),
      _color(color),
      _baseThickness(std::max(0.1, static_cast<qreal>(thickness))),
      _thickness(std::max(0.1, static_cast<qreal>(thickness))),
      _baseArrowSize(std::max(0.1, static_cast<qreal>(arrowSize))),
      _arrowSize(std::max(0.1, static_cast<qreal>(arrowSize)))
{
}

QRectF ImageArrowLabel::boundingRect() const{
    // 图元局部坐标以箭头中点(0,0)为原点。
    const QPointF center = (_start + _end) / 2.0;
    const QPointF startLocal = QPointF(_start) - center;
    const QPointF endLocal = QPointF(_end) - center;

    const qreal minX = std::min(startLocal.x(), endLocal.x());
    const qreal minY = std::min(startLocal.y(), endLocal.y());
    const qreal maxX = std::max(startLocal.x(), endLocal.x());
    const qreal maxY = std::max(startLocal.y(), endLocal.y());

    // 箭头头部会超出主线，额外留出边距，避免高缩放下被裁剪。
    const qreal margin = std::max(_thickness / 2.0, static_cast<qreal>(_arrowSize)) + 1.0;
    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY)).adjusted(-margin, -margin, margin, margin);
}

void ImageArrowLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // 箭头中心在(0,0)位置。
    const QPointF center = (_start + _end) / 2.0;
    const QPointF startLocal = QPointF(_start) - center;
    const QPointF endLocal = QPointF(_end) - center;

    QPen pen(_color);
    pen.setWidthF(_thickness);
    painter->setPen(pen);
    painter->drawLine(startLocal, endLocal);

    // 绘制箭头
    const double angle = std::atan2(endLocal.y() - startLocal.y(), endLocal.x() - startLocal.x());
    QPointF arrowP1 = endLocal - QPointF(std::cos(angle + M_PI / 6) * _arrowSize,
                                        std::sin(angle + M_PI / 6) * _arrowSize);
    QPointF arrowP2 = endLocal - QPointF(std::cos(angle - M_PI / 6) * _arrowSize,
                                        std::sin(angle - M_PI / 6) * _arrowSize);
    QPolygonF arrowHead;
    arrowHead << endLocal << arrowP1 << arrowP2;
    painter->setBrush(_color);
    painter->drawPolygon(arrowHead);
}

QPointF ImageArrowLabel::imagePos() const{
    return QPointF((_start.x() + _end.x()) / 2.0, (_start.y() + _end.y()) / 2.0);
}

void ImageArrowLabel::onViewScaleChanged(qreal viewScale){
    // 根据视图缩放调整标签线条粗细。
    const qreal safeScale = (viewScale > 0.0) ? viewScale : 1.0;
    const qreal newThickness = _baseThickness / safeScale;
    const qreal newArrowSize = _baseArrowSize / safeScale;
    if (!qFuzzyCompare(newThickness, _thickness) || !qFuzzyCompare(newArrowSize, _arrowSize)) {
        // 线宽变化会影响 boundingRect，先通知场景更新图元几何范围。
        prepareGeometryChange();
        _thickness = newThickness;
        _arrowSize = newArrowSize;
    }
    update(); // 触发重绘
}