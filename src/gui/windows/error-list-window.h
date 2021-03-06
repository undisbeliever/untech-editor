/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech::Project {
class ProjectData;
}

namespace UnTech::Gui {
class AbstractEditorData;

void processErrorListWindow(const Project::ProjectData& projectData, AbstractEditorData* editorData);

}
