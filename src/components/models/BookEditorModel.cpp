//
// Created by benichn on 04/09/24.
//

#include "BookEditorModel.h"

#include "../../app.h"

BookEditorModel::BookEditorModel(QObject* parent) : StaticJsonModel(parent)
{
    connect(&app().book(), &Book::bookReset, this, &StaticJsonModel::resetJson);
}

void BookEditorModel::createJson() const
{
    setJson(app().book().globalSettings());
}

void BookEditorModel::afterEditJsonProperty(const JsonStructure::path_t& path, const json& value) const
{
    app().book().setGlobalSettings(jSON());
}