//
// Created by benichn on 15/08/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_StaticJsonEditor.h" resolved

#include "StaticJsonEditor.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QSpinBox>

#include "ui_StaticJsonEditor.h"
#include "../imports/stdimports.h"


StaticJsonEditor::StaticJsonEditor(const json& desc, QWidget* parent) :
    QWidget(parent), ui(new Ui::StaticJsonEditor)
{
    ui->setupUi(this);
    _desc = desc;

    if (_desc.at("nullable"))
    {
        connect(ui->nullBtn, &QAbstractButton::toggled, this, &StaticJsonEditor::setNull);
    }
    else
    {
        ui->nullBtn->setHidden(true);
    }

    if (_desc.contains("defaultValue"))
    {
        connect(ui->defaultBtn, &QAbstractButton::clicked, this, &StaticJsonEditor::setDefault);
    }
    else
    {
        ui->defaultBtn->setHidden(true);
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
    if (_isNull) return nullptr;
    if (_desc.contains("details") && _desc["details"].contains("choices"))
    {
        const auto ed = qobject_cast<QComboBox*>(_ed);
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
        const auto ed = qobject_cast<QCheckBox*>(_ed);
        return ed->isChecked();
    }
    if (type == "int")
    {
        const auto ed = qobject_cast<QSpinBox*>(_ed);
        return ed->value();
    }
    if (type == "float")
    {
        const auto ed = qobject_cast<QDoubleSpinBox*>(_ed);
        return ed->value();
    }
    if (type == "string")
    {
        const auto ed = qobject_cast<QPlainTextEdit*>(_ed);
        return ed->toPlainText().toStdString();
    }
    return nullptr;
}

void StaticJsonEditor::setupEditor(const json& value)
{
    const auto ly = qobject_cast<QHBoxLayout*>(layout());
    ly->removeWidget(_ed);
    delete _ed; // ! utile ?
    const bool noval = value.is_object();
    _isNull = value.is_null();
    if (_isNull)
    {
        const auto ed = new QLabel(this);
        ed->setText("null");
        ed->setStyleSheet("*{ background: white; }");
        _ed = ed;
    }
    else
    {
        const auto& type = _desc.at("type");
        const auto details = _desc.contains("details") ? _desc["details"] : json{};
        if (details.contains("choices"))
        {
            const auto ed = new QComboBox(this);
            const auto& choices = details["choices"].get_ref<const json::array_t&>();
            ed->addItems(
                choices | stv::transform([&](const auto& v)
                {
                    return
                        details.contains("unit")
                            ? dumpValue(v) + details["unit"].get_ref<const json::string_t&>()
                            : dumpValue(v);
                }) | str::to<QStringList>());
            if (!noval)
            {
                auto it = str::find(choices, value);
                // qDebug() << _nvd.dump(2).c_str();
                if (it != choices.end())
                {
                    ed->setCurrentIndex(it - choices.begin());
                }
            }
            _ed = ed;
        }
        else if (type == "bool")
        {
            const auto ed = new QCheckBox(this);
            if (!noval)
            {
                ed->setChecked(value); // ! pas close apres
            }
            _ed = ed;
        }
        else if (type == "int")
        {
            const auto ed = new QSpinBox(this);
            ed->setMinimum(details.contains("min") ? details["min"].get<int>() : INT_MIN);
            ed->setMaximum(details.contains("max") ? details["max"].get<int>() : INT_MAX);
            if (details.contains("step")) ed->setSingleStep(details["step"]);
            if (details.contains("unit")) ed->setSuffix(details["unit"].get_ref<const json::string_t&>().c_str());
            if (!noval)
            {
                ed->setValue(value);
            }
            _ed = ed;
        }
        else if (type == "float")
        {
            const auto ed = new QDoubleSpinBox(this);
            ed->setMinimum(details.contains("min") ? details["min"].get<double>() : -INFINITY);
            ed->setMaximum(details.contains("max") ? details["max"].get<double>() : INFINITY);
            if (details.contains("step")) ed->setSingleStep(details["step"]);
            if (details.contains("unit")) ed->setSuffix(details["unit"].get_ref<const json::string_t&>().c_str());
            if (!noval)
            {
                ed->setValue(value);
            }
            _ed = ed;
        }
        else if (type == "string")
        {
            const auto ed = new QPlainTextEdit(this);
            if (!noval)
            {
                ed->setPlainText(value.get_ref<const json::string_t&>().c_str());
            }
            _ed = ed;
        }
    }
    ui->nullBtn->setChecked(_isNull);
    ly->insertWidget(0, _ed);
}

void StaticJsonEditor::setNull(const bool null)
{
    if (null)
    {
        setupEditor(nullptr);
    }
    else
    {
        setupEditor(_desc.contains("defaultValue") ? _desc["defaultValue"] : json::object());
    }
}

void StaticJsonEditor::setDefault()
{
    setupEditor(_desc.at("nullable"));
}
