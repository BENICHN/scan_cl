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

StaticJsonModel::StaticJsonModel(QObject* parent) : QAbstractItemModel(parent)
{
}

json& StaticJsonModel::jSON() const
{
    if (!_json.has_value()) resetJsonInternal();
    return _json.value();
}

json& StaticJsonModel::descriptor() const
{
    if (!_json.has_value()) resetJsonInternal();
    return _jsonDescriptor.value();
}

JsonStructure& StaticJsonModel::structure() const
{
    if (!_json.has_value()) resetJsonInternal();
    return _jsonStructure.value();
}

void StaticJsonModel::resetJsonInternal() const
{
    _json = createJson();
    _jsonDescriptor = createJsonDescriptor(_json);
    _jsonStructure = JsonStructure::fromJsonWithoutParents(_json);
    _jsonStructure->setParents();
}

void StaticJsonModel::resetJson()
{
    resetJsonInternal();
    endResetModel();
}

json StaticJsonModel::createJsonDescriptor(const json& j) const
{
    return defaultDescriptor(j);
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
                    const auto s = dumpValue(jSON().at(js.path));
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
        break;
    case Qt::SizeHintRole:
        return QSize{0, 24};
    // case Qt::BackgroundRole:
    //     return Color::br1;
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
    auto path = static_cast<JsonStructure*>(index.internalPointer())->path; // pas ref pour si le path est détruit par l'appel à editJsonProprerty
    // const auto t = jSON()[js.path].type();
    // if (js.path.empty()) return false;
    if (editJsonProperty(path, value))
    {
        jSON()[path] = value;
        return true;
    }
    return false;
}
