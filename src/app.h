//
// Created by benichn on 29/07/24.
//

#ifndef APP_H
#define APP_H
#include <QApplication>

#include "MainWindow.h"
#include "Scanner.h"
#include "works.h"
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
    Book _book;
    Works _works;
    Scanner _scanner;
    GlobalEventFilter _eventFilter;
    MainWindow _mainWindow;

public:
    App(int &argc, char** argv);

    static App* instance() { return dynamic_cast<App*>(QApplication::instance()); }
    [[nodiscard]] Book& book() { return _book; }
    [[nodiscard]] MainWindow& mainWindow() { return _mainWindow; }
    [[nodiscard]] GlobalEventFilter& globalEventFilter() { return _eventFilter; }
    [[nodiscard]] Works& works() { return _works; }
    [[nodiscard]] Scanner& scanner() { return _scanner; }
};

inline App& app() { return *App::instance(); }

#endif //APP_H
