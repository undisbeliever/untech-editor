/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstract-editor.h"
#include "models/common/aabb.h"
#include "models/common/vectorset-upoint.h"

namespace UnTech::Gui {

template <typename ActionPolicy>
struct GridActions {
    using EditorT = typename ActionPolicy::EditorT;
    using EditorDataT = typename ActionPolicy::EditorDataT;
    using ListArgsT = typename ActionPolicy::ListArgsT;
    using GridT = typename ActionPolicy::GridT;

    static const GridT* getEditorGridPtr(EditorT* editor, const ListArgsT& listArgs)
    {
        EditorDataT* data = ActionPolicy::getEditorData(*editor);
        assert(data != nullptr);
        return std::apply(&ActionPolicy::getGrid,
                          std::tuple_cat(std::forward_as_tuple(*data), listArgs));
    }

    class BaseAction : public EditorUndoAction {
    private:
        EditorT* const editor;
        const ListArgsT listArgs;

    protected:
        BaseAction(EditorT* editor, const ListArgsT& listArgs)
            : editor(editor)
            , listArgs(listArgs)
        {
            assert(editor != nullptr);
        }
        virtual ~BaseAction() = default;

        GridT& getProjectGrid(Project::ProjectFile& projectFile) const
        {
            EditorDataT* data = ActionPolicy::getEditorData(projectFile, editor->itemIndex());
            assert(data != nullptr);
            GridT* grid = std::apply(&ActionPolicy::getGrid,
                                     std::tuple_cat(std::forward_as_tuple(*data), listArgs));
            assert(grid != nullptr);
            return *grid;
        }

        GridT& getEditorGrid() const
        {
            EditorDataT* data = ActionPolicy::getEditorData(*editor);
            assert(data != nullptr);
            GridT* grid = std::apply(&ActionPolicy::getGrid,
                                     std::tuple_cat(std::forward_as_tuple(*data), listArgs));
            assert(grid != nullptr);
            return *grid;
        }

        void clearSelection() const
        {
            upoint_vectorset& sel = editor->*ActionPolicy::SelectionPtr;
            sel.clear();
        }
    };

    class EditGridAction final : public BaseAction {
    private:
        const GridT newGrid;
        // set by firstDo()
        GridT oldGrid;

    public:
        EditGridAction(EditorT* editor, const ListArgsT& listArgs, const GridT&& g)
            : BaseAction(editor, listArgs)
            , newGrid(g)
            , oldGrid()
        {
        }
        virtual ~EditGridAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            GridT& projectGrid = this->getProjectGrid(projectFile);

            oldGrid = projectGrid;

            setCells(projectFile, newGrid);

            // operator!= may not implemented in a few of my structs
            return !(newGrid == oldGrid);
        }

        void setCells(Project::ProjectFile& projectFile, const GridT& grid) const
        {
            GridT& projectGrid = this->getProjectGrid(projectFile);
            GridT& editorGrid = this->getEditorGrid();

            projectGrid = grid;
            editorGrid = grid;

            if (oldGrid.size() != newGrid.size()) {
                this->clearSelection();
            }
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            setCells(projectFile, oldGrid);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            setCells(projectFile, newGrid);
        }
    };

    class EditMultipleCellsAction final : public BaseAction {
    private:
        const upoint position;
        const GridT newValues;
        // set by firstDo()
        GridT oldValues;

    public:
        EditMultipleCellsAction(EditorT* editor, const ListArgsT& listArgs, upoint p, const GridT&& g)
            : BaseAction(editor, listArgs)
            , position(p)
            , newValues(g)
            , oldValues()
        {
        }
        virtual ~EditMultipleCellsAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            GridT& projectGrid = this->getProjectGrid(projectFile);
            GridT& editorGrid = this->getEditorGrid();

            assert(projectGrid.size() == editorGrid.size());
            assert(position.x + newValues.width() <= projectGrid.width());
            assert(position.y + newValues.height() <= projectGrid.height());

            oldValues = projectGrid.subGrid(position, newValues.size());

            projectGrid.setCells(position, newValues);

            // operator!= may not implemented in a few of my structs
            return !(newValues == oldValues);
        }

        void setCells(Project::ProjectFile& projectFile, const GridT& values) const
        {
            GridT& projectGrid = this->getProjectGrid(projectFile);
            GridT& editorGrid = this->getEditorGrid();

            assert(projectGrid.size() == editorGrid.size());
            assert(position.x + values.width() <= projectGrid.width());
            assert(position.y + values.height() <= projectGrid.height());

            projectGrid.setCells(position, values);
            editorGrid.setCells(position, values);
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            setCells(projectFile, oldValues);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            setCells(projectFile, newValues);
        }
    };

    static void gridTilesPlaced(EditorT* editor, const urect r)
    {
        const ListArgsT listArgs;

        const GridT* grid = getEditorGridPtr(editor, listArgs);
        if (grid == nullptr) {
            return;
        }

        if (r.width < 1 || r.height < 1
            || r.right() > grid->width() || r.bottom() > grid->height()) {

            return;
        }

        editor->addAction(std::make_unique<EditMultipleCellsAction>(
            editor, listArgs, r.topLeft(), grid->subGrid(r)));
    }

    static void resizeGrid(EditorT* editor, const usize newSize)
    {
        const ListArgsT listArgs;

        const GridT* grid = getEditorGridPtr(editor, listArgs);
        if (grid == nullptr) {
            return;
        }

        if (newSize.width < 1
            || newSize.height < 1
            || newSize.width > ActionPolicy::MAX_SIZE.width
            || newSize.height > ActionPolicy::MAX_SIZE.height) {

            return;
        }

        if (newSize == grid->size()) {
            return;
        }

        editor->addAction(std::make_unique<EditGridAction>(
            editor, listArgs, grid->resized(newSize, ActionPolicy::DEFAULT_VALUE)));
    }
};

}
