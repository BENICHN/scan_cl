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
    auto&& value(this Self self)
    {
        if (!self.isOk()) throw runtime_error("attempting to access value of a non ok SANEOpt");
        return get<0>(self._data);
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
using SANEStream = basic_ostream<SANE_Byte>;

class Scanner final : public QObject
{
    Q_OBJECT

    bool _init = false;
    SANE_Int _version = -1;
    span<const SANE_Device> _devices;
    const SANE_Device* _currentDevice = nullptr;
    SANE_Handle _currentDeviceHandle = nullptr;
    vector<const SANE_Option_Descriptor*> _currentOptions;
    SANE_Parameters _currentParameters;
    bool _scanning = false;
    SANEStream _stream;

public:
    [[nodiscard]] optional<SANE_Int> version() const
    {
        if (!_init) return nullopt;
        return _version;
    }

    [[nodiscard]] bool deviceSelected() const { return _currentDevice; }
    [[nodiscard]] const span<const SANE_Device>& devices() const { return _devices; }
    [[nodiscard]] const SANE_Device* currentDevice() const { return _currentDevice; }
    [[nodiscard]] const vector<const SANE_Option_Descriptor*>& currentOptions() const { return _currentOptions; }
    [[nodiscard]] const SANEStream& stream() const { return _stream; }

    [[nodiscard]] const SANE_Parameters* currentParameters() const
    {
        return deviceSelected() ? &_currentParameters : nullptr;
    }

    [[nodiscard]] Task<SANEOpt<json>> getOptionValueAt(int i) const;
    [[nodiscard]] Task<SANEOpt<json>> getOptionValue(const SANE_Option_Descriptor* desc) const;
    [[nodiscard]] Task<SANEOpt<json>> getOptionsValues() const;

    Task<SANEOpt<SANE_Info>> setOptionsValueAt(int i, const json& value);
    Task<SANEOpt<SANE_Info>> setOptionsValue(const SANE_Option_Descriptor* desc, const json& value);

    Task<SANEOpt<>> init();
    Task<SANEOpt<>> updateDevices();
    Task<SANEOpt<>> setCurrentDevice(const SANE_Device* dev);
    Task<> exit();

    Task<SANEOpt<>> startScan();
    Task<SANEOpt<>> stopScan();

    static constexpr int BUFFER_SIZE = 1048576;

private:
    Task<SANEOpt<>> updateParameters();
    Task<SANEOpt<>> updateOptions();
    Task<SANEOpt<>> closeCurrentDevice();
    Task<> readLoop();

signals:
    void pageScanned();
};


#endif //SCANNER_H
