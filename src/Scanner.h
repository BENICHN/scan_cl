//
// Created by benichn on 13/08/24.
//

#ifndef SCANNER_H
#define SCANNER_H

#include "imports/qtimports.h"
#include "imports/qcoroimports.h"
#include "imports/stdimports.h"
#include "imports/jsonimports.h"
#include <sane/sane.h>
#include <sane/saneopts.h>

template <typename T=void>
class SANEOpt
{
    using data_type = variant<T, SANE_Status>;
    data_type _data;

    SANEOpt() = default;

public:
    operator bool() const
    {
        return isOk();
    }

    operator SANE_Status() const
    {
        return status();
    }

    template <class Self>
    auto&& value(this Self&& self)
    {
        if (!self.isOk()) throw runtime_error("attempting to access value of a non ok SANEOpt");
        return get<0>(std::forward<Self>(self)._data);
    }

    [[nodiscard]] SANE_Status status() const
    {
        return isOk() ? SANE_STATUS_GOOD : get<1>(_data);
    }

    [[nodiscard]] bool isOk() const
    {
        return _data.index() == 0;
    }

    SANEOpt(const T& value)
    {
        _data = data_type(in_place_index<0>, value);
    }

    SANEOpt(const SANE_Status status)
    {
        _data = data_type(in_place_index<1>, status);
    }

    SANEOpt(const T& value, const SANE_Status status)
    {
        if (status == SANE_STATUS_GOOD)
        {
            _data = data_type(in_place_index<0>, value);
        }
        else
        {
            _data = data_type(in_place_index<1>, status);
        }
    }
};

template <>
class SANEOpt<void>
{
    bool _ok;
    SANE_Status _status;
    SANEOpt() = default;

public:
    operator bool() const
    {
        return _ok;
    }

    operator SANE_Status() const
    {
        return status();
    }

    [[nodiscard]] SANE_Status status() const
    {
        return _status;
    }

    [[nodiscard]] bool isOk() const
    {
        return _ok;
    }

    SANEOpt(const SANE_Status status):
        _ok(status == SANE_STATUS_GOOD),
        _status(status)
    {
    }
};

using SANE_Info = SANE_Int;

struct SANEOptionDescriptor
{
    string name; /* name of this option (command-line name) */
    string title; /* title of this option (single-line) */
    string desc; /* description of this option (multi-line) */
    SANE_Value_Type type; /* how are values interpreted? */
    SANE_Unit unit; /* what is the (physical) unit? */
    SANE_Int size;
    SANE_Int cap; /* capabilities */

    SANE_Constraint_Type constraint_type;
    using SANEConstraint = optional<variant<vector<string>, vector<SANE_Word>, SANE_Range>>;

    SANEConstraint constraint;

    template <typename Self>
    auto&& constraintStringList(this Self&& self) { return get<0>(std::forward<Self>(self).constraint.value()); }

    template <typename Self>
    auto&& constraintWordList(this Self&& self) { return get<1>(std::forward<Self>(self).constraint.value()); }

    template <typename Self>
    auto&& constraintRange(this Self&& self) { return get<2>(std::forward<Self>(self).constraint.value()); }

    explicit SANEOptionDescriptor(const SANE_Option_Descriptor* p);
};

struct SANEDevice
{
    explicit SANEDevice(const SANE_Device* p);

    string name; /* unique device name */
    string vendor; /* device vendor string */
    string model; /* device model name */
    string type; /* device type (e.g., "flatbed scanner") */
};

class Scanner final : public QObject
{
    Q_OBJECT

    bool _init = false;
    SANE_Int _version = -1;
    vector<SANEDevice> _devices;
    optional<string> _currentDeviceName = nullopt;
    SANE_Handle _currentDeviceHandle = nullptr;
    vector<SANEOptionDescriptor> _currentOptions;
    SANE_Parameters _currentParameters;
    bool _scanning = false;
    vector<char> _currentBuffer;
    QImage _currentImage;
    ospanstream _stream;
    bool _imageEmpty = true;
    bool _pageCanceled = false;
    bool _requestStopScan = false;
    bool _isChangingDevice = false;

public:

    explicit Scanner(QObject* parent = nullptr);

    [[nodiscard]] optional<SANE_Int> version() const
    {
        if (!_init) return nullopt;
        return _version;
    }

    [[nodiscard]] bool initialized() const { return _init; }
    [[nodiscard]] bool scanning() const { return _scanning; }
    [[nodiscard]] bool imageEmpty() const { return _imageEmpty; }
    [[nodiscard]] bool isChangingDevice() const { return _isChangingDevice; }
    [[nodiscard]] bool pageCanceled() const { return _pageCanceled; }
    [[nodiscard]] bool deviceSelected() const { return _currentDeviceName.has_value(); }
    [[nodiscard]] const vector<SANEDevice>& devices() const { return _devices; }
    [[nodiscard]] const optional<string>& currentDeviceName() const { return _currentDeviceName; }
    [[nodiscard]] const vector<SANEOptionDescriptor>& currentOptions() const { return _currentOptions; }
    [[nodiscard]] QPixmap generateCurrentPixmap() const;

    [[nodiscard]] const SANE_Parameters* currentParameters() const
    {
        return deviceSelected() ? &_currentParameters : nullptr;
    }

    [[nodiscard]] SANEOpt<json> getOptionValueAt(int i) const;
    [[nodiscard]] SANEOpt<json> getOptionsValues() const;
    SANEOpt<SANE_Info> setOptionValueAt(int i, const json& value, bool update = true);
    // update les options sans attendre le resultat
    SANEOpt<SANE_Info> setOptionValues(json values, bool update = true); // update les options sans attendre le resultat

    Task<SANEOpt<>> init();
    Task<SANEOpt<>> updateDevices();
    Task<SANEOpt<>> setCurrentDevice(const optional<string>& name);
    Task<SANEOpt<>> setCurrentDevice(int i);
    Task<> exit();

    Task<SANEOpt<>> startScan();
    SANEOpt<> stopScan();
    SANEOpt<> clearCurrentImage();

    static constexpr int BUFFER_SIZE = 1048576;

private:
    void setIsChangingDevice(bool value);
    Task<SANEOpt<>> updateParameters();
    Task<SANEOpt<>> updateOptions();
    // SANEOpt<> updateActiveOptions() const;
    Task<SANEOpt<>> closeCurrentDevice();
    Task<> readLoop();

signals:
    void pageScanned();
    void devicesFound(const vector<SANEDevice>& newValue);
    void currentDeviceChanged(const optional<std::string>& newValue);
    void currentOptionsChanged(const vector<SANEOptionDescriptor>& newValue);
    void currentParametersChanged(const SANE_Parameters* newValue);
    void isChangingDeviceChanged(bool newValue);
};


#endif //SCANNER_H
