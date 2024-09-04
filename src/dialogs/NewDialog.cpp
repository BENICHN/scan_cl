//
// Created by benichn on 04/09/24.
//

#include "NewDialog.h"
#include "../imports/stdimports.h"
#include "../app.h"

void NewDialog::accept()
{
    const auto& dn = path(selectedFiles().first().toStdString());
    if (is_empty(dn))
    {
        ofstream f(dn / "book.json");
        auto b = Book::newBook(dn);
        f << json(b).dump(2);
        f.close();
        app().book().loadFromRoot(dn);
        QFileDialog::accept();
    }
}

NewDialog::NewDialog(QWidget* parent, const Qt::WindowFlags& f): QFileDialog(parent, f)
{
    setFileMode(Directory);
    setWindowTitle("Sélectionnez un dossier vide");
}
