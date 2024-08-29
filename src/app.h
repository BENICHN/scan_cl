//
// Created by benichn on 29/07/24.
//

#ifndef APP_H
#define APP_H
#include <QApplication>

#include "MainWindow.h"
#include "Scanner.h"
#include "works.h"
#include "data/AppSettings.h"
#include "data/book.h"

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
    AppSettings _appSettings;
    Book _book;
    Works _works;
    Scanner _scanner;
    GlobalEventFilter _eventFilter;
    MainWindow _mainWindow;

public:
    App(int &argc, char** argv);

    static auto instance() { return dynamic_cast<App*>(QApplication::instance()); }
    [[nodiscard]] auto& appSettings() { return _appSettings; }
    [[nodiscard]] auto& book() { return _book; }
    [[nodiscard]] auto& mainWindow() { return _mainWindow; }
    [[nodiscard]] auto& globalEventFilter() { return _eventFilter; }
    [[nodiscard]] auto& works() { return _works; }
    [[nodiscard]] auto& scanner() { return _scanner; }
};

inline auto& app() { return *App::instance(); }

#endif //APP_H
