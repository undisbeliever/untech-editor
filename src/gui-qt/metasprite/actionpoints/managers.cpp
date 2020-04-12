/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "accessors.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::ActionPoints;

const QStringList ActionPointFunctionsManager::PARAMETER_TYPE_STRINGS = {
    QStringLiteral("Unused"),
    QStringLiteral("Word"),
    QStringLiteral("Signed Word"),
};

ActionPointFunctionsManager::ActionPointFunctionsManager(QObject* parent)
    : ListAccessorTableManager(parent)
{
    addProperty(tr("Name"), NAME, PropertyType::IDSTRING);
    addProperty(tr("Manually Invoked"), MANUALLY_INVOKED, PropertyType::BOOLEAN);
}

void ActionPointFunctionsManager::setAccessor(ActionPointFunctionsList* ftList)
{
    ListAccessorTableManager::setAccessor(ftList);
}

inline ActionPointFunctionsList* ActionPointFunctionsManager::accessor() const
{
    return static_cast<ActionPointFunctionsList*>(ListAccessorTableManager::accessor());
}

QVariant ActionPointFunctionsManager::data(int index, int id) const
{
    auto* ftList = accessor();
    if (ftList == nullptr
        || index < 0
        || (unsigned)index >= ftList->size()) {

        return QVariant();
    }

    const auto& ap = ftList->list()->at(index);

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return QString::fromStdString(ap.name);

    case PropertyId::MANUALLY_INVOKED:
        return ap.manuallyInvoked;
    }

    return QVariant();
}

bool ActionPointFunctionsManager::setData(int index, int id, const QVariant& value)
{
    auto* apfList = accessor();
    if (apfList == nullptr
        || index < 0
        || (unsigned)index >= apfList->size()) {

        return false;
    }

    switch (static_cast<PropertyId>(id)) {
    case PropertyId::NAME:
        return apfList->edit_setName(index, value.toString().toStdString());

    case PropertyId::MANUALLY_INVOKED:
        return apfList->edit_setManuallyInvoked(index, value.toBool());
    }

    return false;
}
