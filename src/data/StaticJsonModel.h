//
// Created by benichn on 13/08/24.
//

#ifndef JSONMODEL_H
#define JSONMODEL_H
#include "../imports/stdimports.h"
#include "../imports/jsonimports.h"
#include "../imports/qtimports.h"

struct JsonStructure
{
    struct parent_t
    {
        JsonStructure* ptr = nullptr;
        int index = -1;

        operator bool() const { return ptr; }

        static parent_t null() { return {}; }
        // a cause du bug https://stackoverflow.com/questions/53408962/try-to-understand-compiler-error-message-default-member-initializer-required-be
    };

    using path_t = json_pointer<string>;
    using children_t = vector<JsonStructure>;

    JsonStructure(const path_t& path, const children_t& children);
    JsonStructure(const path_t& path);
    JsonStructure();

    parent_t parent;
    path_t path;
    children_t children;
    bool isContainer;

    void setParents(const parent_t& parent = parent_t::null());

    static JsonStructure fromJsonWithoutParents(const json& j, const path_t& prefix = path_t());
};

class StaticJsonModel : public QAbstractItemModel
{
    Q_OBJECT
    mutable optional<json> _json;
    mutable optional<json> _jsonDescriptor;
    mutable optional<JsonStructure> _jsonStructure;

protected:
    void resetJsonInternal() const;

    [[nodiscard]] virtual json createJson() const = 0;
    [[nodiscard]] virtual json createJsonDescriptor(const json& j) const;
    virtual bool editJsonProperty(const JsonStructure::path_t& path, const json& value) const = 0;

public:
    explicit StaticJsonModel(QObject* parent = nullptr);

    json& jSON() const;
    json& descriptor() const;
    JsonStructure& structure() const;
    void resetJson();

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant data(const QModelIndex& index, int role) const override;

    static json defaultDescriptor(const json& j, const json& templ = {})
    {
        bool settable = templ.contains("settable") ? templ["settable"] : json{true};
        bool nullable = templ.contains("nullable") ? templ["nullable"] : json{false};
        bool active = templ.contains("active") ? templ["active"] : json{true};
        json res;
        for (const auto& kv : j.items())
        {
            json e = {
                {"settable", settable},
                {"nullable", nullable},
                {"active", active}
            };
            switch (kv.value().type())
            {
            case nlohmann::detail::value_t::null:
                e["type"] = "null";
                break;
            case nlohmann::detail::value_t::string:
                e["type"] = "string";
                break;
            case nlohmann::detail::value_t::boolean:
                e["type"] = "bool";
                break;
            case nlohmann::detail::value_t::number_integer:
                e["type"] = "int";
                break;
            case nlohmann::detail::value_t::number_unsigned:
                e["type"] = "int";
                e["details"] = {
                    {"min", 0}
                };
                break;
            case nlohmann::detail::value_t::number_float:
                e["type"] = "float";
                break;
            case nlohmann::detail::value_t::binary:
                e["type"] = "null"; // !
                break;
            case nlohmann::detail::value_t::discarded:
                e["type"] = "null"; // !
                break;
            case nlohmann::detail::value_t::object:
            case nlohmann::detail::value_t::array:
                res[kv.key()] = defaultDescriptor(kv.value(), templ);
                continue;
            }
            res[kv.key()] = e;
        }
        return res;
    }
};


#endif //JSONMODEL_H
