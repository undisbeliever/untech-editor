/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "item-index.h"
#include <memory>
#include <string>

namespace UnTech::Project {
struct ProjectFile;
}

namespace UnTech::Gui {

class AbstractEditor {

private:
    ItemIndex _itemIndex;

protected:
    std::string name;

    // ::TODO undo stack::

public:
    bool pendingChanges = false;

public:
    AbstractEditor(const ItemIndex itemIndex);
    virtual ~AbstractEditor() = default;

    AbstractEditor(const AbstractEditor&) = delete;
    AbstractEditor(AbstractEditor&&) = delete;
    AbstractEditor& operator=(const AbstractEditor&) = delete;
    AbstractEditor& operator=(AbstractEditor&&) = delete;

    ItemIndex itemIndex() const { return _itemIndex; }

    // Return false if itemIndex is invalid
    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) = 0;
    virtual void commitPendingChanges(Project::ProjectFile& projectFile) = 0;

    virtual void editorOpened() = 0;
    virtual void editorClosed() = 0;

    // This is fine - only one Editor is active at any given time.
    virtual void processGui(const Project::ProjectFile& projectFile) = 0;
};

std::unique_ptr<AbstractEditor> createEditor(ItemIndex itemIndex);

}
