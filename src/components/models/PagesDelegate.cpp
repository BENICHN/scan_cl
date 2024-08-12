//
// Created by benichn on 11/08/24.
//

#include "PagesDelegate.h"

#include <QListView>

#include "../../colors.h"
#include "../../utils.h"

PagesDelegate::PagesDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

void PagesDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->setRenderHint(QPainter::Antialiasing);
    const auto& r = option.rect;
    const auto tl = r.topLeft();
    if (option.state & QStyle::State_Selected)
    {
        painter->save();
        painter->setBrush(Color::br2);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(r, 4, 4);
        painter->restore();
    }
    const auto img = index.data(Qt::DecorationRole).value<QPixmap>();
    const auto imgR = fitIn(img.rect(), QRect{ 16, 16, r.width()-32, 128 }.translated(tl));
    const auto imgRR = roundRect(imgR, 4);
    painter->setClipPath(imgRR);
    painter->drawPixmap(imgR, img);
    painter->setClipping(false);
    painter->setPen({Color::br3, 2});
    painter->drawPath(imgRR);
    painter->setPen(Qt::black);
    painter->drawText(QRect{16, 152, r.width()-32, 16}.translated(tl), Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
    // QStyledItemDelegate::paint(painter, option, index);
}

QSize PagesDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const auto* lv = qobject_cast<const QListView*>(option.widget);
    return { lv->viewport()->width()-2*lv->spacing(), 180 };
}
