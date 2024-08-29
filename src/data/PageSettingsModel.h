//
// Created by benichn on 13/08/24.
//

#ifndef PAGESETTINGSMODEL_H
#define PAGESETTINGSMODEL_H

#include "StaticJsonModel.h"


class PageSettingsModel final : public StaticJsonModel
{
    Q_OBJECT
    int _pageId = -1;

public:
    explicit PageSettingsModel(int pageId, QObject* parent = nullptr);
    explicit PageSettingsModel(QObject* parent = nullptr);

    [[nodiscard]] int pageId() const { return _pageId; }
    void setPageId(int pageId);

protected:
    void createJsonPlaceholder() const override;
    void createJson() const override;
    bool beforeEditJsonProperty(const JsonStructure::path_t& path, const json& value) const override;
    void createJsonDescriptor() const override;
};


#endif //PAGESETTINGSMODEL_H
