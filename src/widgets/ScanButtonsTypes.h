//
// Created by benichn on 27/08/24.
//

#ifndef SCANBUTTONSTYPES_H
#define SCANBUTTONSTYPES_H

#include <QKeySequence>

#include "../imports/stdimports.h"

struct ScanButtonData
{
    ScanButtonData(const string& name, bool checkable, bool checked, bool alone, Qt::Key primary_key, Qt::Key secondary_key = Qt::Key_Escape)
        : name(name),
          checkable(checkable),
          checked(checked),
          alone(alone),
          primaryKey(primary_key),
          secondaryKey(secondary_key)
    {
    }

    string name;
    bool checkable;
    bool checked;
    bool alone;
    Qt::Key primaryKey;
    Qt::Key secondaryKey;

    [[nodiscard]] string keyText() const
    {
        const auto ks = secondaryKey == Qt::Key_Escape ? QKeySequence(primaryKey) : QKeySequence(primaryKey, secondaryKey);
        return ks.toString().toStdString();
    }
};

struct ScanButtonGroup
{
    string name;
    vector<ScanButtonData> buttons;
};
using ScanButtonPanel = vector<ScanButtonGroup>;

#endif //SCANBUTTONSTYPES_H
