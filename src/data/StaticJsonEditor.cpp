//
// Created by benichn on 15/08/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_StaticJsonEditor.h" resolved

#include "StaticJsonEditor.h"

#include <QCheckBox>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QSpinBox>

#include "ui_StaticJsonEditor.h"
#include "../imports/stdimports.h"


StaticJsonEditor::StaticJsonEditor(const json& nvd, QWidget* parent) :
    QWidget(parent), ui(new Ui::StaticJsonEditor)
{
    ui->setupUi(this);
    const auto ly = qobject_cast<QHBoxLayout*>(layout());
    _desc = nvd["desc"];
    const auto& type = _desc.at("type");
    const auto details = _desc.contains("details") ? _desc["details"] : json{};
    // ! check null for value
    if (details.contains("choices"))
    {
        const auto ed = new QComboBox(this);
        ed->addItems(
            details["choices"] | stv::transform([](const auto& v) { return dumpValue(v); }) | str::to<QStringList>());
        ed->setCurrentText(dumpValue(nvd["value"]).c_str());
        ly->insertWidget(0, ed);
        return;
    }
    if (type == "bool")
    {
        const auto ed = new QCheckBox(this);
        ed->setChecked(nvd["value"]); // ! pas close apres
    }
    else if (type == "int")
    {
        const auto ed = new QSpinBox(this);
        if (details.contains("min")) ed->setMinimum(details["min"]);
        if (details.contains("max")) ed->setMaximum(details["max"]);
        if (details.contains("step")) ed->setSingleStep(details["step"]);
        if (details.contains("unit")) ed->setSuffix(details["unit"].get_ref<const string&>().c_str());
        ed->setValue(nvd["value"]);
        ly->insertWidget(0, ed);
    }
    else if (type == "float")
    {
        const auto ed = new QDoubleSpinBox(this);
        if (details.contains("min")) ed->setMinimum(details["min"]);
        if (details.contains("max")) ed->setMaximum(details["max"]);
        if (details.contains("step")) ed->setSingleStep(details["step"]);
        if (details.contains("unit")) ed->setSuffix(details["unit"].get_ref<const string&>().c_str());
        ed->setValue(nvd["value"]);
        ly->insertWidget(0, ed);
    }
    else if (type == "string")
    {
        const auto ed = new QPlainTextEdit(this);
        ed->setPlainText(nvd["value"].get_ref<const string&>().c_str());
        ly->insertWidget(0, ed);
    }
}

StaticJsonEditor::~StaticJsonEditor()
{
    delete ui;
}

// void StaticJsonEditor::keyPressEvent(QKeyEvent* event)
// {
//     switch (event->key())
//     {
//     case Qt::Key_Enter:
//         emit editingFinished(value());
//         break;
//     }
// }

json StaticJsonEditor::value() const
{
    const auto& type = _desc.at("type");
    // ! check null for value
    if (_desc.contains("details") && _desc["details"].contains("choices"))
    {
        const auto ed = qobject_cast<QComboBox*>(layout()->itemAt(0)->widget());
        auto v = ed->currentText().toStdString();
        if (type == "int")
        {
            return std::stoi(v);
        }
        if (type == "float")
        {
            return std::stod(v);
        }
        return v;
    }
    if (type == "bool")
    {
        const auto ed = qobject_cast<QCheckBox*>(layout()->itemAt(0)->widget());
        return ed->isChecked();
    }
    if (type == "int")
    {
        const auto ed = qobject_cast<QSpinBox*>(layout()->itemAt(0)->widget());
        return ed->value();
    }
    if (type == "float")
    {
        const auto ed = qobject_cast<QDoubleSpinBox*>(layout()->itemAt(0)->widget());
        return ed->value();
    }
    if (type == "string")
    {
        const auto ed = qobject_cast<QPlainTextEdit*>(layout()->itemAt(0)->widget());
        return ed->toPlainText().toStdString();
    }
    return nullptr;
}
