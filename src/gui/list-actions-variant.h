/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "list-actions.h"
#include <variant>

namespace UnTech::Gui {

template <typename ActionPolicy>
class ListActionsVariant final : public ListActions<ActionPolicy> {
    using EditorT = typename ActionPolicy::EditorT;
    using EditorDataT = typename ActionPolicy::EditorDataT;
    using ListArgsT = typename ActionPolicy::ListArgsT;
    using SelectionT = typename ActionPolicy::SelectionT;
    using ListT = typename ActionPolicy::ListT;
    using index_type = typename ListT::size_type;
    using value_type = typename ListT::value_type;

private:
    template <auto FieldPtr>
    class EditVariantItemFieldAction final : public AbstractListActions<ActionPolicy>::BaseAction {
        static_assert(std::is_member_object_pointer_v<decltype(FieldPtr)>);

        using ClassT = typename member_class<decltype(FieldPtr)>::type;
        using FieldT = typename remove_member_pointer<decltype(FieldPtr)>::type;

    private:
        const index_type index;
        const FieldT newValue;
        // set by firstDo()
        FieldT oldValue;

        [[nodiscard]] ClassT& getEditorData(EditorT& editor) const
        {
            ListT& editorList = this->getEditorList(editor);
            assert(index < editorList.size());
            return std::get<ClassT>(editorList.at(index));
        }

        [[nodiscard]] ClassT& getProjectData(Project::ProjectFile& projectFile, EditorT& editor) const
        {
            ListT& projectList = this->getProjectList(projectFile, editor);
            assert(index < projectList.size());
            return std::get<ClassT>(projectList.at(index));
        }

    public:
        EditVariantItemFieldAction(const std::shared_ptr<EditorT>& editor,
                                   const ListArgsT& listArgs,
                                   const index_type index)
            : AbstractListActions<ActionPolicy>::BaseAction(editor, listArgs)
            , index(index)
            , newValue(getEditorData(*editor).*FieldPtr)
        {
        }
        virtual ~EditVariantItemFieldAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            if (auto e = this->getEditor()) {
                ClassT& pd = getProjectData(projectFile, *e);

                oldValue = pd.*FieldPtr;

                pd.*FieldPtr = newValue;

                return !(oldValue == newValue);
            }
            return false;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            if (auto e = this->getEditor()) {
                ClassT& ed = getEditorData(*e);
                ClassT& pd = getProjectData(projectFile, *e);

                ed.*FieldPtr = oldValue;
                pd.*FieldPtr = oldValue;
            }
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            if (auto e = this->getEditor()) {
                ClassT& ed = getEditorData(*e);
                ClassT& pd = getProjectData(projectFile, *e);

                ed.*FieldPtr = newValue;
                pd.*FieldPtr = newValue;
            }
        }
    };

public:
    template <auto FieldPtr>
    static void variantFieldEdited(const std::shared_ptr<EditorT>& editor, const ListArgsT& listArgs, const index_type index)
    {
        const ListT* list = ListActions<ActionPolicy>::getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (index < list->size()) {
            editor->undoStack().addAction(
                std::make_unique<EditVariantItemFieldAction<FieldPtr>>(editor, listArgs, index));
        }
    }

    template <auto FieldPtr>
    static void variantFieldEdited(const std::shared_ptr<EditorT>& editor, const index_type index)
    {
        const std::tuple<> listArgs = std::make_tuple();

        variantFieldEdited<FieldPtr>(editor, listArgs, index);
    }
};
}
