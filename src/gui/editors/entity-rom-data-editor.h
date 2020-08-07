/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/abstract-editor.h"
#include "gui/selection.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class EntityRomDataEditor final : public AbstractEditor {
public:
    using EntityType = UnTech::Entity::EntityType;
    using EntityRomEntry = UnTech::Entity::EntityRomEntry;

private:
    struct AP;

    UnTech::Entity::EntityRomData _entityRomData;

    MultipleSelection _listIdsSel;

    SingleSelection _structsSel;
    MultipleChildSelection _structFieldsSel;

    SingleSelection _functionTablesSel;

    SingleSelection _entitiesSel;
    SingleSelection _projectilesSel;
    SingleSelection _playersSel;

public:
    EntityRomDataEditor(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile) final;
    virtual void updateSelection() final;

private:
    void listIdsWindow();
    void structsWindow();
    void functionTablesWindow(const Project::ProjectFile& projectFile);

    template <typename ActionPolicy>
    void entityEntriesWindow(const char* name,
                             const Project::ProjectFile& projectFile);

    std::vector<unsigned> generateStructChain(const idstring& name) const;
};

}
