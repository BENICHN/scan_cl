//
// Created by benichn on 12/07/24.
//

#include "MouseAction.h"
#include "MouseHandler.h"
#include "../utils.h"

void MouseAction::updateData(const QPoint& pos)
{
    _lastDiff = pos - _lastPos;
    _lastPos = pos;
}

void MouseAction::mousePressEvent(const QMouseEvent* e, MouseHandler* handler)
{
    _handler = handler;
    _buttonDown = e->button();
    _downPos = e->pos();
    _lastPos = _downPos;
    onDown();
}

void MouseAction::mouseReleaseEvent(const QMouseEvent* e)
{
    updateData(e->pos());
    onUp();
    finish(false);
}

void MouseAction::mouseMoveEvent(const QMouseEvent* e)
{
    updateData(e->pos());
    if (!hasMoved() && l22(downPos(), lastPos()) > MINIMAL_MOVE_DISTANCE_SQ)
    {
        _hasMoved = true;
    }
    if (hasMoved())
    {
        onMove();
    }
}

void MouseAction::wheelEvent(const QWheelEvent* e)
{
    onWheel(e->pixelDelta());
}

void MouseAction::callDefault() const
{
    _handler->callDefault();
}

void MouseAction::finish(const bool canceled)
{
    onFinish(canceled);
    _isFinished = true;
}
