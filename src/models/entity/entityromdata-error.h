/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
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
    template <typename... Args>
    StructFieldError(const EntityRomStruct& rs, const StructField& field, const Args... message)
        : _romStruct(&rs)
        , _fieldIndex(calcFieldIndex(rs, &field))
        , _message(field.name.isValid()
                       ? stringBuilder("EntityRomStruct ", rs.name, ", Field ", field.name, ": ", message...)
                       : stringBuilder("EntityRomStruct ", rs.name, ", Field #", _fieldIndex, ": ", message...))
    {
    }

    template <typename... Args>
    StructFieldError(const EntityRomStruct& rs, unsigned fIndex, const Args... message)
        : _romStruct(&rs)
        , _fieldIndex(fIndex)
        , _message(stringBuilder("EntityRomStruct ", rs.name, ", Field #", _fieldIndex, ": ", message...))
    {
    }

    const void* romStruct() const { return _romStruct; }
    unsigned fieldIndex() const { return _fieldIndex; }

    virtual std::string message() const final { return _message; }
};

class EntityRomStructError : public ListItemError {
public:
    template <typename... Args>
    EntityRomStructError(const EntityRomStruct& rs, const Args&... message)
        : ListItemError(&rs, stringBuilder("EntityRomStruct ", rs.name, ": ", message...))
    {
    }
};

class EntityFunctionTableError : public ListItemError {
public:
    template <typename... Args>
    EntityFunctionTableError(const EntityFunctionTable& ft, const Args... message)
        : ListItemError(&ft, "EntityFunctionTable ", ft.name, ": ", message...)
    {
    }
};

class EntityRomEntryError : public ListItemError {
public:
    template <typename... Args>
    EntityRomEntryError(const EntityRomEntry& re, const Args... message)
        : ListItemError(&re, "EntityRomEntry ", re.name, ": ", message...)
    {
    }
};

}
}
