//
// Created by benichn on 02/08/24.
//

#ifndef PREVIEWERSELECTOR_H
#define PREVIEWERSELECTOR_H

#include "PreviewerSelectorTypes.h"
#include "../data/book.h"


QT_BEGIN_NAMESPACE

namespace Ui
{
    class PreviewerSelector;
}

QT_END_NAMESPACE

class PreviewerSelector : public QWidget
{
    Q_OBJECT
    bool _hasColor = false;
    bool _resMixed = false;
    PreviewerSelection _selection = {
        PWM_ASK,
        PWC_MIX
    };

protected:
    void paintEvent(QPaintEvent* event) override;

public:
    [[nodiscard]] bool hasColor() const { return _hasColor; }
    [[nodiscard]] bool resMixed() const { return _resMixed; }
    [[nodiscard]] PreviewerSelection selection() const { return _selection; }

    void setHasColor(bool value);
    void setResMixed(bool value);

    explicit PreviewerSelector(QWidget* parent = nullptr);
    ~PreviewerSelector() override;

    QPoint position();
    void setPosition(const QPoint& position);

private:
    Ui::PreviewerSelector* ui;
    void updateButtons();
    void updateSelection();

signals:
    void selectionChanged(PreviewerSelection value);
};


#endif //PREVIEWERSELECTOR_H
