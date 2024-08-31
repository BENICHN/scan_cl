//
// Created by benichn on 28/07/24.
//

#include "PagesModel.h"
#include "../../app.h"
#include "../../colors.h"

PagesModel::PagesModel(QObject* parent) : QAbstractListModel(parent)
{
    connect(&app().book(), &Book::pageListChanged, [=]
    {
        endResetModel();
    });
    connect(&app().book(), &Book::pageStatusChanged, [=](const int id)
    {
        int i = 0;
        while (i < _data.size())
        {
            const auto& src = _data[i];
            if (src.index() == PTY_PAGE && get<PTY_PAGE>(src) == id)
            {
                break;
            }
            ++i;
        }
        const auto idx = index(i, 0, {});
        emit dataChanged(idx, idx);
    });
    resetInternalData();
}

QVariant PagesModel::data(const QModelIndex& index, int role) const
{
    return {};
}

int PagesModel::rowCount(const QModelIndex& parent) const
{
    return _data.size();
}

QModelIndex PagesModel::index(int row, int column, const QModelIndex& parent) const
{
    const auto idx = QAbstractListModel::index(row, column, parent);
    return idx.isValid() ? createIndex(idx.row(), idx.column(), &_data[idx.row()]) : idx;
}

void PagesModel::resetInternalData()
{
    _data.clear();
    const auto& book = app().book();
    for (const auto id : book.ids())
    {
        _data.emplace_back(in_place_index<PTY_PAGE>, id);
    }
}
