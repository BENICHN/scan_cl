//
// Created by benichn on 15/08/24.
//

#include "ScanOptionsModel.h"

#include <magic_enum.hpp>

#include "../app.h"
#include "../utils.h"

void ScanOptionsModel::setMode(PageColorMode mode)
{
    _mode = mode;
    resetJson();
}

void ScanOptionsModel::createJson() const
{
    // cout << "[" << magic_enum::enum_name(_mode).data() << ":uj| ";
    auto& settings = app().appSettings();
    if (!app().scanner().currentDeviceName().has_value())
    {
        StaticJsonModel::createJson();
        StaticJsonModel::createJsonPlaceholder();
        // cout << " |uj]\n";
        return;
    }
    auto& scanner = app().scanner();
    const auto jc = settings.getScanOptions(_mode);
    auto jcr = jc;
    if (_mode != PT_BLACK)
    {
        updateNewKeys(jcr, settings.getScanOptions(PT_BLACK));
    }
    scanner.setOptionValues(jcr);
    const auto sta = scanner.getOptionsValues();
    auto ja = sta ? sta.value() : json::object();
    if (_mode != PT_BLACK)
    {
        setJsonPlaceholder(ja);
        nullifyKeys(ja, jc);
    }
    else
    {
        StaticJsonModel::createJsonPlaceholder();
    }
    settings.setScanOptions(_mode, ja);
    setJson(ja);
    // cout << " |uj]\n";
}

bool ScanOptionsModel::beforeEditJsonProperty(const JsonStructure::path_t& path, const json& value) const
{
    // cout << "[" << magic_enum::enum_name(_mode).data() << ":bef| ";
    auto& scanner = app().scanner();
    const auto& optName = path.back();
    const auto& opts = scanner.currentOptions();
    const auto it = str::find(opts, optName, [](const auto& desc) { return desc.title; });
    if (it == opts.end()) return false;
    const auto sta = scanner.setOptionValueAt(it - opts.begin(), value.is_null() ? placeholder().at(path) : value);
    // ! info ignored
    // cout << " |bef]\n";
    return sta;
}

void ScanOptionsModel::afterEditJsonProperty(const JsonStructure::path_t& path, const json& value) const
{
    // cout << "[" << magic_enum::enum_name(_mode).data() << ":aft| ";
    app().appSettings().setScanOptions(_mode, jSON());
    // cout << " |aft]\n";
}

void ScanOptionsModel::createJsonPlaceholder() const
{
}

void ScanOptionsModel::createJsonDescriptor() const
{
    const auto& scanner = app().scanner();
    if (!scanner.currentDeviceName().has_value())
    {
        StaticJsonModel::createJsonDescriptor();
        return;
    }
    const auto& opts = scanner.currentOptions();
    json res;
    for (int i = 0; i < opts.size(); ++i)
    {
        if (i == 0) continue; // skip "number of options"
        const auto& opt = opts[i];
        json j{
            {"nullable", _mode != PT_BLACK},
            {"name", opt.title},
        };
        switch (opt.type)
        {
        case SANE_TYPE_BOOL:
            j["type"] = "bool";
            break;
        case SANE_TYPE_INT:
            j["type"] = "int";
            break;
        case SANE_TYPE_FIXED:
            j["type"] = "float";
            break;
        case SANE_TYPE_STRING:
            j["type"] = "string";
            break;
        default:
            continue;
        }
        json details{};
        switch (opt.constraint_type)
        {
        case SANE_CONSTRAINT_RANGE:
            {
                const auto& c = opt.constraintRange();
                if (opt.type == SANE_TYPE_FIXED)
                {
                    details["min"] = SANE_UNFIX(c.min);
                    details["max"] = SANE_UNFIX(c.max);
                    details["step"] = SANE_UNFIX(c.quant);
                }
                else
                {
                    details["min"] = c.min;
                    details["max"] = c.max;
                    details["step"] = c.quant;
                }
            }
            break;
        case SANE_CONSTRAINT_WORD_LIST:
            {
                details["choices"] = opt.constraintWordList() | stv::transform([&](const auto& w)
                {
                    return opt.type == SANE_TYPE_FIXED ? json(SANE_UNFIX(w)) : json(w);
                }) | str::to<json::array_t>();
            }
            break;
        case SANE_CONSTRAINT_STRING_LIST:
            {
                details["choices"] = opt.constraintStringList();
            }
            break;
        }
        switch (opt.unit)
        {
        case SANE_UNIT_PIXEL:
            details["unit"] = "px";
            break;
        case SANE_UNIT_BIT:
            details["unit"] = "b";
            break;
        case SANE_UNIT_MM:
            details["unit"] = "mm";
            break;
        case SANE_UNIT_DPI:
            details["unit"] = "dpi";
            break;
        case SANE_UNIT_PERCENT:
            details["unit"] = "%";
            break;
        case SANE_UNIT_MICROSECOND:
            details["unit"] = "us";
            break;
        }
        if (!details.empty()) j["details"] = details;
        j["settable"] = SANE_OPTION_IS_SETTABLE(opt.cap) == SANE_TRUE;
        j["active"] = SANE_OPTION_IS_ACTIVE(opt.cap) == SANE_TRUE;
        res[opt.title] = j;
    }
    setJsonDescriptor(res);
}

ScanOptionsModel::ScanOptionsModel(QObject* parent) : StaticJsonModel(parent)
{
    connect(&app().scanner(), &Scanner::currentDeviceChanged, [=]
    {
        resetJson();
    });
    connect(&app().appSettings(), &AppSettings::scanOptionsChanged, [=](const auto&, auto mode)
    {
        if (initialized() && _mode != PT_BLACK && mode == PT_BLACK)
        {
            updatePlaceholder();
        }
    });
}

void ScanOptionsModel::updatePlaceholder()
{
    // cout << "[" << magic_enum::enum_name(_mode).data() << ":up| ";
    auto& ph = placeholder();
    const auto& opts = app().appSettings().getScanOptions(PT_BLACK);
    for (const auto& kv : opts.items())
    {
        if (ph.contains(kv.key()))
        {
            ph[kv.key()] = kv.value(); // ! (si nouvelles entrees)
        }
    }
    endResetModel();
    // cout << " |up]\n";
}
