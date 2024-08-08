//
// Created by benichn on 29/07/24.
//

#ifndef PREVIEWERWIDGET_H
#define PREVIEWERWIDGET_H

#include "PreviewerSelectionTypes.h"
#include "../widgets/PreviewerSelector.h"

QT_BEGIN_NAMESPACE

namespace Ui
{
    class PreviewerWidget;
}

QT_END_NAMESPACE

class PreviewerWidget final : public QWidget
{
    Q_OBJECT
    int _pageId = -1;

public:
    explicit PreviewerWidget(QWidget* parent = nullptr);
    ~PreviewerWidget() override;

    [[nodiscard]] int pageId() const { return _pageId; }

    void setPageId(int id);
    void validateChoice();
    [[nodiscard]] PreviewerSettings previewerSettings() const;

private:
    void updateImageAndRect(bool updateImage, bool updateRect);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    Ui::PreviewerWidget* ui;
};


#endif //PREVIEWERWIDGET_H
