//
// Created by benichn on 13/08/24.
//

#include "StaticJsonModel.h"

#include "../colors.h"

JsonStructure::JsonStructure(const path_t& path, const children_t& children): parent(nullptr),
                                                                              path(path),
                                                                              children(children),
                                                                              isContainer(true)
{
}

JsonStructure::JsonStructure(const path_t& path): parent(nullptr),
                                                  path(path),
                                                  isContainer(false)
{
}

JsonStructure::JsonStructure() : JsonStructure(path_t(), {})
{
}

void JsonStructure::setParents(const parent_t& parent)
{
    this->parent = parent;
    for (int i = 0; i < children.size(); ++i)
    {
        children[i].setParents({this, i});
    }
}

JsonStructure JsonStructure::fromJsonWithoutParents(const json& j, const path_t& prefix)
{
    switch (j.type())
    {
    case json_value_t::array:
    case json_value_t::object:
        {
            JsonStructure res = {
                prefix,
                children_t(j.size())
            };
            int i = 0;
            for (const auto& kv : j.items())
            {
                auto path = prefix;
                path.push_back(kv.key());
                res.children[i] = fromJsonWithoutParents(kv.value(), path);
                i++;
            }
            return res;
        }
    default:
        return {
            prefix
        };
    }
}

void StaticJsonModel::afterEditJsonProperty(const JsonStructure::path_t& path, const json& value) const
{
}

StaticJsonModel::StaticJsonModel(QObject* parent) : QAbstractItemModel(parent)
{
}

json& StaticJsonModel::jSON() const
{
    if (!initialized()) resetJsonInternal();
    return _json.value();
}

json& StaticJsonModel::placeholder() const
{
    if (!initialized()) resetJsonInternal();
    return _jsonPlaceholder.value();
}

json& StaticJsonModel::descriptor() const
{
    if (!initialized()) resetJsonInternal();
    return _jsonDescriptor.value();
}

JsonStructure& StaticJsonModel::structure() const
{
    if (!initialized()) resetJsonInternal();
    return _jsonStructure.value();
}

void StaticJsonModel::resetJsonInternal() const
{
    ++_resetCounter;
    _json = nullopt;
    _jsonStructure = nullopt;
    _jsonPlaceholder = nullopt;
    _jsonDescriptor = nullopt;
    createJson();
    createJsonDescriptor();
    createJsonPlaceholder();
    _jsonStructure = JsonStructure::fromJsonWithoutParents(_json);
    _jsonStructure->setParents();
}

bool StaticJsonModel::initialized() const
{
    return _json.has_value();
}

void StaticJsonModel::createJson() const
{
    setJson(json::object());
}

void StaticJsonModel::resetJson()
{
    resetJsonInternal();
    endResetModel();
}

void StaticJsonModel::createJsonDescriptor() const
{
    setJsonDescriptor(defaultDescriptor(_json));
}

void StaticJsonModel::createJsonPlaceholder() const
{
    setJsonPlaceholder(json::object());
}

QModelIndex StaticJsonModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!parent.isValid()) // parent is hidden root item
    {
        return createIndex(row, column, &structure().children.at(row));
    }
    const auto& js = *static_cast<JsonStructure*>(parent.internalPointer());
    return createIndex(row, column, &js.children.at(row));
}

QModelIndex StaticJsonModel::parent(const QModelIndex& child) const
{
    const auto& js = *static_cast<JsonStructure*>(child.internalPointer());
    const auto& p = js.parent; // ok car js n'est jamais structure()
    if (const auto& pp = p.ptr->parent) // p != root
    {
        return createIndex(pp.index, 0, p.ptr);
    }
    return {}; // return root
}

Qt::ItemFlags StaticJsonModel::flags(const QModelIndex& index) const
{
    const auto f = QAbstractItemModel::flags(index);
    return index.column() == 1 ? f | Qt::ItemIsEditable : f;
}

int StaticJsonModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) return structure().children.size();
    const auto& js = *static_cast<JsonStructure*>(parent.internalPointer());
    return js.children.size();
}

int StaticJsonModel::columnCount(const QModelIndex& parent) const
{
    // if (!parent.isValid()) return 1;
    // const auto& js = *static_cast<JsonStructure*>(parent.internalPointer());
    // return js.isContainer ? 1 : 2;
    return 2;
}

QVariant StaticJsonModel::data(const QModelIndex& index, int role) const
{
    const auto& js = *static_cast<JsonStructure*>(index.internalPointer());
    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        {
            switch (index.column())
            {
            case 0:
                return js.path.back().c_str();
            case 1:
                {
                    if (js.isContainer) return "";
                    auto v = jSON().at(js.path);
                    if (v.is_null() && placeholder().contains(js.path))
                    {
                        v = placeholder().at(js.path);
                    }
                    const auto s = dumpValue(v);
                    const auto& desc = descriptor().at(js.path);
                    if (desc.contains("details"))
                    {
                        const auto& details = desc["details"];
                        if (details.contains("unit"))
                        {
                            return (s + details["unit"].get_ref<const json::string_t&>()).c_str();
                        }
                    }
                    return s.c_str();
                }
            default:
                break;
            }
        }
        break;
    case Qt::FontRole:
        {
            QFont f{};
            if (!js.parent.ptr->parent && jSON().at(js.path).type() == json_value_t::object)
            {
                f.setBold(true);
            }
            return f;
        }
    case Qt::SizeHintRole:
        return QSize{0, 24};
    // case Qt::BackgroundRole:
    //     return Color::br1;
    case Qt::ForegroundRole:
        {
            if (index.column() == 1 && jSON().at(js.path).is_null())
            {
                return QColor(Qt::gray);
            }
            return QColor(Qt::black);
        }
        break;
    default:
        break;
    }
    return {};
}

json StaticJsonModel::defaultDescriptor(const json& j, const json& templ)
{
    const auto settable = templ.contains("settable") ? templ["settable"] : json(true);
    const auto nullable = templ.contains("nullable") ? templ["nullable"] : json(false);
    const auto active = templ.contains("active") ? templ["active"] : json(true);
    json res;
    for (const auto& kv : j.items())
    {
        json e = {
            {"settable", settable},
            {"name", kv.key()},
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

bool StaticJsonModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return setJsonData(index, qVariantToJson(value), role);
}

bool StaticJsonModel::setJsonData(const QModelIndex& index, const json& value, int role)
{
    const auto& path = static_cast<JsonStructure*>(index.internalPointer())->path;
    const auto c = _resetCounter;
    // const auto t = jSON()[js.path].type();
    // if (js.path.empty()) return false;
    if (beforeEditJsonProperty(path, value) && c == _resetCounter)
    {
        jSON()[path] = value;
        afterEditJsonProperty(path, value);
        return true;
    }
    return false;
}
