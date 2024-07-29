//
// Created by benichn on 29/07/24.
//

#ifndef APP_H
#define APP_H
#include <QApplication>
#include "MainWindow.h"
#include "data/Database.h"

class GlobalEventFilter final : public QObject
{
    Q_OBJECT
signals:
    void modifiersChangeEvent(Qt::KeyboardModifiers value);

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;
};

class App final : public QApplication
{
    Database _database;
    GlobalEventFilter _eventFilter;
    MainWindow _mainWindow;

public:
    App(int& argc, char** argv);

    static App* instance() { return dynamic_cast<App*>(QApplication::instance()); }
    [[nodiscard]] Database* database() { return &_database; }
    [[nodiscard]] MainWindow* mainWindow() { return &_mainWindow; }
    [[nodiscard]] GlobalEventFilter* globalEventFilter() { return &_eventFilter; }
};


#endif //APP_H
