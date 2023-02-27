/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/project/compiler-status.h"
#include <gsl/gsl>

namespace UnTech::Gui {
class AbstractEditorData;

void processErrorListWindow(gsl::not_null<AbstractEditorData*> editorData, const Project::ResourceState resourceState, const ErrorList& errors);

}
