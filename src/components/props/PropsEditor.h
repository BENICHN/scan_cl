//
// Created by benichn on 13/08/24.
//

#ifndef PROPSEDITOR_H
#define PROPSEDITOR_H

#include <QWidget>
#include "PropsEditorTypes.h"

QT_BEGIN_NAMESPACE

namespace Ui
{
    class PropsEditor;
}

QT_END_NAMESPACE

class PropsEditor : public QWidget
{
    Q_OBJECT
    PropsSource _source;

public:
    [[nodiscard]] PropsSource source() const { return _source; }
    void setSource(const PropsSource& source);

    explicit PropsEditor(QWidget* parent = nullptr);
    ~PropsEditor() override;

private:
    void updateSource();
    Ui::PropsEditor* ui;
};


#endif //PROPSEDITOR_H
