//
// Created by benichn on 13/08/24.
//

#include "Scanner.h"

#include "utils.h"

Task<SANEOpt<>> Scanner::init()
{
    if (_init) co_return SANE_STATUS_GOOD;

    SANE_Int version;
    const SANEOpt sta = co_await QtConcurrent::run([&]
    {
        return sane_init(&version, nullptr);
    });

    if (sta) _init = true;
    co_return sta;
}

Task<SANEOpt<>> Scanner::updateDevices()
{
    if (!_init) co_return SANE_STATUS_UNSUPPORTED;
    _devices = {};

    const SANE_Device** devices;
    const SANEOpt sta = co_await QtConcurrent::run([&]
    {
        return sane_get_devices(&devices, SANE_FALSE);
    });

    if (sta)
    {
        int i = 0;
        while (devices[i])
        {
            ++i;
        }
        _devices = span(devices, i);
    }
    emit devicesFound(_devices);
    co_return sta;
}

Task<SANEOpt<>> Scanner::setCurrentDevice(const SANE_Device* dev)
{
    if (!_init) co_return SANE_STATUS_UNSUPPORTED;
    if (deviceSelected()) co_await closeCurrentDevice(); // retourne toujours true
    if (!dev) co_return SANE_STATUS_GOOD;

    SANEOpt sta = co_await QtConcurrent::run([=]
    {
        return sane_open(dev->name, &_currentDeviceHandle);
    });

    if (sta)
    {
        _currentDevice = dev;
        sta = co_await updateOptions();
        if (!sta) closeCurrentDevice();
        else
        {
            sta = co_await updateParameters();
            if (!sta) closeCurrentDevice();
        }
    }
    emit currentDeviceChanged(_currentDevice);
    co_return sta;
}

Task<SANEOpt<>> Scanner::setCurrentDevice(const int i)
{
    if (i != -1 && (i < 0 || i >= _devices.size())) co_return SANE_STATUS_UNSUPPORTED;
    co_return co_await setCurrentDevice(i == -1 ? nullptr : _devices[i]);
}

Task<> Scanner::exit()
{
    co_await stopScan();
    co_await closeCurrentDevice();
    _init = false;
    co_await QtConcurrent::run([]
    {
        sane_exit();
    });
}

Task<SANEOpt<>> Scanner::updateParameters()
{
    if (!_init || !deviceSelected()) co_return SANE_STATUS_UNSUPPORTED;
    const auto sta = co_await QtConcurrent::run([&, this]
    {
        return sane_get_parameters(_currentDeviceHandle, &_currentParameters);
    });
    emit currentParametersChanged(currentParameters());
    co_return sta;
}

Task<SANEOpt<>> Scanner::updateOptions()
{
    if (!_init || !deviceSelected()) co_return SANE_STATUS_UNSUPPORTED;

    _currentOptions.clear();
    const SANE_Option_Descriptor* currentOption;
    for (int i = 0; currentOption = sane_get_option_descriptor(_currentDeviceHandle, i); ++i)
    {
        _currentOptions.emplace_back(currentOption);
    }
    emit currentOptionsChanged(_currentOptions);
    co_return SANE_STATUS_GOOD;
}

Task<SANEOpt<>> Scanner::closeCurrentDevice()
{
    if (!_init || !deviceSelected()) co_return SANE_STATUS_UNSUPPORTED;

    const auto h = _currentDeviceHandle;
    _currentDevice = nullptr;
    _currentDeviceHandle = nullptr;
    _currentOptions.clear();
    co_await QtConcurrent::run([=] { sane_close(h); });
    emit currentDeviceChanged(_currentDevice);
    co_return SANE_STATUS_GOOD;
}

Scanner::Scanner(QObject* parent) : QObject(parent), _stream({})
{
}

Task<SANEOpt<json>> Scanner::getOptionValueAt(int i) const
{
    if (!_init || !deviceSelected() || i < 0 || i >= _currentOptions.size()) co_return SANE_STATUS_UNSUPPORTED;
    const auto desc = _currentOptions[i];
    switch (desc->type)
    {
    case SANE_TYPE_BOOL:
        {
            SANE_Bool v;
            const auto sta = co_await QtConcurrent::run([=, &v]
            {
                return sane_control_option(_currentDeviceHandle, i, SANE_ACTION_GET_VALUE, &v, nullptr);
            });
            co_return {v == SANE_TRUE, sta};
        }
    case SANE_TYPE_INT:
        {
            SANE_Int v;
            const auto sta = co_await QtConcurrent::run([=, &v]
            {
                return sane_control_option(_currentDeviceHandle, i, SANE_ACTION_GET_VALUE, &v, nullptr);
            });
            co_return {v, sta};
        }
    case SANE_TYPE_FIXED:
        {
            SANE_Fixed v;
            const auto sta = co_await QtConcurrent::run([=, &v]
            {
                return sane_control_option(_currentDeviceHandle, i, SANE_ACTION_GET_VALUE, &v, nullptr);
            });
            co_return {SANE_UNFIX(v), sta};
        }
    case SANE_TYPE_STRING:
        {
            const auto v = new SANE_Char[desc->size];
            const auto sta = co_await QtConcurrent::run([=]
            {
                return sane_control_option(_currentDeviceHandle, i, SANE_ACTION_GET_VALUE, v, nullptr);
            });
            const json res = v;
            delete[] v;
            co_return {res, sta};
        }
    default:
        co_return nullptr;
    }
}

Task<SANEOpt<json>> Scanner::getOptionValue(const SANE_Option_Descriptor* desc) const
{
    const int i = str::find(_currentOptions, desc) - _currentOptions.begin();
    if (i == _currentOptions.size())
    {
        co_return SANE_STATUS_UNSUPPORTED;
    }
    co_return co_await getOptionValueAt(i);
}

Task<SANEOpt<json>> Scanner::getOptionsValues() const
{
    json res = {};
    for (int i = 0; i < _currentOptions.size(); ++i)
    {
        const auto desc = _currentOptions[i];
        switch (desc->type)
        {
        case SANE_TYPE_BUTTON:
        case SANE_TYPE_GROUP:
            break;
        default:
            {
                const auto v = co_await getOptionValueAt(i);
                if (!v) co_return v;
                res[desc->title] = v.value();
            }
            break;
        }
    }
    co_return res;
}

Task<SANEOpt<SANE_Info>> Scanner::setOptionsValueAt(const int i, const json& value)
{
    if (!_init || !deviceSelected() || i < 0 || i >= _currentOptions.size()) co_return SANE_STATUS_UNSUPPORTED;
    const auto desc = _currentOptions[i];
    SANE_Info inf;
    switch (desc->type)
    {
    case SANE_TYPE_BOOL:
        {
            SANE_Bool v = value.get<bool>();
            const auto sta = co_await QtConcurrent::run([&, this]
            {
                return sane_control_option(_currentDeviceHandle, i, SANE_ACTION_SET_VALUE, &v, &inf);
            });
            if (!sta) co_return sta;
        }
    case SANE_TYPE_INT:
        {
            SANE_Int v = value.get<int>();
            const auto sta = co_await QtConcurrent::run([&, this]
            {
                return sane_control_option(_currentDeviceHandle, i, SANE_ACTION_SET_VALUE, &v, &inf);
            });
            if (!sta) co_return sta;
        }
    case SANE_TYPE_FIXED:
        {
            SANE_Fixed v = SANE_FIX(value.get<double>());
            const auto sta = co_await QtConcurrent::run([&, this]
            {
                return sane_control_option(_currentDeviceHandle, i, SANE_ACTION_SET_VALUE, &v, &inf);
            });
            if (!sta) co_return sta;
        }
    case SANE_TYPE_STRING:
        {
            SANE_String v = value.get<string>().data();
            const auto sta = co_await QtConcurrent::run([&, this]
            {
                return sane_control_option(_currentDeviceHandle, i, SANE_ACTION_SET_VALUE, &v, &inf);
            });
            if (!sta) co_return sta;
        }
    default:
        {
            const auto sta = co_await QtConcurrent::run([&, this]
            {
                return sane_control_option(_currentDeviceHandle, i, SANE_ACTION_SET_VALUE, nullptr, &inf);
            });
            if (!sta) co_return sta;
        }
    }
    auto st = SANE_STATUS_GOOD;
    switch (inf)
    {
    case SANE_INFO_RELOAD_OPTIONS:
        st = co_await updateOptions();
        break;
    case SANE_INFO_RELOAD_PARAMS:
        st = co_await updateParameters();
        break;
    }
    co_return {inf, st};
}

Task<SANEOpt<SANE_Info>> Scanner::setOptionsValue(const SANE_Option_Descriptor* desc, const json& value)
{
    const int i = str::find(_currentOptions, desc) - _currentOptions.begin();
    if (i == _currentOptions.size())
    {
        co_return SANE_STATUS_UNSUPPORTED;
    }
    co_return co_await setOptionsValueAt(i, value);
}

Task<SANEOpt<>> Scanner::startScan()
{
    if (!_init || !deviceSelected()) co_return SANE_STATUS_UNSUPPORTED;
    if (_scanning) co_return SANE_STATUS_DEVICE_BUSY; // ! necessaire ?

    auto sta = co_await QtConcurrent::run([=]
    {
        return sane_start(_currentDeviceHandle);
    });
    if (sta)
    {
        sta = co_await updateParameters();
        if (sta)
        {
            _buffer = vector<SANE_Byte>(_currentParameters.bytes_per_line * _currentParameters.lines);
            _stream = SANEStream(span(_buffer.data(), _buffer.size()));
            readLoop();
        }
    }

    co_return sta;
}

Task<SANEOpt<>> Scanner::stopScan()
{
    if (!_init || !deviceSelected() || !_scanning) co_return SANE_STATUS_UNSUPPORTED;
    co_await QtConcurrent::run([=]
    {
        sane_cancel(_currentDeviceHandle);
    });
    co_return SANE_STATUS_GOOD;
}

void Scanner::clearPageBuffer()
{
    _buffer = {};
}

Task<> Scanner::readLoop()
{
    SANE_Byte buffer[BUFFER_SIZE];
    int len;
    while (true)
    {
        do
        {
            const auto sta = sane_read(_currentDeviceHandle, buffer, BUFFER_SIZE, &len); // ! blocking
            if (!sta)
            {
                _scanning = false;
                emit pageScanned();
                co_return;
            }
            _stream.write(buffer, len);
        }
        while (len == BUFFER_SIZE);
        co_await delay(200);
    }
}
