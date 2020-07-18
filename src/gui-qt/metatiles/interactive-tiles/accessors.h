/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "resourceitem.h"
#include "gui-qt/accessor/abstractaccessors.h"
#include "gui-qt/project.h"
#include "models/metatiles/interactive-tiles.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
namespace InteractiveTiles {
class ResourceItem;

namespace MT = UnTech::MetaTiles;

class FunctionTableList : public Accessor::NamedListAccessor<MT::InteractiveTileFunctionTable, ResourceItem> {
    Q_OBJECT

    using UndoHelper = Accessor::ListAndSelectionUndoHelper<FunctionTableList>;

public:
    FunctionTableList(ResourceItem* resourceItem);
    ~FunctionTableList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool edit_setSymbol(size_t index, const std::string& symbol);
    bool edit_setSymbolColor(size_t index, const rgba& symbolColor);
};

}
}
}
}
