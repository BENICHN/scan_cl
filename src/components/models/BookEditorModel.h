//
// Created by benichn on 04/09/24.
//

#ifndef BOOKEDITORMODEL_H
#define BOOKEDITORMODEL_H

#include "../../data/StaticJsonModel.h"

class BookEditorModel final : public StaticJsonModel
{
    Q_OBJECT

public:
    explicit BookEditorModel(QObject* parent = nullptr);

protected:
    void createJson() const override;
    void afterEditJsonProperty(const JsonStructure::path_t& path, const json& value) const override;
};



#endif //BOOKEDITORMODEL_H
