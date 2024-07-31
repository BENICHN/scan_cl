//
// Created by benichn on 29/07/24.
//

#ifndef APP_H
#define APP_H
#include <QApplication>
#include "MainWindow.h"
#include "works.h"

class GlobalEventFilter final : public QObject
{
    Q_OBJECT
signals:
    void modifiersChangeEvent(Qt::KeyboardModifiers value);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;
};

class App final : public QApplication
{
    Q_OBJECT
    GlobalEventFilter _eventFilter;
    MainWindow _mainWindow;
    Works _works;
    Book _book = {
        "/home/benichn/prog/cpp/scan/testBook",
        "Test",
        {
            true, QSize(3000, 5000),
            60, 250, 10, 50, 234, 20, 240, 0.5, 15000, QSize(110, 12), 4000
        },
        {
            Page{rand(), PT_GRAY, "test.png", 1, {}, {}, PST_READY},
            Page{rand(), PT_GRAY, "test.png", 1, {}, {}, PST_READY},
            Page{rand(), PT_GRAY, "test.png", 1, {}, {}, PST_READY},
            Page{rand(), PT_GRAY, "test.png", 1, {}, {}, PST_READY},
            Page{rand(), PT_GRAY, "test.png", 1, {}, {}, PST_READY},
            Page{rand(), PT_GRAY, "test.png", 1, {}, {}, PST_READY},
            Page{rand(), PT_GRAY, "test.png", 1, {}, {}, PST_READY},
            Page{rand(), PT_GRAY, "test.png", 1, {}, {}, PST_READY}
        }
    };

public:
    App(int& argc, char** argv);

    static App* instance() { return dynamic_cast<App*>(QApplication::instance()); }
    [[nodiscard]] Book* book() { return &_book; }
    [[nodiscard]] MainWindow* mainWindow() { return &_mainWindow; }
    [[nodiscard]] GlobalEventFilter* globalEventFilter() { return &_eventFilter; }
    [[nodiscard]] Works* works() { return &_works; }

signals:
    void pageStatusChanged(int pageId);
};

inline App* app() { return App::instance(); }

#endif //APP_H
