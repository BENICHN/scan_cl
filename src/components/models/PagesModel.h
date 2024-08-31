//
// Created by benichn on 28/07/24.
//

#ifndef PAGESMODEL_H
#define PAGESMODEL_H

#include "../../imports/qtimports.h"
#include "../../data/book.h"
#include "../props/PropsEditorTypes.h"

class PagesModel final : public QAbstractListModel
{
    Q_OBJECT

    vector<PropsSource> _data;

public:
    explicit PagesModel(QObject* parent = nullptr);
    void setPages(const vector<Page>& pages);
    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;

protected slots:
    void resetInternalData() override;
};


#endif //PAGESMODEL_H
