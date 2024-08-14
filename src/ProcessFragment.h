//
// Created by benichn on 28/07/24.
//

#ifndef PROCESSFRAGMENT_H
#define PROCESSFRAGMENT_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class ProcessFragment; }
QT_END_NAMESPACE

class ProcessFragment final : public QWidget {
Q_OBJECT

public:
    explicit ProcessFragment(QWidget *parent = nullptr);
    ~ProcessFragment() override;

private:
    Ui::ProcessFragment *ui;
    [[nodiscard]] int uniqueSelectedId() const;
};


#endif //PROCESSFRAGMENT_H
