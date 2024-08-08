//
// Created by benichn on 12/07/24.
//

#include "ImageViewerWidget.h"

void ImageViewerWidget::MoveMouseAction::onMove()
{
    _sr->offset(lastDiff());
    _sr->updateSelectionRect();
}

unique_ptr<MouseAction> ImageViewerWidget::MouseHandler::createMouseAction(QMouseEvent* e)
{
    switch (e->button())
    {
    case Qt::RightButton:
        return make_unique<MoveMouseAction>(MoveMouseAction(_sr));
    default:
        return nullptr;
    }
}

void ImageViewerWidget::MouseHandler::wheelEvent(QWheelEvent* e)
{
    const QPoint p = e->pixelDelta();
    // const int xs = sgn(p.x());
    const int ys = sgn(p.y());
    const float newZoomLevel = ceilAbs(_sr->_zoomLevel) + ys;
    _sr->zoomToLevel(newZoomLevel, _sr->_imageLabel->mapFromParent(e->position()));
}

int ImageViewerWidget::adjustPos(int L, int l, int x)
{
    return l < L ? (L - l) / 2 : max(L - l, min(x, 0));
}

ImageViewerWidget::ImageViewerWidget(QWidget* parent) : QWidget(parent),
                                                        _imageLabel(new QLabel(this)),
                                                        _selectionRect(new SelectionRectWidget(this))
{
    _imageLabel->setScaledContents(true);
}

void ImageViewerWidget::offset(const QPoint& v) const
{
    const QPoint newPos = _imageLabel->pos() + v;
    _imageLabel->move(QPoint(
        adjustPos(width(), _imageLabel->width(), newPos.x()),
        adjustPos(height(), _imageLabel->height(), newPos.y())));
}

void ImageViewerWidget::zoomToLevel(const float level, const QPointF& p)
{
    const float newZoomLevel = min(_maxZoomLevel, max(_minZoomLevel, level));
    const float factor = powf(WHEEL_FACTOR, newZoomLevel);
    const float relativeFactor = powf(WHEEL_FACTOR, newZoomLevel - _zoomLevel);
    const QSizeF newSize = _pixmap.size() * factor;
    _zoomLevel = newZoomLevel;
    _imageLabel->resize(newSize.toSize());
    offset(((1 - relativeFactor) * p).toPoint());
    updateSelectionRect();
}

void ImageViewerWidget::setPixmap(const QPixmap& pixmap)
{
    _pixmap = pixmap;
    _selectionRect->resetDisabled();
    _imageLabel->setPixmap(_pixmap);
    computeZoomLevels();
    zoomToLevel(0, QPointF());
}

void ImageViewerWidget::setPixmapAndRect(const QPixmap& pixmap)
{
    _pixmap = pixmap;
    _selectionRect->resetRect(_pixmap.size());
    _imageLabel->setPixmap(_pixmap);
    computeZoomLevels();
    zoomToLevel(0, QPointF());
}

void ImageViewerWidget::setPixmapAndRect(const QPixmap& pixmap, const QImage& image)
{
    _pixmap = pixmap;
    _selectionRect->resetRect(image);
    _imageLabel->setPixmap(_pixmap);
    computeZoomLevels();
    zoomToLevel(0, QPointF());
}

void ImageViewerWidget::setPixmapAndPicker(const QPixmap& pixmap, const vector<PickerElement>& elements)
{
    _pixmap = pixmap;
    _selectionRect->resetPicker(_pixmap.size(), elements);
    _imageLabel->setPixmap(_pixmap);
    computeZoomLevels();
    zoomToLevel(0, QPointF());
}

void ImageViewerWidget::setSRDisabled()
{
    _selectionRect->resetDisabled();
}

void ImageViewerWidget::setSRRect()
{
    _selectionRect->resetRect(_pixmap.size());
}

void ImageViewerWidget::setSRRect(const QImage& sel)
{
    _selectionRect->resetRect(sel);
}

void ImageViewerWidget::setSRPicker(const vector<PickerElement>& elements)
{
    _selectionRect->resetPicker(_pixmap.size(), elements);
}

void ImageViewerWidget::updateSelectionRect() const
{
    _selectionRect->setGeometry(_imageLabel->geometry());
}

void ImageViewerWidget::computeZoomLevels()
{
    const QSize imgSize = _pixmap.size();
    const QSize size = this->size();
    const float minZoomFactor = min(
        static_cast<float>(size.width()) / imgSize.width(),
        static_cast<float>(size.height()) / imgSize.height());
    _minZoomLevel = logf(minZoomFactor) / WHEEL_FACTOR_LOG;
    _maxZoomLevel = 0;
}

bool ImageViewerWidget::event(QEvent* e)
{
    return _mouseHandler.event(e) || QWidget::event(e);
}

void ImageViewerWidget::resizeEvent(QResizeEvent* e)
{
    computeZoomLevels();
    zoomToLevel(_zoomLevel, QPoint());
    QWidget::resizeEvent(e);
}
