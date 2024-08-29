//
// Created by benichn on 12/07/24.
//

#include "ImageViewerWidget.h"

#include <QVBoxLayout>

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

bool ImageViewerWidget::isCurrentFt(const string& filename, const stf::file_time_type& time)
{
    return _ft.has_value() && _ft->filename == filename && _ft->lastWriteTime == time;
}

bool ImageViewerWidget::isCurrentFt(const FilenameWithTimestamp& ft)
{
    return isCurrentFt(ft.filename, ft.lastWriteTime);
}

ImageViewerWidget::ImageViewerWidget(QWidget* parent) : QWidget(parent),
                                                        _imageLabel(new QLabel(this)),
                                                        _selectionRect(new SelectionRectWidget(this))
{
    _imageLabel->setScaledContents(true);
    auto* l = new QGridLayout(this);
    _loadingLabel = new QLabel(this);
    auto* mov = new QMovie(":/pics/loader.gif", {}, this);
    mov->start();
    _loadingLabel->setMovie(mov);
    _loadingLabel->setMaximumSize(200,200);
    _loadingLabel->hide();
    l->addWidget(_loadingLabel);
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

Task<> ImageViewerWidget::setPixmap(const string& filename)
{
    if (!stf::exists(filename))
    {
        setPixmap(QPixmap());
        co_return;
    }
    const auto lwt = stf::last_write_time(filename);
    if (isCurrentFt(filename, lwt)) co_return;
    _ft = {
        filename,
        lwt
    };
     co_await setPixmapInternal([=]{ return QPixmap(filename.c_str()); });
}

void ImageViewerWidget::setPixmap(const QPixmap& pixmap)
{
    _ft = nullopt;
    setPixmapInternal(pixmap);
}

void ImageViewerWidget::setSRDisabled()
{
    _srSettings = {
        SR_NONE,
        nullopt
    };
    if (!isLoading()) setSR();
}

void ImageViewerWidget::setSRRect()
{
    _srSettings = {
        SR_RECT,
        nullopt
    };
    if (!isLoading()) setSR();
}

void ImageViewerWidget::setSRRect(const QImage& sel)
{
    _srSettings = {
        SR_RECT,
        sel
    };
    if (!isLoading()) setSR();
}

void ImageViewerWidget::setSRRect(const optional<SelectionInfo>& sel)
{
    _srSettings = {
        SR_RECT,
        sel
    };
}

void ImageViewerWidget::setSRPicker(const vector<PickerElement>& elements)
{
    _srSettings = {
        SR_PICKER,
        elements
    };
    if (!isLoading()) setSR();
}

bool ImageViewerWidget::isLoading() const
{
    return _loadingLabel->isVisible();
}

void ImageViewerWidget::setSR()
{
    switch (_srSettings.type)
    {
    case SR_NONE:
        _selectionRect->resetDisabled();
        break;
    case SR_RECT:
        if (_srSettings.selection.has_value()) _selectionRect->resetRect(get<QImage>(_srSettings.selection.value()));
        else _selectionRect->resetRect(_pixmap.size());
        break;
    case SR_PICKER:
        _selectionRect->resetPicker(_pixmap.size(), get<vector<PickerElement>>(_srSettings.selection.value()));
        break;
    }
}

void ImageViewerWidget::setPixmapInternal(const QPixmap& pixmap)
{
    if (_imageLoadingToken) _imageLoadingToken->cancel();
    _loadingLabel->hide();
    _pixmap = pixmap;
    _imageLabel->setPixmap(_pixmap);
    setSR();
    computeZoomLevels();
    zoomToLevel(_minZoomLevel, QPointF());
}

void ImageViewerWidget::updateSelectionRect() const
{
    _selectionRect->setGeometry(_imageLabel->geometry());
}

void ImageViewerWidget::computeZoomLevels()
{
    computeZoomLevels(_pixmap.size());
}
void ImageViewerWidget::computeZoomLevels(const QSize& imgSize)
{
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