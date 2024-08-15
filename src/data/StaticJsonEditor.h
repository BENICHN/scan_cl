//
// Created by benichn on 15/08/24.
//

#ifndef STATICJSONEDITOR_H
#define STATICJSONEDITOR_H

#include <QWidget>
#include "../imports/jsonimports.h"

QT_BEGIN_NAMESPACE

namespace Ui
{
    class StaticJsonEditor;
}

QT_END_NAMESPACE

class StaticJsonEditor : public QWidget
{
    Q_OBJECT
    json _desc;

public:
    explicit StaticJsonEditor(const json& nvd, QWidget* parent = nullptr);
    ~StaticJsonEditor() override;

// protected:
//     void keyPressEvent(QKeyEvent* event) override;
//
// public:
    [[nodiscard]] json value() const;

private:
    Ui::StaticJsonEditor* ui;

// signals:
    // void editingFinished(const nlohmann::json& value);
};


#endif //STATICJSONEDITOR_H
