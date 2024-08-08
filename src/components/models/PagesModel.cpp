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
    return app().book().pages().size();
}

QVariant PagesModel::data(const QModelIndex& index, int role) const
{
    switch (role)
    {
    case Qt::DisplayRole:
        return index.row();
    case Qt::DecorationRole:
         return QPixmap(app().book().getPageSourceThumbnail(app().book().ids()[index.row()]).c_str());
    }
    return {};
}
