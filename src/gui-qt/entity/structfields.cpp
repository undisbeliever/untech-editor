/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "structfields.h"
#include "models/entity/entityromdata.h"

#include <QList>

using namespace UnTech::GuiQt::Entity;

using DataType = EN::DataType;

static StructFields::FieldData convertStructField(const EN::StructField& entry, bool isParent)
{
    return {
        QString::fromStdString(entry.name),
        entry.type,
        QString::fromStdString(entry.defaultValue),
        QString::fromStdString(entry.comment),
        isParent
    };
}

StructFields::StructFields()
    : _data()
    , _fieldsInParent()
    , _isValid(false)
    , _entityRomData(nullptr)
{
}

void StructFields::clear()
{
    _data.clear();
    _fieldsInParent = 0;
    _isValid = true;
}

void StructFields::setEntityRomData(const EN::EntityRomData* romData)
{
    clear();
    _entityRomData = romData;
}

void StructFields::rebuildDataForStruct(const EN::EntityRomStruct* selectedStruct)
{
    if (_entityRomData == nullptr) {
        return;
    }
    const auto& structs = _entityRomData->structs;

    clear();

    if (selectedStruct == nullptr) {
        return;
    }

    int fieldsInParent = 0;
    QList<const EN::EntityRomStruct*> structChain;
    {
        structChain.append(selectedStruct);

        auto* s = selectedStruct;
        while (s->parent.isValid()) {
            auto parent = structs.find(s->parent);
            if (!parent) {
                break;
            }
            s = &*parent;
            if (structChain.contains(s)) {
                structChain.clear();
                break;
            }
            structChain.append(s);

            fieldsInParent += s->fields.size();
        }
    }

    if (structChain.empty()) {
        structChain.append(selectedStruct);
        fieldsInParent = 0;
    }
    _fieldsInParent = fieldsInParent;

    _data.reserve(fieldsInParent + selectedStruct->fields.size());
    {
        auto it = structChain.cend();
        while (it != structChain.cbegin()) {
            it--;
            const EN::EntityRomStruct* s = *it;
            Q_ASSERT(s);

            const bool isParent = s != selectedStruct;

            for (auto& e : s->fields) {
                _data.append(convertStructField(e, isParent));
            }
        }
    }
}

int StructFields::updateFieldInChildStruct(size_t fieldIndex, const EN::StructField& entry)
{
    int i = fieldIndex + _fieldsInParent;

    if (_entityRomData == nullptr
        || _data.empty()
        || i < 0 || i >= _data.size()) {

        return 0;
    }

    _data.replace(i, convertStructField(entry, false));

    return i;
}
