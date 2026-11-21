#include "imagelabelview.h"
#include "ui_imagelabelview.h"

#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>
#include <QtMath>
#include <cmath>

ImageLabelView::ImageLabelView(QWidget *parent)
    : QWidget(parent), ui(new Ui::ImageLabelViewForm), _scene(new QGraphicsScene(this)),
      _pixmapItem(new QGraphicsPixmapItem)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(_scene);
    _scene->addItem(_pixmapItem);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setInteractive(true);
    ui->graphicsView->setMouseTracking(true);
    ui->graphicsView->viewport()->setMouseTracking(true);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->viewport()->installEventFilter(this);
    const auto updatePosition = [this]() {
        updateInfo(_hasMousePos ? &_lastMousePos : nullptr);
    };
    connect(ui->graphicsView->horizontalScrollBar(), &QScrollBar::valueChanged, this, updatePosition);
    connect(ui->graphicsView->verticalScrollBar(), &QScrollBar::valueChanged, this, updatePosition);
    updateInfo();
}

ImageLabelView::~ImageLabelView()
{
    delete ui;
}

void ImageLabelView::setImage(const QImage &image)
{
    const QPointF previousCenter = ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect().center());
    _image = image;
    const QImage::Format format = _image.format();
    _grayscale = format == QImage::Format_Grayscale8
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
        || format == QImage::Format_Grayscale16
#endif
        || ((format == QImage::Format_Indexed8 || format == QImage::Format_Mono
             || format == QImage::Format_MonoLSB)
            && !_image.hasAlphaChannel() && _image.allGray());
    _pixmapItem->setPixmap(QPixmap::fromImage(_image));
    if (!_image.isNull()) {
        const QRectF newSceneRect(0, 0, _image.width(), _image.height());
        const bool sceneRectChanged = _scene->sceneRect() != newSceneRect;
        if (sceneRectChanged)
            _scene->setSceneRect(newSceneRect);
        if (!_hasInitialImage) {
            ui->graphicsView->resetTransform();
            _scaleFactor = 1.0;
            const QSize viewportSize = ui->graphicsView->viewport()->size();
            if (viewportSize.width() > 0 && viewportSize.height() > 0) {
                _scaleFactor = qMin(qreal(viewportSize.width()) / _image.width(),
                                    qreal(viewportSize.height()) / _image.height());
            }
            ui->graphicsView->scale(_scaleFactor, _scaleFactor);
            ui->graphicsView->centerOn(_scene->sceneRect().center());
            _hasInitialImage = true;
        } else if (sceneRectChanged) {
            ui->graphicsView->centerOn(previousCenter);
        }
    } else if (!_hasInitialImage) {
        _scene->setSceneRect(0, 0, 1, 1);
    }
    updateLabels();
    updateInfo(_hasMousePos ? &_lastMousePos : nullptr);
}

void ImageLabelView::addLabel(ImageLabel *label)
{
    if (!label || _labels.contains(label)) {
        qWarning() << "ImageLabelView: null or duplicate label";
        return;
    }
    if (label->parentItem() || label->scene()) {
        qWarning() << "ImageLabelView: label already has a parent or scene";
        return;
    }
    _scene->addItem(label);
    _labels.append(label);
    label->setPos(label->imagePos());
    label->onViewScaleChanged(_scaleFactor);
}

void ImageLabelView::clearLabels()
{
    qDeleteAll(_labels);
    _labels.clear();
}

void ImageLabelView::setScaleFactorRange(qreal minScale, qreal maxScale)
{
    if (!std::isfinite(minScale) || !std::isfinite(maxScale)
        || minScale <= 0 || maxScale < minScale) {
        qWarning() << "ImageLabelView: invalid scale range" << minScale << maxScale;
        return;
    }
    _minScaleFactor = minScale;
    _maxScaleFactor = maxScale;
}

void ImageLabelView::updateLabels()
{
    for (ImageLabel *label : _labels)
        label->onViewScaleChanged(_scaleFactor);
}

void ImageLabelView::updateInfo(const QPoint *mousePos)
{
    ui->imgSizeLabel->setText(tr("图像大小:%1x%2").arg(_image.width()).arg(_image.height()));
    ui->scaleLabel->setText(tr("当前缩放:%1%").arg(_scaleFactor * 100, 0, 'f', 1));
    ui->positionLabel->setText(tr("位置:--"));
    ui->pixInfoLabel->setText(tr("像素:--"));
    if (!mousePos || _image.isNull())
        return;

    const QPointF pos = ui->graphicsView->mapToScene(*mousePos);
    const int x = qFloor(pos.x());
    const int y = qFloor(pos.y());
    ui->positionLabel->setText(tr("位置:(%1,%2)").arg(x).arg(y));
    if (x >= 0 && x < _image.width() && y >= 0 && y < _image.height()) {
        const QColor color = _image.pixelColor(x, y);
        if (_grayscale) {
            int value = color.red();
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
            if (_image.format() == QImage::Format_Grayscale16)
                value = reinterpret_cast<const quint16 *>(_image.constScanLine(y))[x];
#endif
            ui->pixInfoLabel->setText(tr("像素:(%1)").arg(value));
        } else if (_image.hasAlphaChannel()) {
            ui->pixInfoLabel->setText(tr("像素:(%1,%2,%3,%4)")
                                      .arg(color.red()).arg(color.green())
                                      .arg(color.blue()).arg(color.alpha()));
        } else {
            ui->pixInfoLabel->setText(tr("像素:(%1,%2,%3)")
                                      .arg(color.red()).arg(color.green()).arg(color.blue()));
        }
    }
}

bool ImageLabelView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->graphicsView->viewport()) {
        if (event->type() == QEvent::MouseMove) {
            _lastMousePos = static_cast<QMouseEvent *>(event)->pos();
            _hasMousePos = true;
            updateInfo(&_lastMousePos);
        } else if (event->type() == QEvent::Leave) {
            _hasMousePos = false;
            updateInfo();
        } else if (event->type() == QEvent::Wheel) {
            auto *wheel = static_cast<QWheelEvent *>(event);
            if (wheel->angleDelta().y() == 0)
                return QWidget::eventFilter(watched, event);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            _lastMousePos = wheel->position().toPoint();
#else
            _lastMousePos = wheel->pos();
#endif
            _hasMousePos = true;
            const qreal desired = _scaleFactor *
                (wheel->angleDelta().y() > 0 ? _wheelZoomStep : 1.0 / _wheelZoomStep);
            const qreal newScale = qBound(_minScaleFactor, desired, _maxScaleFactor);
            ui->graphicsView->scale(newScale / _scaleFactor, newScale / _scaleFactor);
            _scaleFactor = newScale;
            updateLabels();
            updateInfo(&_lastMousePos);
            wheel->accept();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}
