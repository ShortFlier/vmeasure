#pragma once

/*
* 图像及其标签查看控件
* 支持基本的缩放、移动查看图片
* 支持多种类型标签，在imagelabel.h中定义
* 暂不支持修改标签，仅支持查看
* 性能优化
*/

#include "imagelabel.h"

#include <QGraphicsView>


/*
* 图片大小、当前缩放比例、当前像素信息使用QGraphicsTextItem显示在控件下方
* 鼠标滚轮缩放查看图片，标签位置始终保持在图形像素位置，标签大小不随缩放改变
*/
class ImageLabelView : public QGraphicsView
{
    public:
        ImageLabelView(QWidget *parent = nullptr);

        void setImage(const QImage& image);

        void addLabel(ImageLabel* label);
        void clearLabels();

        void setScaleFactorRange(qreal minScale, qreal maxScale){
            _minScaleFactor = minScale;
            _maxScaleFactor = maxScale;
        }

    protected:
        // 重写鼠标滚轮事件，实现缩放功能，标签大小始终保持不变
        void wheelEvent(QWheelEvent *event) override;

        void mouseMoveEvent(QMouseEvent *event) override;

        void resizeEvent(QResizeEvent *event) override;

    private:
        //更新图像信息显示
        void updateInfoText(const QPointF& mousePos);
        //始终位于左下角
        void updateInfoTextPosition();

        //更新图片缩放大小，位置
        void updatePixmapItem();

        //统一更新标签位置和缩放
        void updateLabels();
        void updateLabel(ImageLabel* label);

        QGraphicsScene* _scene;
        QGraphicsPixmapItem* _pixmapItem;
        QGraphicsTextItem* _infoTextItem;

        QList<ImageLabel*> _labels;

        QImage _image;

        QPointF _lastMousePos; //鼠标移动时的上一次位置

        qreal _wheelZoomStep=1.15; // 滚轮缩放步长
        qreal _scaleFactor=1.0; // 当前视图缩放比例
        qreal _minScaleFactor=0.1; // 最小缩放比例
        qreal _maxScaleFactor=20.0; // 最大缩放比例
};