//
// Created by benichn on 13/08/24.
//

#ifndef PAGESETTINGSMODEL_H
#define PAGESETTINGSMODEL_H

#include "StaticJsonModel.h"
#include "../components/props/PropsEditorTypes.h"


class PageSettingsModel final : public StaticJsonModel
{
    Q_OBJECT
    PropsSource _source;

public:
    explicit PageSettingsModel(QObject* parent = nullptr);

    [[nodiscard]] const auto& source() const { return _source; }
    void setSource(const PropsSource& source);

protected:
    void createJsonPlaceholder() const override;
    void createJson() const override;
    void afterEditJsonProperty(const JsonStructure::path_t& path, const json& value) const override;
    void createJsonDescriptor() const override;
};


#endif //PAGESETTINGSMODEL_H
