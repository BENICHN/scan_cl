//
// Created by benichn on 28/07/24.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "data/book.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow final : public QWidget {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    int uniqueSelectedId() const;

public slots:
    void pageNavSelectionChanged();
};


#endif //MAINWINDOW_H
