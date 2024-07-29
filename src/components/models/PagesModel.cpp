//
// Created by benichn on 28/07/24.
//

#include "PagesModel.h"
#include "../../app.h"

PagesModel::PagesModel(QObject* parent) : QAbstractListModel(parent)
{
}

int PagesModel::rowCount(const QModelIndex& parent) const
{
    return App::instance()->database()->book()->pages.size();
}

QVariant PagesModel::data(const QModelIndex& index, int role) const
{
    switch (role)
    {
    case Qt::DisplayRole:
        return index.row();
    case Qt::DecorationRole:
         return QPixmap((App::instance()->database()->book()->sourcesDir + "/" + App::instance()->database()->book()->pages[index.row()].source).c_str());
    }
    return {};
}
