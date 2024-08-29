//
// Created by benichn on 15/08/24.
//

#ifndef SCANOPTIONSMODEL_H
#define SCANOPTIONSMODEL_H
#include "book.h"
#include "StaticJsonModel.h"


class ScanOptionsModel : public StaticJsonModel
{
    Q_OBJECT
    PageColorMode _mode;

public:
    [[nodiscard]] PageColorMode mode() const { return _mode; }
    void setMode(PageColorMode mode);
    explicit ScanOptionsModel(QObject* parent = nullptr);
    void updatePlaceholder();

protected:
    void createJson() const override;
    bool beforeEditJsonProperty(const JsonStructure::path_t& path, const json& value) const override;
    void afterEditJsonProperty(const JsonStructure::path_t& path, const json& value) const override;
    void createJsonPlaceholder() const override;
    void createJsonDescriptor() const override;
};


#endif //SCANOPTIONSMODEL_H
