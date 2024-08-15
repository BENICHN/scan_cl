//
// Created by benichn on 15/08/24.
//

#ifndef SCANOPTIONSDELEGATE_H
#define SCANOPTIONSDELEGATE_H
#include <QStyledItemDelegate>


class StaticJsonDelegate final : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit StaticJsonDelegate(QWidget* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;

private slots:
    void commitAndCloseEditor();
};


#endif //SCANOPTIONSDELEGATE_H
