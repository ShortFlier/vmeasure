#pragma once

#include <QGraphicsItem>

class ImageLabel : public QGraphicsItem
{
    public:
        ImageLabel(QGraphicsItem *parent = nullptr) : QGraphicsItem(parent) {}

        // 视图缩放变化时由子类自行决定如何处理。
        // 例如点标签可忽略视图变换；矩形/圆标签可仅调整边框粗细。
        virtual void onViewScaleChanged(qreal viewScale)=0;

        virtual QPointF imagePos() const = 0;
};

/*
* 图像点标签
* 十字标记
*图形项(0,0)位置是点标签的中心点
*/
class ImagePointLabel : public ImageLabel
{
    public:
        /*
        * @param point: 相对于图像的坐标
        * @param size: 标签大小
        * @param color: 标签颜色
        * @param thickness: 标签线条粗细
        * @param angle: 标签旋转角度，影响十字线角度，不会影响标签大小，逆时针为正方向
        */
        ImagePointLabel(QPoint point, int size=20, QColor color = Qt::red, int thickness = 4, int angle = 45, QGraphicsItem *parent = nullptr);

        QRectF boundingRect() const override;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPointF imagePos() const override {
            return QPointF(_point);
        }

        void onViewScaleChanged(qreal viewScale) override{
            Q_UNUSED(viewScale);
            // 点标签通过 ItemIgnoresTransformations 保持屏幕尺寸，
            // 此处无需随视图缩放做额外处理。
        }

    private:
        QPoint _point; //相对于图像的坐标
        int _size; //标签大小
        QColor _color; //标签颜色
        int _thickness; //标签线条粗细
        int _angle; //标签旋转角度
};


/*
*旋转矩形标签
*图形项(0,0)位置是矩形中心点
*/
class ImageRotatedRectLabel : public ImageLabel
{
    public:
        /*
        * @param rect: 相对于图像的矩形
        * @param angle: 矩形旋转角度，逆时针为正方向
        * @param color: 标签颜色
        * @param thickness: 标签线条粗细
        */
        ImageRotatedRectLabel(QRect rect, int angle=0, QColor color = Qt::red, int thickness = 4, QGraphicsItem *parent = nullptr);

        /*
        * @param w: 矩形宽度
        * @param h: 矩形高度
        * @param center: 矩形中心点
        */
        ImageRotatedRectLabel(int w, int h, QPoint center, int angle=0, QColor color = Qt::red, int thickness = 4, QGraphicsItem *parent = nullptr);

        QRectF boundingRect() const override;
        
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPointF imagePos() const override {
            return QPointF(_rect.center());
        }

        void onViewScaleChanged(qreal viewScale) override;

    private:
        QRect _rect; //相对于图像的矩形
        int _angle; //矩形旋转角度
        QColor _color; //标签颜色
        int _baseThickness; //标签基准线条粗细
        int _thickness; //标签线条粗细
};



/*
*圆及椭圆标签
*图形项(0,0)位置是圆/椭圆的中心点
*/
class ImageEllipseLabel : public ImageLabel
{
    public:
        /*
        * @param center: 圆心相对于图像的坐标
        * @param radius: 圆半径
        * @param color: 标签颜色
        * @param thickness: 标签线条粗细
        */
        ImageEllipseLabel(QPoint center, int radius, QColor color = Qt::red, int thickness = 4, QGraphicsItem *parent = nullptr);

        /*
        * @param rect: 相对于图像的矩形
        * @param angle: 椭圆旋转角度，逆时针为正方向
        * @param color: 标签颜色
        * @param thickness: 标签线条粗细
        */
        ImageEllipseLabel(QRect rect, int angle, QColor color = Qt::red, int thickness = 4, QGraphicsItem *parent = nullptr);

        /*
        * @param rx: 椭圆水平半径
        * @param ry: 椭圆垂直半径
        * @param center: 椭圆中心点
        * @param angle: 椭圆旋转角度，逆时针为正方向
        */
        ImageEllipseLabel(QPoint center, int rx, int ry, int angle, QColor color = Qt::red, int thickness = 4, QGraphicsItem *parent = nullptr);

        QRectF boundingRect() const override;
        
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPointF imagePos() const override {
            return QPointF(_rect.center());
        }

        void onViewScaleChanged(qreal viewScale) override;

    private:
        QRect _rect; //相对于图像的矩形
        int _angle; //椭圆旋转角度
        QColor _color; //标签颜色
        int _baseThickness; //标签基准线条粗细
        int _thickness; //标签线条粗细
};




/*
*箭头标签
*图形项(0,0)位置是箭头起点和终点的中心点
*/
class ImageArrowLabel : public ImageLabel
{
    public:
        /*
        * @param start: 箭头起点相对于图像的坐标
        * @param end: 箭头终点相对于图像的坐标
        * @param color: 标签颜色
        * @param thickness: 标签线条粗细
        */
        ImageArrowLabel(QPoint start, QPoint end, QColor color = Qt::red, int thickness = 2, int arrowSize = 5, QGraphicsItem *parent = nullptr);

        QRectF boundingRect() const override;
        
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        QPointF imagePos() const override;

        void onViewScaleChanged(qreal viewScale) override;

    private:
        QPoint _start; //箭头起点相对于图像的坐标
        QPoint _end; //箭头终点相对于图像的坐标
        QColor _color; //标签颜色
        int _baseThickness; //标签基准线条粗细
        int _thickness; //标签线条粗细
        int _arrowSize; //箭头大小
};