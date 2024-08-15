//
// Created by benichn on 15/08/24.
//

#ifndef SCANOPTIONSMODEL_H
#define SCANOPTIONSMODEL_H
#include "StaticJsonModel.h"


class ScanOptionsModel : public StaticJsonModel {
    Q_OBJECT
protected:
    [[nodiscard]] json createJson() const override;
    bool editJsonProperty(const JsonStructure::path_t& path, const json& value) const override;
    [[nodiscard]] json createJsonDescriptor(const json& j) const override;

public:
    explicit ScanOptionsModel(QObject* parent = nullptr);
};



#endif //SCANOPTIONSMODEL_H
