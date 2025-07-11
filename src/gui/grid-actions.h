﻿/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstract-editor.h"
#include "editor-actions-notify-gui.h"
#include "models/common/aabb.h"
#include "models/common/vectorset-upoint.h"
#include <gsl/gsl>

namespace UnTech::Gui {

template <typename ActionPolicy>
struct GridActions {
    using EditorT = typename ActionPolicy::EditorT;
    using EditorDataT = typename ActionPolicy::EditorDataT;
    using ListArgsT = typename ActionPolicy::ListArgsT;
    using GridT = typename ActionPolicy::GridT;

    using NotNullEditorPtr = gsl::not_null<std::shared_ptr<EditorT>>;

    [[nodiscard]] static const GridT* getEditorGridPtr(EditorT& editor, const ListArgsT& listArgs)
    {
        EditorDataT* data = ActionPolicy::getEditorData(editor);
        assert(data != nullptr);
        return std::apply(&ActionPolicy::getGrid,
                          std::tuple_cat(std::forward_as_tuple(*data), listArgs));
    }

    class BaseAction : public UndoAction {
    private:
        std::weak_ptr<EditorT> _editor;
        ListArgsT listArgs;

    public:
        ~BaseAction() override = default;

    protected:
        BaseAction(const NotNullEditorPtr& editor, const ListArgsT& listArgs)
            : _editor(editor.get())
            , listArgs(listArgs)
        {
        }

        [[nodiscard]] inline NotNullEditorPtr getEditor() const
        {
            auto e = _editor.lock();
            assert(e != nullptr);
            return e;
        }

        [[nodiscard]] GridT& getProjectGrid(Project::ProjectFile& projectFile, EditorT& editor) const
        {
            EditorDataT* data = ActionPolicy::getEditorData(projectFile, editor.itemIndex());
            assert(data != nullptr);
            GridT* grid = std::apply(&ActionPolicy::getGrid,
                                     std::tuple_cat(std::forward_as_tuple(*data), listArgs));
            assert(grid != nullptr);
            return *grid;
        }

        [[nodiscard]] GridT& getEditorGrid(EditorT& editor) const
        {
            EditorDataT* data = ActionPolicy::getEditorData(editor);
            assert(data != nullptr);
            GridT* grid = std::apply(&ActionPolicy::getGrid,
                                     std::tuple_cat(std::forward_as_tuple(*data), listArgs));
            assert(grid != nullptr);
            return *grid;
        }

        static void clearSelection(EditorT& editor)
        {
            upoint_vectorset& sel = editor.*(ActionPolicy::SelectionPtr);
            sel.clear();
        }

    public:
        void notifyGui(AbstractEditorGui* gui) const final
        {
            editorUndoAction_notifyGui<ActionPolicy>(gui);
        }
    };

    class EditGridAction final : public BaseAction {
    private:
        GridT newGrid;
        // set by firstDo()
        GridT oldGrid;

    public:
        EditGridAction(const NotNullEditorPtr& editor, const ListArgsT& listArgs, const GridT&& g)
            : BaseAction(std::move(editor), listArgs)
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
            auto e = this->getEditor();

            GridT& projectGrid = this->getProjectGrid(projectFile, *e);

            oldGrid = projectGrid;

            setCells(projectFile, newGrid);

            // operator!= may not implemented in a few of my structs
            return !(newGrid == oldGrid);
        }

        void setCells(Project::ProjectFile& projectFile, const GridT& grid) const
        {
            auto e = this->getEditor();

            GridT& projectGrid = this->getProjectGrid(projectFile, *e);
            GridT& editorGrid = this->getEditorGrid(*e);

            projectGrid = grid;
            editorGrid = grid;

            if (oldGrid.size() != newGrid.size()) {
                this->clearSelection(*e);
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
        upoint position;
        GridT newValues;
        // set by firstDo()
        GridT oldValues;

    public:
        EditMultipleCellsAction(const NotNullEditorPtr& editor, const ListArgsT& listArgs, upoint p, const GridT&& g)
            : BaseAction(std::move(editor), listArgs)
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
            auto e = this->getEditor();

            GridT& projectGrid = this->getProjectGrid(projectFile, *e);
            GridT& editorGrid = this->getEditorGrid(*e);

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
            auto e = this->getEditor();

            GridT& projectGrid = this->getProjectGrid(projectFile, *e);
            GridT& editorGrid = this->getEditorGrid(*e);

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

    static void gridTilesPlaced(const NotNullEditorPtr& editor, const urect r)
    {
        const ListArgsT listArgs;

        const GridT* grid = getEditorGridPtr(*editor, listArgs);
        if (grid == nullptr) {
            return;
        }

        if (r.width < 1 || r.height < 1
            || r.right() > grid->width() || r.bottom() > grid->height()) {

            return;
        }

        editor->undoStack().addAction(std::make_unique<EditMultipleCellsAction>(
            editor, listArgs, r.topLeft(), grid->subGrid(r)));
    }

    static void resizeGrid(const NotNullEditorPtr& editor, const usize newSize)
    {
        const ListArgsT listArgs;

        const GridT* grid = getEditorGridPtr(*editor, listArgs);
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

        editor->undoStack().addAction(std::make_unique<EditGridAction>(
            editor, listArgs, grid->resized(newSize, ActionPolicy::DEFAULT_VALUE)));
    }
};

}
