//
// Created by benichn on 11/08/24.
//

#include "PagesDelegate.h"

#include <QListView>

#include "../../app.h"
#include "../../colors.h"
#include "../../utils.h"
#include "../props/PropsEditorTypes.h"

PagesDelegate::PagesDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

void PagesDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const auto& data = *static_cast<PropsSource*>(index.internalPointer());
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
    switch (data.index())
    {
    case PTY_PAGE:
        {
            const auto id = get<PTY_PAGE>(data);
            const auto& book = app().book();
            const auto& page = book.page(id);

            const auto img = QPixmap(book.getPageSourceThumbnail(id).c_str());
            const auto imgR = fitIn(img.rect(), QRect{16, 16, r.width() - 32, 128}.translated(tl));
            const auto imgRR = roundRect(imgR, 4);
            painter->setClipPath(imgRR);
            painter->drawPixmap(imgR, img);
            painter->setClipping(false);
            painter->setPen({Color::br3, 2});
            painter->drawPath(imgRR);
            painter->setPen(Qt::black);
            const int i = index.row() + 1;
            const int lim = book.romanLimit();
            const auto text = i <= lim ? intToRoman(i) : to_string(i - lim);
            painter->drawText(QRect{16, 152, r.width() - 32, 16}.translated(tl), Qt::AlignCenter,
                              text.c_str());
            {
                QColor c;
                switch (page.status())
                {
                case PST_READY:
                    c = Qt::transparent;
                    break;
                case PST_WORKING:
                    c = Color::orange;
                    break;
                case PST_COMPLETED:
                    c = Color::emerald;
                    break;
                case PST_WAITING:
                    c = Color::sunFlower;
                    break;
                case PST_ERROR:
                    c = Color::alizarin;
                    break;
                }
                painter->save();
                painter->setBrush(c);
                painter->setPen(Qt::NoPen);
                painter->drawEllipse(QRect{imgR.left(), 156, 8, 8}.translated(tl));
                painter->restore();
            }
        }
        break;
    }
}

QSize PagesDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const auto* lv = qobject_cast<const QListView*>(option.widget);
    return {lv->viewport()->width() - 2 * lv->spacing(), 180};
}
