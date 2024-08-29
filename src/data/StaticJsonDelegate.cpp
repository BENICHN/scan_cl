//
// Created by benichn on 15/08/24.
//

#include "StaticJsonDelegate.h"

#include <QCheckBox>

#include "StaticJsonEditor.h"
#include "StaticJsonModel.h"

StaticJsonDelegate::StaticJsonDelegate(QWidget* parent) : QStyledItemDelegate(parent)
{
}

void StaticJsonDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

QSize StaticJsonDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

json getPD(const QModelIndex& index)
{
    json res;
    const auto model = qobject_cast<const StaticJsonModel*>(index.model());
    const auto& path = static_cast<JsonStructure*>(index.internalPointer())->path;
    res["desc"] = model->descriptor()[path];
    const auto& pl = model->placeholder();
    if (pl.contains(path))
    {
        res["placeholderValue"] = pl[path];
    }
    return res;
}

json getVal(const QModelIndex& index)
{
    const auto model = qobject_cast<const StaticJsonModel*>(index.model());
    const auto& j = model->jSON();
    const auto& path = static_cast<JsonStructure*>(index.internalPointer())->path;
    return j[path];
}

QWidget* StaticJsonDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const
{
    const auto ed = new StaticJsonEditor(getPD(index), parent);
    // connect(ed, &StaticJsonEditor::editingFinished, this, &StaticJsonDelegate::commitAndCloseEditor);
    return ed;
}

void StaticJsonDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    qDebug() << "SED\n";
    qobject_cast<StaticJsonEditor*>(editor)->setupEditor(getVal(index));
    // QStyledItemDelegate::setEditorData(editor, index);
}

void StaticJsonDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    qDebug() << "SMD\n";
    const auto ed = qobject_cast<StaticJsonEditor*>(editor);
    qobject_cast<StaticJsonModel*>(model)->setJsonData(index, ed->value());
}

void StaticJsonDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                              const QModelIndex& index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

// void StaticJsonDelegate::commitAndCloseEditor()
// {
//     const auto editor = qobject_cast<StaticJsonEditor*>(sender());
//     emit commitData(editor);
//     emit closeEditor(editor);
// }
