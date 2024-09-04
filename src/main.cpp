#include <QMainWindow>

#include "app.h"
#include <sane/sane.h>
#include <sane/saneopts.h>

int main(int argc, char* argv[])
{
    App a(argc, argv);
    if (argc>0)
    {
        const auto fn = argv[0];
        if (std::filesystem::exists(fn))
        {
            const auto p = path(fn);
            if (is_directory(p))
            {
                if (exists(p/"book.json"))
                {
                    a.book().loadFromRoot(p);
                }
            }
            else
            {
                if (p.filename() == "book.json")
                {
                    a.book().loadFromRoot(p.parent_path());
                }
            }
        }
    }
    a.mainWindow().showMaximized();
    int res = QApplication::exec();
    a.book().save();
    a.appSettings().save();
    return res;
}
