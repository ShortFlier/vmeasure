#include "imagelabelview.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QTextDocument>
#include <QGraphicsDropShadowEffect>

ImageLabelView::ImageLabelView(QWidget *parent)
    : QGraphicsView(parent)
{
    _scene=new QGraphicsScene(this);
    this->setScene(_scene);

    _pixmapItem=new QGraphicsPixmapItem();
    _scene->addItem(_pixmapItem);

    _infoTextItem=new QGraphicsTextItem();
    _scene->addItem(_infoTextItem);
    _infoTextItem->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    _infoTextItem->setZValue(1000);
    _infoTextItem->setDefaultTextColor(Qt::white);

    auto* textShadow = new QGraphicsDropShadowEffect(this);
    textShadow->setBlurRadius(6.0);
    textShadow->setOffset(0.0, 0.0);
    textShadow->setColor(QColor(0, 0, 0, 220));
    _infoTextItem->setGraphicsEffect(textShadow);

    // 启用QGraphicsView内建的手型拖拽平移
    this->setDragMode(QGraphicsView::ScrollHandDrag);
    this->setInteractive(true);
    this->setMouseTracking(true);
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->setResizeAnchor(QGraphicsView::AnchorViewCenter);


    //隐藏滚动条
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}



void ImageLabelView::setImage(const QImage& image)
{
    _image=image;
    //修改缩放为刚好容纳整张图片
    _scaleFactor=qMin(static_cast<qreal>(this->viewport()->width()) / _image.width(),
                      static_cast<qreal>(this->viewport()->height()) / _image.height());
    
    _scene->setSceneRect(0, 0, _image.width(), _image.height());

    resetTransform();
    scale(_scaleFactor, _scaleFactor);


    updatePixmapItem();
    updateLabels();
}

void ImageLabelView::updatePixmapItem(){
    _pixmapItem->setPixmap(QPixmap::fromImage(_image));

    
    //将图形项左上角放置在场景坐标(0,0)位置
    _pixmapItem->setPos(0, 0);
}

void ImageLabelView::addLabel(ImageLabel* label)
{
    _labels.append(label);
    _scene->addItem(label);
    label->setPos(label->imagePos());
    updateLabel(label);
}

void ImageLabelView::clearLabels()
{
    for (ImageLabel* label : _labels) {
        _scene->removeItem(label);
        delete label;
    }
    _labels.clear();
}


void ImageLabelView::updateLabel(ImageLabel* label)
{
    // 场景坐标与图像像素坐标保持一致
    //label->setPos(label->imagePos());

    // 变换策略由标签子类自行决定
    label->onViewScaleChanged(_scaleFactor);
}

void ImageLabelView::updateLabels()
{
    for (ImageLabel* label : _labels) {
        updateLabel(label);
    }
}


void ImageLabelView::updateInfoText(const QPointF& mousePos)
{
    //信息显示设置位于控件左下角
    QString infoText("Image Size: %1 x %2     Scale: %3     ");
    infoText=infoText.arg(_image.width()).arg(_image.height()).arg(QString::number(_scaleFactor, 'f', 2));

    //当前像素信息
    QString pixelInfo("(%1, %2)");
    //转为场景坐标
    QPointF scenePos = mapToScene(mousePos.toPoint());
    QPointF imagePos = scenePos; // 场景坐标直接就是图像像素坐标
    int x = static_cast<int>(imagePos.x());
    int y = static_cast<int>(imagePos.y());
    pixelInfo=pixelInfo.arg(x).arg(y);
    if(x >= 0 && x < _image.width() && y >= 0 && y < _image.height()) {
        QColor pixelColor = QColor::fromRgba(_image.pixel(x, y));
        pixelInfo += QString(" Color: R=%1 G=%2 B=%3 A=%4")
                .arg(pixelColor.red())
                .arg(pixelColor.green())
                .arg(pixelColor.blue())
                .arg(pixelColor.alpha());
    } else {
        pixelInfo += "Out of bounds";
    }


    _infoTextItem->setPlainText(infoText + pixelInfo);

    updateInfoTextPosition();
}


void ImageLabelView::updateInfoTextPosition()
{
    QRectF textRect = _infoTextItem->boundingRect();
    QPoint viewPos(5, this->viewport()->height() - static_cast<int>(textRect.height()) - 5);
    _infoTextItem->setPos(mapToScene(viewPos));
}


void ImageLabelView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    updateInfoTextPosition();
}

void ImageLabelView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    _lastMousePos= event->pos();
    updateInfoText(event->pos());
}


void ImageLabelView::wheelEvent(QWheelEvent *event)
{
    qreal oldScale = _scaleFactor;
    if (event->angleDelta().y() > 0) {
        _scaleFactor *= _wheelZoomStep;
    } else {
        _scaleFactor /= _wheelZoomStep;
    }
    if (_scaleFactor < _minScaleFactor) _scaleFactor = _minScaleFactor;
    if (_scaleFactor > _maxScaleFactor) _scaleFactor = _maxScaleFactor;

    qreal factor = _scaleFactor / oldScale;
    scale(factor, factor);

    updateLabels();

    updateInfoText(_lastMousePos);
    updateInfoTextPosition();

    event->accept();
}