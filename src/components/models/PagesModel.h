//
// Created by benichn on 28/07/24.
//

#ifndef PAGESMODEL_H
#define PAGESMODEL_H

#include "../../imports/qtimports.h"
#include "../../data/book.h"

class PagesModel final : public QAbstractListModel {
    Q_OBJECT
public:
    explicit PagesModel(QObject *parent = nullptr);
    void setPages(const vector<Page>& pages);
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
};



#endif //PAGESMODEL_H
