/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/abstract-editor.h"
#include "gui/selection.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class EntityRomDataEditorData final : public AbstractEditorData {
public:
    using EntityType = UnTech::Entity::EntityType;
    using EntityRomEntry = UnTech::Entity::EntityRomEntry;

private:
    friend class EntityRomDataEditorGui;
    struct AP;

    UnTech::Entity::EntityRomData entityRomData;

    MultipleSelection listIdsSel;

    SingleSelection structsSel;
    MultipleChildSelection structFieldsSel;

    SingleSelection functionTablesSel;

    SingleSelection entitiesSel;
    SingleSelection projectilesSel;
    SingleSelection playersSel;

public:
    EntityRomDataEditorData(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void updateSelection() final;
};

class EntityRomDataEditorGui final : public AbstractEditorGui {
private:
    using AP = EntityRomDataEditorData::AP;

    EntityRomDataEditorData* _data;

public:
    EntityRomDataEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) final;
    virtual void resetState() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile,
                            const Project::ProjectData& projectData) final;

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
