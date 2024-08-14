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
    bool _ok;

    union
    {
        T _value;
        SANE_Status _status;
    };

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

    template <class Self>
    auto&& value(this Self self) const
    {
        if (!self._ok) throw runtime_error("attempting to access value of a non ok SANEOpt");
        return self._value;
    }

    [[nodiscard]] SANE_Status status() const
    {
        return _ok ? SANE_STATUS_GOOD : _status;
    }

    [[nodiscard]] bool isOk() const
    {
        return _ok;
    }

    SANEOpt(const T& value)
    {
        _ok = true;
        _value = value;
    }

    SANEOpt(const SANE_Status status)
    {
        _ok = false;
        _status = status;
    }

    SANEOpt(const T& value, const SANE_Status status)
    {
        if (status == SANE_STATUS_GOOD)
        {
            _ok = true;
            _value = value;
        }
        else
        {
            _ok = false;
            _status = status;
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
    ostream<SANE_Byte> _stream;

public:
    [[nodiscard]] optional<SANE_Int> version() const { return _init ? nullopt : _version; }
    [[nodiscard]] bool deviceSelected() const { return _currentDevice; }
    [[nodiscard]] const span<const SANE_Device>& devices() const { return _devices; }
    [[nodiscard]] const SANE_Device* currentDevice() const { return _currentDevice; }
    [[nodiscard]] const vector<const SANE_Option_Descriptor*>& currentOptions() const { return _currentOptions; }

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

    Task<SANEOpt<const ostream<SANE_Byte>&>> startScan();
    Task<SANEOpt<>> stopScan();

    constexpr int BUFFER_SIZE = 1048576;

private:
    Task<SANEOpt<>> updateParameters();
    Task<SANEOpt<>> updateOptions();
    Task<SANEOpt<>> closeCurrentDevice();
    Task<> readLoop();

signals:
    void pageScanned();
};


#endif //SCANNER_H
