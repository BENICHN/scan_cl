//
// Created by benichn on 28/08/24.
//

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "book.h" // ! froward
#include "../imports/stdimports.h"
#include "../imports/jsonimports.h"

class AppSettings final : public QObject
{
    Q_OBJECT
    optional<string> _scanDevName;
    json _scanOptions;
    bool _postTEnabled;
    string _postT;

public:
    explicit AppSettings(QObject* parent = nullptr);

    [[nodiscard]] optional<string> scanDevName() const { return _scanDevName; }
    void setScanDevName(const optional<string>& scanDevName);

    [[nodiscard]] string postT() const { return _postT; }
    void setPostT(const string& postT);

    [[nodiscard]] bool postTEnabled() const { return _postTEnabled; }
    void setPostTEnabled(bool postTEnabled);

    [[nodiscard]] json getScanOptions(PageColorMode mode) const;
    [[nodiscard]] json getScanOptions(const string& devName, PageColorMode mode) const;
    [[nodiscard]] json getRealScanOptions(PageColorMode mode) const;
    [[nodiscard]] json getRealScanOptions(const string& devName, PageColorMode mode) const;
    void setScanOptions(PageColorMode mode, const json& options);
    void setScanOptions(const string& devName, PageColorMode mode, const json& options);

    void save() const;

    friend void to_json(json& j, const AppSettings& astgs);
    void loadFromJson(const json& j);

signals:
    void scanOptionsChanged(const std::string& devName, PageColorMode mode);
};

#endif //APPSETTINGS_H
