//
// Created by benichn on 12/07/24.
//

#include "MouseHandler.h"
#include "MouseAction.h"

bool MouseHandler::event(QEvent* e)
{
    _e = e;
    switch (e->type())
    {
    case QEvent::MouseButtonPress:
        {
            const auto mouseEvent = dynamic_cast<QMouseEvent*>(e);
            if (_mouseAction)
            {
                _mouseAction->cancel();
            }
            _mouseAction = createMouseAction(mouseEvent);
            break;
        }
    default: ;
    }
    if (_mouseAction)
    {
        switch (e->type())
        {
        case QEvent::MouseButtonPress:
            {
                const auto mouseEvent = dynamic_cast<QMouseEvent*>(e);
                _mouseAction->mousePressEvent(mouseEvent, this);
                break;
            }
        case QEvent::MouseButtonRelease:
            {
                const auto mouseEvent = dynamic_cast<QMouseEvent*>(e);
                if (mouseEvent->button() == _mouseAction->buttonDown())
                {
                    _mouseAction->mouseReleaseEvent(mouseEvent);
                }
                break;
            }
        // case QEvent::MouseButtonDblClick:
        // {
        //     QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(e);
        //     if (mouseEvent->button() == Qt::LeftButton)
        //     {
        //         qDebug() << "Left button double-clicked at" << mouseEvent->pos();
        //     }
        //     break;
        // }
        case QEvent::MouseMove:
            {
                const auto mouseEvent = dynamic_cast<QMouseEvent*>(e);
                _mouseAction->mouseMoveEvent(mouseEvent);
                break;
            }
        case QEvent::Wheel:
            {
                const auto ev = dynamic_cast<QWheelEvent*>(e);
                _mouseAction->wheelEvent(ev);
                break;
            }
        default:
            return false;
        }
        if (_mouseAction->isFinished())
        {
            _mouseAction = nullptr;
        }
    }
    if (!_mouseAction)
    {
        return callDefault();
    }
    return true;
}

bool MouseHandler::callDefault()
{
    if (_e == nullptr)
        return true;
    switch (_e->type())
    {
    case QEvent::MouseButtonPress:
        {
            const auto mouseEvent = dynamic_cast<QMouseEvent*>(_e);
            mousePressEvent(mouseEvent);
            break;
        }
    case QEvent::MouseButtonRelease:
        {
            const auto mouseEvent = dynamic_cast<QMouseEvent*>(_e);
            mouseReleaseEvent(mouseEvent);
            break;
        }
    case QEvent::MouseButtonDblClick:
        {
            const auto mouseEvent = dynamic_cast<QMouseEvent*>(_e);
            mouseDoubleClickEvent(mouseEvent);
            break;
        }
    case QEvent::MouseMove:
        {
            const auto mouseEvent = dynamic_cast<QMouseEvent*>(_e);
            mouseMoveEvent(mouseEvent);
            break;
        }
    case QEvent::Wheel:
        {
            const auto ev = dynamic_cast<QWheelEvent*>(_e);
            wheelEvent(ev);
            break;
        }
    default:
        return false;
    }
    _e = nullptr; // pour n'être appelé qu'une seule fois
    return true; // indicate that the event has been handled
}
