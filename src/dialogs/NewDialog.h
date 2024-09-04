//
// Created by benichn on 04/09/24.
//

#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include <QFileDialog>

class NewDialog final : public QFileDialog
{
    Q_OBJECT

protected:
    void accept() override;

public:
    explicit NewDialog(QWidget* parent = nullptr, const Qt::WindowFlags& f = Qt::WindowFlags());
};



#endif //NEWDIALOG_H
