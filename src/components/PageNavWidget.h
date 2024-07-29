//
// Created by benichn on 28/07/24.
//

#ifndef PAGENAVWIDGET_H
#define PAGENAVWIDGET_H

#include <ui_PageNavWidget.h>

#include "../qtimports.h"


QT_BEGIN_NAMESPACE
namespace Ui { class PageNavWidget; }
QT_END_NAMESPACE

class PageNavWidget : public QWidget {
Q_OBJECT

public:
    explicit PageNavWidget(QWidget *parent = nullptr);
    ~PageNavWidget() override;

    [[nodiscard]] QListView* list() const { return ui->list; }

private:
    Ui::PageNavWidget *ui;
};


#endif //PAGENAVWIDGET_H
