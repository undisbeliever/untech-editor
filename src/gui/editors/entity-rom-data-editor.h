/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/abstract-editor.h"
#include "gui/selection.h"
#include "gui/splitter.h"
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
    explicit EntityRomDataEditorData(ItemIndex itemIndex);

    bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    void errorDoubleClicked(const AbstractError*) final;
    void updateSelection() final;
};

class EntityRomDataEditorGui final : public AbstractEditorGui {
private:
    using AP = EntityRomDataEditorData::AP;

    std::shared_ptr<EntityRomDataEditorData> _data;

    SplitterBarState _sidebar;
    SplitterBarState _leftTopbar;
    SplitterBarState _listIdsSidebar;
    SplitterBarState _playersTopbar;
    SplitterBarState _projectilesTopbar;

public:
    EntityRomDataEditorGui();

    bool setEditorData(const std::shared_ptr<AbstractEditorData>& data) final;
    void resetState() final;
    void editorClosed() final;

    void processGui(const Project::ProjectFile& projectFile,
                    const Project::ProjectData& projectData) final;

private:
    void listIdsGui();
    void structsGui();
    void functionTablesGui(const Project::ProjectFile& projectFile);

    template <typename ActionPolicy>
    void entityEntriesGui(const char8_t* text, const Project::ProjectFile& projectFile);

    [[nodiscard]] std::vector<unsigned> generateStructChain(const idstring& name) const;
};

}
