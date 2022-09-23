/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/type-traits.h"

namespace UnTech::Gui {

class AbstractEditorGui;

template <typename ActionPolicy>
requires requires { ActionPolicy::validFlag; }
void editorUndoAction_notifyGui(AbstractEditorGui* abstractGui)
{
    using GuiClass = typename member_class<decltype(ActionPolicy::validFlag)>::type;

    if (auto* gui = dynamic_cast<GuiClass*>(abstractGui)) {
        gui->*ActionPolicy::validFlag = false;
    }
}

template <typename ActionPolicy>
void editorUndoAction_notifyGui(AbstractEditorGui*)
{
}

}
