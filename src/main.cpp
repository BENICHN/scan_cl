#include <QMainWindow>

#include "app.h"
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    App a(argc, argv);
    a.mainWindow()->showMaximized();
    return QApplication::exec();
}
