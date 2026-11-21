#pragma once

#include <QColor>
#include <QFont>
#include <QGraphicsItem>


// 点和文字的大小以屏幕像素为单位。
// 几何标签的尺寸以图像像素为单位，线宽以屏幕像素为单位。
class ImageLabel : public QGraphicsItem
{
public:
    explicit ImageLabel(QGraphicsItem *parent = nullptr) : QGraphicsItem(parent) {}
    virtual void onViewScaleChanged(qreal viewScale) = 0;
    virtual QPointF imagePos() const = 0;
};

//交叉点
class ImagePointLabel : public ImageLabel
{
public:
    ImagePointLabel(QPointF point, int size = 20, QColor color = Qt::red,
                    int thickness = 4, int angle = 45, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPointF imagePos() const override { return _point; }
    void onViewScaleChanged(qreal viewScale) override { Q_UNUSED(viewScale); }

private:
    QPointF _point;
    int _size;
    QColor _color;
    int _thickness;
    int _angle;
};

//文字显示
class ImageTextLabel : public ImageLabel
{
public:
    ImageTextLabel(QPointF point, QString text, QColor color = Qt::red,
                   QFont font = QFont("Arial", 12), QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPointF imagePos() const override { return _point; }
    void onViewScaleChanged(qreal viewScale) override { Q_UNUSED(viewScale); }

private:
    QPointF _point;
    QString _text;
    QColor _color;
    QFont _font;
};

//矩形、旋转矩形
class ImageRotatedRectLabel : public ImageLabel
{
public:
    ImageRotatedRectLabel(QRect rect, int angle = 0, QColor color = Qt::red,
                          int thickness = 4, QGraphicsItem *parent = nullptr);
    ImageRotatedRectLabel(int w, int h, QPoint center, int angle = 0, QColor color = Qt::red,
                          int thickness = 4, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPointF imagePos() const override { return _rect.center(); }
    void onViewScaleChanged(qreal viewScale) override;

private:
    QRectF _rect;
    int _angle;
    QColor _color;
    qreal _baseThickness;
    qreal _thickness;
};

//圆、椭圆
class ImageEllipseLabel : public ImageLabel
{
public:
    ImageEllipseLabel(QPoint center, int radius, QColor color = Qt::red,
                      int thickness = 4, QGraphicsItem *parent = nullptr);
    ImageEllipseLabel(QRect rect, int angle, QColor color = Qt::red,
                      int thickness = 4, QGraphicsItem *parent = nullptr);
    ImageEllipseLabel(QPoint center, int rx, int ry, int angle, QColor color = Qt::red,
                      int thickness = 4, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPointF imagePos() const override { return _rect.center(); }
    void onViewScaleChanged(qreal viewScale) override;

private:
    QRectF _rect;
    int _angle;
    QColor _color;
    qreal _baseThickness;
    qreal _thickness;
};

//箭头
class ImageArrowLabel : public ImageLabel
{
public:
    ImageArrowLabel(QPoint start, QPoint end, QColor color = Qt::red,
                    int thickness = 2, int arrowSize = 5, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPointF imagePos() const override;
    void onViewScaleChanged(qreal viewScale) override;

private:
    QPointF _start;
    QPointF _end;
    QColor _color;
    qreal _baseThickness;
    qreal _thickness;
    qreal _baseArrowSize;
    qreal _arrowSize;
};
