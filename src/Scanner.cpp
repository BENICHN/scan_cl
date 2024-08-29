//
// Created by benichn on 13/08/24.
//

#include "Scanner.h"

#include "app.h"
#include "utils.h"

Task<SANEOpt<>> Scanner::init()
{
    if (_init) co_return SANE_STATUS_GOOD;

    SANE_Int version;
    const SANEOpt sta = co_await QtConcurrent::run([&]
    {
        return sane_init(&version, nullptr);
    });

    if (sta)
    {
        _init = true;
        // co_await setCurrentDevice(app().appSettings().scanDevName());
    }
    co_return sta;
}

Task<SANEOpt<>> Scanner::updateDevices()
{
    if (!_init) co_return SANE_STATUS_UNSUPPORTED;
    _devices.clear();

    const SANE_Device** devices;
    const SANEOpt sta = co_await QtConcurrent::run([&]
    {
        return sane_get_devices(&devices, SANE_FALSE);
    });

    if (sta)
    {
        while (*devices)
        {
            _devices.emplace_back(*devices);
            ++devices;
        }
    }
    emit devicesFound(_devices);
    co_return sta;
}

Task<SANEOpt<>> Scanner::setCurrentDevice(const optional<string>& name)
{
    qDebug() << (name.has_value() ? name->c_str() : "null") << '\n';
    if (!_init) co_return SANE_STATUS_UNSUPPORTED;
    setIsChangingDevice(true);
    if (deviceSelected()) co_await closeCurrentDevice(); // retourne toujours true
    if (!name.has_value())
    {
        setIsChangingDevice(false);
        co_return SANE_STATUS_GOOD;
    }

    SANEOpt sta = co_await QtConcurrent::run([=]
    {
        return sane_open(name->c_str(), &_currentDeviceHandle);
    });

    if (sta)
    {
        _currentDeviceName = name;
        app().appSettings().setScanDevName(name);
        sta = co_await updateOptions();
        // setOptionValues(app().appSettings().getScanOptions());
        if (!sta) closeCurrentDevice();
        else
        {
            sta = co_await updateParameters();
            if (!sta) closeCurrentDevice();
        }
    }
    setIsChangingDevice(false);
    emit currentDeviceChanged(_currentDeviceName);
    co_return sta;
}

Task<SANEOpt<>> Scanner::setCurrentDevice(const int i)
{
    if (i != -1 && (i < 0 || i >= _devices.size())) co_return SANE_STATUS_UNSUPPORTED;
    const auto name = i == -1 ? nullopt : optional(_devices[i].name);
    co_return co_await setCurrentDevice(name);
}

Task<> Scanner::exit()
{
    stopScan();
    co_await closeCurrentDevice();
    _init = false;
    co_await QtConcurrent::run([]
    {
        sane_exit();
    });
}

void Scanner::setIsChangingDevice(bool value)
{
    _isChangingDevice = value;
    emit isChangingDeviceChanged(value);
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
    for (int i = 0; currentOption = sane_get_option_descriptor(_currentDeviceHandle, i); ++i) // ! blocking
    {
        _currentOptions.emplace_back(currentOption);
    }
    // const auto sta = updateActiveOptions();
    // if (!sta) co_return sta.status();
    emit currentOptionsChanged(_currentOptions);
    co_return SANE_STATUS_GOOD;
}

// SANEOpt<> Scanner::updateActiveOptions() const
// {
//     // const auto sta = getOptionsValues();
//     // if (!sta) return sta.status();
//     // app().appSettings().setScanOptions(nullopt, sta.value());
//     return SANE_STATUS_GOOD;
// }

Task<SANEOpt<>> Scanner::closeCurrentDevice()
{
    qDebug() << "clos\n";
    if (!_init || !deviceSelected()) co_return SANE_STATUS_UNSUPPORTED;

    const auto h = _currentDeviceHandle;
    _currentDeviceName = nullopt;
    app().appSettings().setScanDevName(nullopt);
    _currentDeviceHandle = nullptr;
    _currentOptions.clear();
    co_await QtConcurrent::run([=] { sane_close(h); });
    emit currentDeviceChanged(_currentDeviceName);
    co_return SANE_STATUS_GOOD;
}

SANEOptionDescriptor::SANEOptionDescriptor(const SANE_Option_Descriptor* p):
    name(p->name ? p->name : ""),
    title(p->title),
    desc(p->desc),
    type(p->type),
    unit(p->unit),
    size(p->size),
    cap(p->cap),
    constraint_type(p->constraint_type),
    constraint(
        constraint_type == SANE_CONSTRAINT_RANGE
            ? SANEConstraint{*p->constraint.range}
            : constraint_type == SANE_CONSTRAINT_WORD_LIST
            ? SANEConstraint{vector<SANE_Word>()}
            : constraint_type == SANE_CONSTRAINT_STRING_LIST
            ? SANEConstraint{vector<string>()}
            : nullopt
    )
{
    switch (constraint_type)
    {
    case SANE_CONSTRAINT_WORD_LIST:
        {
            auto wl = p->constraint.word_list;
            const int l = *wl++;
            auto& cwl = constraintWordList();
            for (int i = 0; i < l; ++i)
            {
                cwl.emplace_back(wl[i]);
            }
        }
        break;
    case SANE_CONSTRAINT_STRING_LIST:
        {
            auto sl = p->constraint.string_list;
            auto& csl = constraintStringList();
            while (*sl)
            {
                csl.emplace_back(*sl);
                ++sl;
            }
        }
        break;
    }
}

SANEDevice::SANEDevice(const SANE_Device* p):
    name(p->name),
    vendor(p->vendor),
    model(p->model),
    type(p->type)
{
}

Scanner::Scanner(QObject* parent) : QObject(parent), _stream({})
{
}

QPixmap Scanner::generateCurrentPixmap() const
{
    if (!deviceSelected()) return {};
    if (_currentImage.sizeInBytes() > 0)
    {
        return QPixmap::fromImage(_currentImage);
    }
    // auto res = QPixmap{_currentParameters.pixels_per_line, _currentParameters.lines};
    // res.fill(Qt::white);
    // return res;
    return {};
}

SANEOpt<json> Scanner::getOptionValueAt(const int i) const
{
    if (!_init || !deviceSelected() || i < 0 || i >= _currentOptions.size()) return SANE_STATUS_UNSUPPORTED;
    const auto desc = _currentOptions[i];
    switch (desc.type)
    {
    case SANE_TYPE_BOOL:
        {
            SANE_Bool v;
            const auto sta = sane_control_option(_currentDeviceHandle, i, SANE_ACTION_GET_VALUE, &v, nullptr);
            return {v == SANE_TRUE, sta};
        }
    case SANE_TYPE_INT:
        {
            SANE_Int v;
            const auto sta = sane_control_option(_currentDeviceHandle, i, SANE_ACTION_GET_VALUE, &v, nullptr);
            return {v, sta};
        }
    case SANE_TYPE_FIXED:
        {
            SANE_Fixed v;
            const auto sta = sane_control_option(_currentDeviceHandle, i, SANE_ACTION_GET_VALUE, &v, nullptr);
            return {SANE_UNFIX(v), sta};
        }
    case SANE_TYPE_STRING:
        {
            const auto v = new SANE_Char[desc.size];
            const auto sta = sane_control_option(_currentDeviceHandle, i, SANE_ACTION_GET_VALUE, v, nullptr);
            const json res = v;
            delete[] v;
            return {res, sta};
        }
    default:
        return json();
    }
}

// Task<SANEOpt<json>> Scanner::getOptionValue(const SANE_Option_Descriptor* desc) const
// {
//     const int i = str::find(_currentOptions, desc) - _currentOptions.begin();
//     if (i == _currentOptions.size())
//     {
//         co_return SANE_STATUS_UNSUPPORTED;
//     }
//     co_return co_await getOptionValueAt(i);
// }

SANEOpt<json> Scanner::getOptionsValues() const
{
    json res = json::object();
    for (int i = 0; i < _currentOptions.size(); ++i)
    {
        if (i == 0) continue; // skip "number of properties"
        const auto desc = _currentOptions[i];
        switch (desc.type)
        {
        case SANE_TYPE_BUTTON:
        case SANE_TYPE_GROUP:
            break;
        default:
            {
                const auto v = getOptionValueAt(i);
                if (!v) v;
                res[desc.title] = v.value();
            }
            break;
        }
    }
    return res;
}

SANEOpt<SANE_Info> Scanner::setOptionValueAt(const int i, const json& value, const bool update)
{
    if (!_init || !deviceSelected() || i < 0 || i >= _currentOptions.size()) return SANE_STATUS_UNSUPPORTED;
    const auto& desc = _currentOptions[i];
    SANE_Info inf;
    switch (desc.type)
    {
    case SANE_TYPE_BOOL:
        {
            SANE_Bool v = value.get<bool>();
            const auto sta = sane_control_option(_currentDeviceHandle, i, SANE_ACTION_SET_VALUE, &v, &inf);
            if (sta != SANE_STATUS_GOOD) return sta;
        }
        break;
    case SANE_TYPE_INT:
        {
            SANE_Int v = value.get<int>();
            const auto sta = sane_control_option(_currentDeviceHandle, i, SANE_ACTION_SET_VALUE, &v, &inf);
            if (sta != SANE_STATUS_GOOD) return sta;
        }
        break;
    case SANE_TYPE_FIXED:
        {
            SANE_Fixed v = SANE_FIX(value.get<double>());
            const auto sta = sane_control_option(_currentDeviceHandle, i, SANE_ACTION_SET_VALUE, &v, &inf);
            if (sta != SANE_STATUS_GOOD) return sta;
        }
        break;
    case SANE_TYPE_STRING:
        {
            auto v = const_cast<SANE_String>(value.get_ref<const string&>().c_str());
            const auto sta = sane_control_option(_currentDeviceHandle, i, SANE_ACTION_SET_VALUE, v, &inf);
            if (sta != SANE_STATUS_GOOD) return sta;
        }
        break;
    default:
        {
            const auto sta = sane_control_option(_currentDeviceHandle, i, SANE_ACTION_SET_VALUE, nullptr, &inf);
            if (sta != SANE_STATUS_GOOD) return sta;
        }
        break;
    }
    if (update)
    {
        if (inf & SANE_INFO_RELOAD_OPTIONS)
        {
            waitFor(updateOptions().then([]{}));
        }
        if (inf & SANE_INFO_RELOAD_PARAMS)
        {
            waitFor(updateParameters().then([]{}));
        }
        // const auto sta = updateActiveOptions();
        // if (!sta) return sta.status();
    }
    return {inf, SANE_STATUS_GOOD};
}

SANEOpt<SANE_Info> Scanner::setOptionValues(json values, const bool update)
{
    bool updateOpts = false;
    bool updateParams = false;
    bool inexact = false;
    SANE_Status lastError;
    while (true)
    {
        const int remainingOpts = values.size();
        for (const auto vals = values; const auto& kv : vals.items())
        {
            const auto& title = kv.key();
            const auto i = str::find_if(_currentOptions, [&](const auto& opt) { return opt.title == title; }) -
                _currentOptions.begin();
            if (i < _currentOptions.size())
            {
                const auto sta = setOptionValueAt(i, kv.value(), false);
                if (sta)
                {
                    values.erase(title);
                    const auto inf = sta.value();
                    if (inf & SANE_INFO_INEXACT)
                    {
                        inexact = true;
                    }
                    if (inf & SANE_INFO_RELOAD_OPTIONS)
                    {
                        updateOpts = true;
                    }
                    if (inf & SANE_INFO_RELOAD_PARAMS)
                    {
                        updateParams = false;
                    }
                }
                else
                {
                    lastError = sta;
                }
            }
        }
        if (remainingOpts == values.size()) break; // !! updateoptions entre temps
    }
    if (update)
    {
        if (updateOpts)
        {
            waitFor(updateOptions().then([]{}));
        }
        if (updateParams)
        {
            waitFor(updateParameters().then([]{}));
        }
        // const auto sta = updateActiveOptions();
        // if (!sta) return sta.status();
    }
    if (values.empty())
    {
        return inexact * SANE_INFO_INEXACT | updateOpts * SANE_INFO_RELOAD_OPTIONS | updateParams *
            SANE_INFO_RELOAD_PARAMS;
    }
    return lastError;
}

// Task<SANEOpt<SANE_Info>> Scanner::setOptionsValue(const SANE_Option_Descriptor* desc, const json& value)
// {
//     const int i = str::find(_currentOptions, desc) - _currentOptions.begin();
//     if (i == _currentOptions.size())
//     {
//         co_return SANE_STATUS_UNSUPPORTED;
//     }
//     co_return co_await setOptionsValueAt(i, value);
// }

QImage::Format toQFormat(const SANE_Parameters& params)
{
    switch (params.format)
    {
    case SANE_FRAME_GRAY:
        switch (params.depth)
        {
        case 1:
            return QImage::Format_Mono;
        case 8:
            return QImage::Format_Grayscale8;
        case 16:
            return QImage::Format_Grayscale16;
        }
        break;
    case SANE_FRAME_RGB:
        switch (params.depth)
        {
        case 8:
            return QImage::Format_RGB888;
        }
        break;
    case SANE_FRAME_RED:
        break;
    case SANE_FRAME_GREEN:
        break;
    case SANE_FRAME_BLUE:
        break;
    }
    throw std::runtime_error("Incompatible image format from scanner");
}

Task<SANEOpt<>> Scanner::startScan()
{
    if (!_init || !deviceSelected()) co_return SANE_STATUS_UNSUPPORTED;
    if (_scanning) co_return SANE_STATUS_DEVICE_BUSY; // ! necessaire ?

    auto sta = co_await QtConcurrent::run([=]
    {
        return sane_start(_currentDeviceHandle);
    });
    if (sta == SANE_STATUS_GOOD)
    {
        sta = co_await updateParameters();
        if (sta == SANE_STATUS_GOOD)
        {
            _scanning = true;
            const auto sz = _currentParameters.bytes_per_line * _currentParameters.lines;
            _currentBuffer = vector<char>(sz, 255);
            _currentImage = QImage(reinterpret_cast<uchar*>(_currentBuffer.data()), _currentParameters.pixels_per_line,
                                   _currentParameters.lines, _currentParameters.bytes_per_line,
                                   toQFormat(_currentParameters));
            _imageEmpty = true;
            _pageCanceled = false;
            _stream = ospanstream(span(_currentBuffer.data(), sz));
            readLoop();
        }
    }

    co_return sta;
}

SANEOpt<> Scanner::stopScan()
{
    if (!_init || !deviceSelected() || !_scanning) return SANE_STATUS_UNSUPPORTED;
    _requestStopScan = true;
    // co_await QtConcurrent::run([=]
    // {
    //     sane_cancel(_currentDeviceHandle);
    // });
    return SANE_STATUS_GOOD;
}

SANEOpt<> Scanner::clearCurrentImage()
{
    if (_scanning) return SANE_STATUS_UNSUPPORTED;
    _currentImage = {};
    _imageEmpty = true;
    _pageCanceled = false;
    _currentBuffer = {};
    return SANE_STATUS_GOOD;
}

Task<> Scanner::readLoop()
{
    SANE_Byte buffer[BUFFER_SIZE];
    auto buf = reinterpret_cast<char*>(buffer);
    int len;
    while (true)
    {
        do
        {
            const auto sta = co_await QtConcurrent::run([this, &len, &buffer]
            {
                return sane_read(_currentDeviceHandle, buffer, BUFFER_SIZE, &len);
            });
            if (_requestStopScan || sta != SANE_STATUS_GOOD)
            {
                if (_requestStopScan)
                {
                    _pageCanceled = true;
                    _requestStopScan = false;
                }
                _scanning = false;
                _stream.flush();
                // co_await QtConcurrent::run([this]
                // {
                //     return _currentImage.save(app().book().getNewScanPath().c_str());
                // }); // ! success
                sane_cancel(_currentDeviceHandle);
                emit pageScanned();
                co_return;
            }
            if (_imageEmpty && len > 0) { _imageEmpty = false; }
            _stream.write(buf, len);
        }
        while (len > 0);
        qDebug() << "l0\n";
        co_await delay(200);
    }
}
