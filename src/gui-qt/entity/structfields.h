/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/entity/entityromdata.h"
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace Entity {
class EntityRomStructsResourceItem;

namespace EN = UnTech::Entity;

class StructFields {
public:
    struct FieldData {
        QString name;
        EN::DataType type;
        QString defaultValue;
        QString comment;
        bool isParent;
    };

private:
    QVector<FieldData> _data;
    int _fieldsInParent;
    bool _isValid;

    const EN::EntityRomData* _entityRomData;

public:
    StructFields();
    ~StructFields() = default;

    bool isValid() const { return _isValid; }
    int fieldsInParent() const { return _fieldsInParent; }

    int size() const { return _data.size(); }
    const FieldData& at(int i) const { return _data.at(i); }

    void clear();

    void setEntityRomData(const EN::EntityRomData* romData);

    // Rebuilds the data fields
    void rebuildDataForStruct(const EN::EntityRomStruct* entityStruct);

    // Does not update parent fields
    // returns the data index that was updated
    int updateFieldInChildStruct(size_t fieldIndex, const UnTech::Entity::StructField& entry);
};

}
}
}
