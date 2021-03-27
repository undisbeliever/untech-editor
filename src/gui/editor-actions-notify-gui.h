/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/type-traits.h"
#include <type_traits>

namespace UnTech::Gui {

class AbstractEditorGui;

template <typename AP, typename = void>
struct has_validFlag_member : std::false_type {
};

template <typename AP>
struct has_validFlag_member<AP, std::void_t<decltype(AP::validFlag)>> : std::true_type {
};

template <typename ActionPolicy>
typename std::enable_if_t<has_validFlag_member<ActionPolicy>::value, void>
editorUndoAction_notifyGui(AbstractEditorGui* abstractGui)
{
    using GuiClass = typename member_class<decltype(ActionPolicy::validFlag)>::type;

    if (auto* gui = dynamic_cast<GuiClass*>(abstractGui)) {
        gui->*ActionPolicy::validFlag = false;
    }
}

template <typename ActionPolicy>
typename std::enable_if_t<not has_validFlag_member<ActionPolicy>::value, void>
editorUndoAction_notifyGui(AbstractEditorGui*)
{
}

}
