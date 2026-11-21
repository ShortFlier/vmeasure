#pragma once

#include "imagelabel.h"

#include <QImage>
#include <QList>
#include <QWidget>

class QGraphicsPixmapItem;
class QGraphicsScene;
namespace Ui { class ImageLabelViewForm; }

// 图像坐标与场景坐标一致。点和文字保持屏幕尺寸，几何标签随图像缩放。
class ImageLabelView : public QWidget
{
    Q_OBJECT

public:
    explicit ImageLabelView(QWidget *parent = nullptr);
    ~ImageLabelView() override;

    // 首次设置非空图像时适配视口，后续更换图像时保持缩放和查看位置。
    void setImage(const QImage &image);
    // 仅接受未属于场景且没有父图元的标签；成功添加后由视图拥有。
    void addLabel(ImageLabel *label);
    void clearLabels();
    void setScaleFactorRange(qreal minScale, qreal maxScale);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void updateInfo(const QPoint *mousePos = nullptr);
    void updateLabels();

    Ui::ImageLabelViewForm *ui;
    QGraphicsScene *_scene;
    QGraphicsPixmapItem *_pixmapItem;
    QList<ImageLabel *> _labels;
    QImage _image;
    bool _hasInitialImage = false;
    bool _grayscale = false;
    QPoint _lastMousePos;
    bool _hasMousePos = false;
    qreal _wheelZoomStep = 1.15;
    qreal _scaleFactor = 1.0;
    qreal _minScaleFactor = 0.1;
    qreal _maxScaleFactor = 20.0;
};
