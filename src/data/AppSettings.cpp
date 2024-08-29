//
// Created by benichn on 28/08/24.
//

#include "AppSettings.h"

#include <magic_enum.hpp>

#include "../utils.h"

AppSettings::AppSettings(QObject* parent): QObject(parent)
{
}

void AppSettings::setScanDevName(const optional<string>& scanDevName)
{
    _scanDevName = scanDevName;
    save();
}

void AppSettings::save() const
{
    ofstream file("settings.json");
    file << json(*this).dump(2);
}

json AppSettings::getScanOptions(const PageColorMode mode) const
{
    if (!_scanDevName.has_value()) return json::object();
    return getScanOptions(_scanDevName.value(), mode);
}

json AppSettings::getScanOptions(const string& devName, const PageColorMode mode) const
{
    const auto it = _scanOptions.find(devName);
    if (it != _scanOptions.end())
    {
        return it.value().at(magic_enum::enum_name(mode)).at("options");
    }
    return json::object();
}

json AppSettings::getRealScanOptions(const PageColorMode mode) const
{
    if (!_scanDevName.has_value()) return json::object();
    return getRealScanOptions(_scanDevName.value(), mode);
}

json AppSettings::getRealScanOptions(const string& devName, const PageColorMode mode) const
{
    auto opts = getScanOptions(devName, mode);
    if (mode != PT_BLACK)
    {
        const auto& bw = getScanOptions(devName, PT_BLACK);
        updateNewKeys(opts, bw);
    }
    return opts;
}

void AppSettings::setScanOptions(const PageColorMode mode, const json& options)
{
    if (!_scanDevName.has_value()) return;
    setScanOptions(_scanDevName.value(), mode, options);
}

void AppSettings::setScanOptions(const string& devName, PageColorMode mode, const json& options)
{
    const auto it = _scanOptions.find(devName);
    if (it == _scanOptions.end())
    {
        _scanOptions[devName] =
        {
            {"PT_BLACK", {{"options", {}}}},
            {"PT_GRAY", {{"enabled", false}, {"options", {}}}},
            {"PT_COLOR", {{"enabled", true}, {"options", {}}}}
        };
    }
    _scanOptions[devName][magic_enum::enum_name(mode)]["options"] = options;
    emit scanOptionsChanged(devName, mode);
    save();
}

void to_json(json& j, const AppSettings& astgs)
{
    if (astgs._scanDevName.has_value()) j["scanDevName"] = astgs._scanDevName;
    j["scanOptions"] = astgs._scanOptions;
}

void AppSettings::loadFromJson(const json& j)
{
    _scanDevName = j.contains("scanDevName") ? optional(j.at("scanDevName")) : nullopt;
    _scanOptions = j.at("scanOptions");
    // ! signals
}
