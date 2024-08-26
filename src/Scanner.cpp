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

    if (sta) _init = true;
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

Task<SANEOpt<>> Scanner::setCurrentDevice(const SANEDevice* dev)
{
    if (!_init) co_return SANE_STATUS_UNSUPPORTED;
    if (deviceSelected()) co_await closeCurrentDevice(); // retourne toujours true
    if (!dev) co_return SANE_STATUS_GOOD;

    SANEOpt sta = co_await QtConcurrent::run([=]
    {
        return sane_open(dev->name.c_str(), &_currentDeviceHandle);
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
    co_return co_await setCurrentDevice(i == -1 ? nullptr : &_devices[i]);
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
        constraint_type == SANE_CONSTRAINT_RANGE ? SANEConstraint{*p->constraint.range} :
            constraint_type == SANE_CONSTRAINT_WORD_LIST ? SANEConstraint{vector<SANE_Word>()} :
            constraint_type == SANE_CONSTRAINT_STRING_LIST ? SANEConstraint{vector<string>()} : nullopt
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
    auto res = QPixmap{_currentParameters.pixels_per_line, _currentParameters.lines};
    res.fill(Qt::white);
    return res;
}

Task<SANEOpt<json>> Scanner::getOptionValueAt(int i) const
{
    if (!_init || !deviceSelected() || i < 0 || i >= _currentOptions.size()) co_return SANE_STATUS_UNSUPPORTED;
    const auto desc = _currentOptions[i];
    switch (desc.type)
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
            const auto v = new SANE_Char[desc.size];
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

// Task<SANEOpt<json>> Scanner::getOptionValue(const SANE_Option_Descriptor* desc) const
// {
//     const int i = str::find(_currentOptions, desc) - _currentOptions.begin();
//     if (i == _currentOptions.size())
//     {
//         co_return SANE_STATUS_UNSUPPORTED;
//     }
//     co_return co_await getOptionValueAt(i);
// }

Task<SANEOpt<json>> Scanner::getOptionsValues() const
{
    json res = {};
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
                const auto v = co_await getOptionValueAt(i);
                if (!v) co_return v;
                res[desc.title] = v.value();
            }
            break;
        }
    }
    co_return res;
}

SANEOpt<SANE_Info> Scanner::setOptionValueAt(const int i, const json& value)
{
    if (!_init || !deviceSelected() || i < 0 || i >= _currentOptions.size()) return SANE_STATUS_UNSUPPORTED;
    const auto desc = _currentOptions[i];
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
    if (inf & SANE_INFO_RELOAD_OPTIONS)
    {
        updateOptions();
    }
    if (inf & SANE_INFO_RELOAD_PARAMS)
    {
        updateParameters();
    }
    return {inf, SANE_STATUS_GOOD};
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
            _currentBuffer = vector<char>(sz);
            _currentImage = QImage(reinterpret_cast<uchar*>(_currentBuffer.data()), _currentParameters.pixels_per_line,
                                   _currentParameters.lines, _currentParameters.bytes_per_line,
                                   toQFormat(_currentParameters));
            _stream = ospanstream(span(_currentBuffer.data(), sz));
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

SANEOpt<> Scanner::clearCurrentImage()
{
    if (_scanning) return SANE_STATUS_UNSUPPORTED;
    _currentImage = {};
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
            if (sta != SANE_STATUS_GOOD)
            {
                _scanning = false;
                _stream.flush();
                co_await QtConcurrent::run([this]
                {
                    return _currentImage.save(app().book().getNewScanPath().c_str());
                }); // ! success
                emit pageScanned();
                co_return;
            }
            _stream.write(buf, len);
        }
        while (len > 0);
        qDebug() << "l0\n";
        co_await delay(200);
    }
}
