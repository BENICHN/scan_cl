//
// Created by benichn on 29/07/24.
//

#ifndef PREVIEWERWIDGET_H
#define PREVIEWERWIDGET_H

#include <QWidget>


QT_BEGIN_NAMESPACE

namespace Ui
{
    class PreviewerWidget;
}

QT_END_NAMESPACE

enum PreviewerMode
{
    SRC,
    ASK,
    RES
};

class PreviewerWidget final : public QWidget
{
    Q_OBJECT
    int _pageId = -1;
    PreviewerMode _mode = RES;

public:
    explicit PreviewerWidget(QWidget* parent = nullptr);
    ~PreviewerWidget() override;

    [[nodiscard]] int pageId() const { return _pageId; }
    [[nodiscard]] int mode() const { return _mode; }
    void setPageId(int id);

private:
    void updateImage() const;
    void updateMode();

    Ui::PreviewerWidget* ui;
};


#endif //PREVIEWERWIDGET_H
