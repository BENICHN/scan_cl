//
// Created by benichn on 04/09/24.
//

#include "../imports/stdimports.h"
#include "../app.h"
#include "OpenDialog.h"

void OpenDialog::accept()
{
    const auto& dn = path(selectedFiles().first().toStdString());
    if (exists(dn / "book.json"))
    {
        app().book().loadFromRoot(dn);
        QFileDialog::accept();
    }
}

OpenDialog::OpenDialog(QWidget* parent, const Qt::WindowFlags& f): QFileDialog(parent, f)
{
    setFileMode(Directory);
    setNameFilter("book.json");
    setWindowTitle("Sélectionnez un dossier contenant un fichier book.json");
}
