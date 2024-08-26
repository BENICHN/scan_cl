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
    bool _isNull = false;
    QWidget* _ed = nullptr;

public:
    explicit StaticJsonEditor(const json& desc, QWidget* parent = nullptr);
    ~StaticJsonEditor() override;

// protected:
//     void keyPressEvent(QKeyEvent* event) override;
//
// public:
    [[nodiscard]] json value() const;
    void setupEditor(const json& value);

private:
    Ui::StaticJsonEditor* ui;

private slots:
    void setNull(bool null);
    void setDefault();

// signals:
    // void editingFinished(const nlohmann::json& value);
};


#endif //STATICJSONEDITOR_H
