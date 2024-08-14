//
// Created by benichn on 11/08/24.
//

#ifndef PAGESDELEGATE_H
#define PAGESDELEGATE_H

#include "../../imports/qtimports.h"

class PagesDelegate final : public QStyledItemDelegate {
    Q_OBJECT
public:
    PagesDelegate(QObject* parent);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};



#endif //PAGESDELEGATE_H
