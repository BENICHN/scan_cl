//
// Created by benichn on 04/09/24.
//

#ifndef BOOKEDITOR_H
#define BOOKEDITOR_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class BookEditor; }
QT_END_NAMESPACE

class BookEditor : public QWidget {
Q_OBJECT

public:
    explicit BookEditor(QWidget *parent = nullptr);
    ~BookEditor() override;

private:
    Ui::BookEditor *ui;
};


#endif //BOOKEDITOR_H
