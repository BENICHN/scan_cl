//
// Created by benichn on 29/07/24.
//

#include "app.h"

bool GlobalEventFilter::eventFilter(QObject* obj, QEvent* e)
{
    if (const auto type = e->type(); type == QEvent::KeyPress || type == QEvent::KeyRelease)
    {
        switch (const auto keyEvent = dynamic_cast<QKeyEvent*>(e); keyEvent->key())
        {
        case Qt::Key_Control:
        case Qt::Key_Shift:
        case Qt::Key_Alt:
        case Qt::Key_AltGr:
        case Qt::Key_Meta:
        case Qt::Key_Super_L:
        case Qt::Key_Super_R:
            emit modifiersChangeEvent(QGuiApplication::queryKeyboardModifiers());
            break;
        default: ;
        }
    }
    return QObject::eventFilter(obj, e);
}

App::App(int& argc, char** argv): QApplication(argc, argv)
{
    Q_INIT_RESOURCE(resources);
    installEventFilter(&_eventFilter);
    QImageReader::setAllocationLimit(512);
    std::srand(std::time(nullptr));
    if (stf::exists("settings.json"))
    {
        ifstream file("settings.json");
        _appSettings.loadFromJson(json::parse(file));
    }
}
