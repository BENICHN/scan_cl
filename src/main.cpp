#include <QMainWindow>

#include "app.h"

int main(int argc, char* argv[])
{
    App a(argc, argv);
    a.mainWindow().showMaximized();
    return QApplication::exec();
}
