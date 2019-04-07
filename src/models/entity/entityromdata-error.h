/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "entityromdata.h"
#include "models/common/errorlist.h"

namespace UnTech {
namespace Entity {

class StructFieldError : public AbstractSpecializedError {

    static inline unsigned calcFieldIndex(const EntityRomStruct& rs, const StructField* field)
    {
        auto it = std::find_if(rs.fields.begin(), rs.fields.end(),
                               [&](const auto& i) { return &i == field; });
        return std::distance(rs.fields.cbegin(), it);
    }

    const void* _romStruct;
    const unsigned _fieldIndex;
    const std::string _message;

public:
    StructFieldError(const EntityRomStruct& rs, const StructField& field, const std::string& message, bool showFieldName = false)
        : _romStruct(&rs)
        , _fieldIndex(calcFieldIndex(rs, &field))
        , _message(showFieldName && field.name.isValid()
                       ? "EntityRomStruct " + rs.name + ", Field " + field.name + ": " + message
                       : "EntityRomStruct " + rs.name + ", Field #" + std::to_string(_fieldIndex) + ": " + message)
    {
    }

    StructFieldError(const EntityRomStruct& rs, unsigned fIndex, const std::string& message)
        : _romStruct(&rs)
        , _fieldIndex(fIndex)
        , _message("EntityRomStruct " + rs.name + ", Field #" + std::to_string(_fieldIndex) + ": " + message)
    {
    }

    const void* romStruct() const { return _romStruct; }
    unsigned fieldIndex() const { return _fieldIndex; }

    virtual std::string message() const final { return _message; }
};

class EntityRomStructError : public ListItemError {
public:
    EntityRomStructError(const EntityRomStruct& rs, const std::string& message)
        : ListItemError(&rs, "EntityRomStruct " + rs.name + ": " + message)
    {
    }
};

class EntityFunctionTableError : public ListItemError {
public:
    EntityFunctionTableError(const EntityFunctionTable& ft, const std::string& message)
        : ListItemError(&ft, "EntityFunctionTable " + ft.name + ": " + message)
    {
    }
};

class EntityRomEntryError : public ListItemError {
public:
    EntityRomEntryError(const EntityRomEntry& re, const std::string& message)
        : ListItemError(&re, "EntityRomEntry " + re.name + ": " + message)
    {
    }
};

}
}
