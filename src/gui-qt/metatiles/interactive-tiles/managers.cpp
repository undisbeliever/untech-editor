/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "accessors.h"
#include "gui-qt/common/helpers.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles::InteractiveTiles;

FunctionTableManager::FunctionTableManager(QObject* parent)
    : ListAccessorTableManager(parent)
{
    setTitle(tr("Interactive Tile Function Tables"));

    setItemsMovable(true);

    addProperty(tr("Name"), NAME, PropertyType::IDSTRING);
    addProperty(tr("Symbol"), SYMBOL, PropertyType::STRING);
    addProperty(tr("Symbol Color"), SYMBOL_COLOR, PropertyType::COLOR);
}

void FunctionTableManager::setFunctionTableList(FunctionTableList* tileFunctionList)
{
    setAccessor(tileFunctionList);
}

inline FunctionTableList* FunctionTableManager::accessor() const
{
    return static_cast<FunctionTableList*>(ListAccessorTableManager::accessor());
}

QVariant FunctionTableManager::data(int index, int id) const
{
    auto* ftList = accessor();
    if (ftList == nullptr
        || index < 0
        || (unsigned)index >= ftList->size()) {

        return QVariant();
    }

    const MT::InteractiveTileFunctionTable& ft = ftList->list()->at(index);

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return QString::fromStdString(ft.name);

    case PropertyId::SYMBOL:
        return QString::fromStdString(ft.symbol);

    case PropertyId::SYMBOL_COLOR:
        return fromRgba(ft.symbolColor);
    }

    return QVariant();
}

bool FunctionTableManager::setData(int index, int id, const QVariant& value)
{
    auto* ftList = accessor();
    if (ftList == nullptr
        || index < 0
        || (unsigned)index >= ftList->size()) {

        return false;
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return ftList->edit_setName(index, value.toString().toStdString());

    case PropertyId::SYMBOL:
        return ftList->edit_setSymbol(index, value.toString().toStdString());

    case PropertyId::SYMBOL_COLOR:
        return ftList->edit_setSymbolColor(index, toRgba(value.value<QColor>()));
    }

    return false;
}

FixedFunctionTableManager::FixedFunctionTableManager(QObject* parent)
    : PropertyTableManager(parent)
{
    setTitle(tr("Fixed Interactive Tile Function Tables"));

    setItemsMovable(false);
    setEnabled(false);

    addProperty(tr("Name"), PropertyId::NAME, PropertyType::IDSTRING);
    addProperty(tr("Symbol"), PropertyId::SYMBOL, PropertyType::STRING);
    addProperty(tr("Symbol Color"), PropertyId::SYMBOL_COLOR, PropertyType::COLOR);
}

int FixedFunctionTableManager::rowCount() const
{
    return MT::InteractiveTiles::FIXED_FUNCTION_TABLES.size();
}

QVariant FixedFunctionTableManager::data(int index, int id) const
{
    const auto& ftList = MT::InteractiveTiles::FIXED_FUNCTION_TABLES;

    if (index < 0
        || (unsigned)index >= ftList.size()) {

        return QVariant();
    }

    const MT::InteractiveTileFunctionTable& ft = ftList.at(index);

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return QString::fromStdString(ft.name);

    case PropertyId::SYMBOL:
        return QString::fromStdString(ft.symbol);

    case PropertyId::SYMBOL_COLOR:
        return fromRgba(ft.symbolColor);
    }

    return QVariant();
}

bool FixedFunctionTableManager::setData(int, int, const QVariant&)
{
    return false;
}
