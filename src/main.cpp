#include <QMainWindow>

#include "app.h"
#include <sane/sane.h>
#include <sane/saneopts.h>

int main(int argc, char* argv[])
{
    App a(argc, argv);
    a.book().loadFromRoot("/home/benichn/prog/cpp/scan/testBook");
    a.mainWindow().showMaximized();
    int res = QApplication::exec();
    a.book().save();
    a.appSettings().save();
    return res;
}
