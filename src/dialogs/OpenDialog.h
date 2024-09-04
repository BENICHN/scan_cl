//
// Created by benichn on 04/09/24.
//

#ifndef OPENDIALOG_H
#define OPENDIALOG_H
#include <QFileDialog>


class OpenDialog final : public QFileDialog
{
    Q_OBJECT

protected:
    void accept() override;

public:
    explicit OpenDialog(QWidget* parent = nullptr, const Qt::WindowFlags& f = Qt::WindowFlags());
};


#endif //OPENDIALOG_H
