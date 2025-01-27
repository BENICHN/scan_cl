//
// Created by benichn on 12/07/24.
//

#include "SelectionRectWidget.h"

#include "../app.h"

PickerElement PickerElement::fromStats(const int l, const int t, const int w, const int h, const int cx,
    const int cy, const bool selected)
{
    return {
        {cx, cy},
        {l - PICKER_RADIUS, t - PICKER_RADIUS, w + 2 * PICKER_RADIUS, h + 2 * PICKER_RADIUS},
        roundedRect(l, t, w, h, 10),
        selected
    };
}

vector<PickerElement> pickerElementsFromStats(const CCStats& st, const bool selected)
{
    vector<PickerElement> res(st.stats.rows-1);
    for (int i = 1; i < st.stats.rows; ++i)
    {
        const auto* row = reinterpret_cast<const int*>(st.stats.ptr(i));
        const auto* cRow = reinterpret_cast<const int*>(st.centroids.ptr(i));
        res[i-1] = PickerElement::fromStats(
            row[0], row[1], row[2], row[3], cRow[0], cRow[1], selected);
    }
    return res;
}

void SelectionRectWidget::SelectMouseAction::onDown()
{
    _sr->beginSelection(
        lastPos());
    _sr->updateNearest(_sr->nearest(lastPos()), Pressed);
}

void SelectionRectWidget::SelectMouseAction::onMove()
{
    _sr->updateSelection(
        lastPos());
    _sr->updateNearest(None);
}

void SelectionRectWidget::SelectMouseAction::onWheel(const QPoint& pixData)
{
    callDefault();
    onMove();
}

void SelectionRectWidget::SelectMouseAction::onUp()
{
    if (!hasMoved())
    {
        _sr->pickNearest();
    }
}

void SelectionRectWidget::SelectMouseAction::onFinish(bool canceled)
{
    if (hasMoved())
    {
        _sr->sendSelection();
    }
    _sr->updateNearest(None);
}

unique_ptr<MouseAction> SelectionRectWidget::MouseHandler::createMouseAction(QMouseEvent* e)
{
    switch (e->button())
    {
    case Qt::LeftButton:
        return make_unique<SelectMouseAction>(_sr);
    default:
        return nullptr;
    }
}

void SelectionRectWidget::MouseHandler::mouseMoveEvent(QMouseEvent* e)
{
    _sr->updateNearest(
        _sr->nearest(e->pos()),
        Hovered);
}

SelectionRectWidget::SelectionRectWidget(QWidget* parent) : QWidget(parent)
{
    updateSelectionMode();
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setMouseTracking(true);
    connect(&app().globalEventFilter(), &GlobalEventFilter::modifiersChangeEvent, this,
            &SelectionRectWidget::onModifiersChange);
}

void SelectionRectWidget::resetDisabled()
{
    _selectionType = SR_NONE;
    reset({});
}

void SelectionRectWidget::resetRect(const QSize& imageSize)
{
    resetRect(QImage(imageSize, QImage::Format_Mono));
}

void SelectionRectWidget::resetRect(const QImage& sel)
{
    _selectionType = SR_RECT;
    auto img = sel;
    img.fill(0);
    _selection = img;
    reset(sel.size());
}

void SelectionRectWidget::resetPicker(const QSize& imageSize, const vector<PickerElement>& elements)
{
    _selectionType = SR_PICKER;
    _selection = elements;
    reset(imageSize);
}

void SelectionRectWidget::beginSelection(const QPoint& value)
{
    if (_selectionType == SR_NONE) return;
    updateSelectionMode();
    if (_selectionType == SR_RECT && _selectionMode == Replace)
    {
        get<QImage>(_selection).fill(0);
        updateSelectionRegion();
    }
    _currentSelectionOrigin = value * _scaleInv;
}

void SelectionRectWidget::updateSelection(const QPoint& value)
{
    _currentSelection = QRect(_currentSelectionOrigin, value * _scaleInv) & QRect(
        0, 0, _imageSize.width(), _imageSize.height());
    updateSelectionRegion();
}

void SelectionRectWidget::sendSelection()
{
    if (_selectionType == SR_NONE) return;
    switch (_selectionType)
    {
    case SR_RECT:
        {
            auto& img = get<QImage>(_selection);
            uchar* sel = img.bits();
            const QRect cSel = _currentSelection;

            const int w = img.bytesPerLine();
            vector<uchar> maskLine(w);
            const int l = cSel.left();
            const int r = cSel.right();
            const div_t lqr = div(l, 8);
            const div_t rqr = div(r, 8);
            if (lqr.quot == rqr.quot)
            {
                maskLine[lqr.quot] = bitRange(lqr.rem, rqr.rem, true);
            }
            else
            {
                maskLine[lqr.quot] = bitRange(lqr.rem, 8, true);
                maskLine[rqr.quot] = bitRange(0, rqr.rem, true);
                const auto beg = maskLine.begin();
                std::fill(beg + lqr.quot + 1, beg + rqr.quot, 255);
            }

            for (int i = cSel.top(); i < cSel.bottom(); ++i)
            {
                if (isRemoving())
                {
                    for (int j = lqr.quot; j <= rqr.quot; ++j)
                    {
                        sel[i * w + j] &= ~maskLine[j];
                    }
                }
                else
                {
                    for (int j = lqr.quot; j <= rqr.quot; ++j)
                    {
                        sel[i * w + j] |= maskLine[j];
                    }
                }
            }
        }
        break;
    case SR_PICKER:
        for (PickerElement& element : get<vector<PickerElement>>(_selection))
        {
            if (element.region.intersects(_currentSelection))
            {
                switch (_selectionMode)
                {
                case Replace:
                    element.selected = !element.selected;
                    break;
                case Add:
                    element.selected = true;
                    break;
                case Substract:
                    element.selected = false;
                    break;
                }
            }
        }
        break;
    }
    _currentSelectionOrigin = {};
    _currentSelection = {};
    updateSelectionRegion();
}

void SelectionRectWidget::updateSelectionRegion()
{
    QRegion selectionRegion = _selectionType == SR_RECT
                                  ? QBitmap::fromImage(get<QImage>(_selection))
                                  : QRegion(0, 0, _imageSize.width(), _imageSize.height());
    if (_selectionType == SR_PICKER)
    {
        for (const PickerElement& element : get<vector<PickerElement>>(_selection))
        {
            bool draw = element.selected;
            if (element.region.intersects(_currentSelection))
            {
                switch (_selectionMode)
                {
                case Replace:
                    draw = !element.selected;
                    break;
                case Add:
                    draw = true;
                    break;
                case Substract:
                    draw = false;
                    break;
                }
            }
            if (draw)
            {
                selectionRegion -= element.region;
            }
        }
    }
    switch (_selectionType)
    {
    case SR_RECT:
        {
            const QRegion currentSelectionRegion = _currentSelection;
            _selectionRegion = isRemoving()
                                   ? selectionRegion + currentSelectionRegion
                                   : selectionRegion - currentSelectionRegion;
        }
        break;
    case SR_PICKER:
        _selectionRegion = selectionRegion;
        break;
    }
    update();
}

void SelectionRectWidget::updateSelectionMode()
{
    updateSelectionMode(QGuiApplication::queryKeyboardModifiers());
}

PickerElement* SelectionRectWidget::nearest(const QPoint& p)
{
    PickerElement* result = nullptr;
    if (_selectionType == SR_PICKER)
    {
        int minDist = 0;
        const QPoint scaledP = p * _scaleInv;
        for (PickerElement& element : get<vector<PickerElement>>(_selection))
        {
            const int dist = l22(element.centroid, scaledP);
            if (!result)
            {
                minDist = dist;
            }
            if (element.region.contains(scaledP) && dist <= minDist)
            {
                minDist = dist;
                result = &element;
            }
        }
    }
    return result;
}

void SelectionRectWidget::pickNearest()
{
    if (_nearest.element)
    {
        _nearest.element->selected = !_nearest.element->selected;
    }
    updateSelectionRegion();
}

void SelectionRectWidget::reset(const QSize& imageSize)
{
    _imageSize = imageSize;
    _currentSelectionOrigin = {};
    _currentSelection = {};
    _selectionRegion = {0, 0, _imageSize.width(), _imageSize.height()};
    _nearest = {};
    updateScale();
    updateSelectionRegion();
}

void SelectionRectWidget::updateScale()
{
    _scale = static_cast<float>(width()) / _imageSize.width();
    _scaleInv = 1.f / _scale;
}

void SelectionRectWidget::updateNearest(PickerElement* element)
{
    updateNearest(element, _nearest.state);
}

void SelectionRectWidget::updateNearest(const ButtonState state)
{
    updateNearest(_nearest.element, state);
}

void SelectionRectWidget::updateNearest(PickerElement* element, const ButtonState state)
{
    if (_nearest.element != element || _nearest.state != state)
    {
        _nearest = {element, state};
        update();
    }
}

void SelectionRectWidget::updateSelectionMode(const Qt::KeyboardModifiers modifiers)
{
    switch (modifiers)
    {
    case Qt::KeyboardModifier::ShiftModifier:
        _selectionMode = Add;
        break;
    case Qt::KeyboardModifier::ControlModifier:
        _selectionMode = Substract;
        break;
    default:
        _selectionMode = Replace;
        break;
    }
}

void SelectionRectWidget::resizeEvent(QResizeEvent* e)
{
    updateScale();
    QWidget::resizeEvent(e);
}

void SelectionRectWidget::paintEvent(QPaintEvent* e)
{
    if (_selectionType == SR_NONE) return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale(_scale, _scale);
    // draw region
    const int w = _imageSize.width();
    const int h = _imageSize.height();
    painter.setOpacity(.5);
    painter.setClipRegion(_selectionRegion);
    painter.fillRect(0, 0, w, h, Qt::black);
    painter.setClipRect(0, 0, w, h);
    painter.setOpacity(1);
    // draw elements
    const auto mods = QGuiApplication::queryKeyboardModifiers();
    if (_selectionType == SR_PICKER && !(mods & Qt::KeyboardModifier::AltModifier))
    {
        painter.setPen(QPen(Qt::blue, _scaleInv * 2));
        for (const PickerElement& element : get<vector<PickerElement>>(_selection))
        {
            if (!(_nearest.element == &element && _nearest.state != ButtonState::None))
            {
                painter.drawRoundedRect(element.regionBounds, PICKER_RADIUS, PICKER_RADIUS);
            }
        }
    }
    if (_nearest.element)
    {
        if (mods & Qt::KeyboardModifier::AltModifier)
        {
            painter.setPen({});
        }
        painter.setOpacity(.8);
        switch (_nearest.state)
        {
        case Hovered:
            painter.setBrush(Qt::lightGray);
            painter.drawRoundedRect(_nearest.element->regionBounds, PICKER_RADIUS, PICKER_RADIUS);
            break;
        case Pressed:
            painter.setBrush(Qt::darkGray);
            painter.drawRoundedRect(_nearest.element->regionBounds, PICKER_RADIUS, PICKER_RADIUS);
            break;
        default: ;
        }
        painter.setOpacity(1);
        painter.setBrush(Qt::transparent);
    }
    // draw rect
    painter.setPen(QPen(Qt::red, _scaleInv * 2));
    painter.drawRect(_currentSelection);
    QWidget::paintEvent(e);
}

bool SelectionRectWidget::event(QEvent* e)
{
    // const auto type = e->type();
    _mouseHandler.event(e) || QWidget::event(e);
    return false;
}

void SelectionRectWidget::onModifiersChange(const Qt::KeyboardModifiers value)
{
    updateSelectionMode(value);
    updateSelectionRegion();
}
