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
    mutable optional<json> _jsonPlaceholder;
    mutable optional<json> _jsonDescriptor;
    mutable optional<JsonStructure> _jsonStructure;
    mutable int _resetCounter = 0;

protected:
    void resetJsonInternal() const;

    bool initialized() const;
    virtual void createJson() const;
    virtual void createJsonDescriptor() const;
    virtual void createJsonPlaceholder() const;
    void setJson(const json& j) const { _json = j; }
    void setJsonPlaceholder(const json& j) const { _jsonPlaceholder = j; }
    void setJsonDescriptor(const json& j) const { _jsonDescriptor = j; }
    virtual bool beforeEditJsonProperty(const JsonStructure::path_t& path, const json& value) const;
    virtual void afterEditJsonProperty(const JsonStructure::path_t& path, const json& value) const;

public:
    explicit StaticJsonModel(QObject* parent = nullptr);

    json& jSON() const;
    json& placeholder() const;
    json& descriptor() const;
    JsonStructure& structure() const;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    bool setJsonData(const QModelIndex& index, const json& value, int role = Qt::EditRole);
    QVariant data(const QModelIndex& index, int role) const override;

    static json defaultDescriptor(const json& j, const json& templ = {});

public slots:
    void resetJson();

signals:
    void jsonReset();
};


#endif //JSONMODEL_H
