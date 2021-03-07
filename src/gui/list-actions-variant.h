/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "list-actions.h"
#include <variant>

namespace UnTech::Gui {

template <typename ActionPolicy>
struct ListActionsVariant {
    using EditorT = typename ActionPolicy::EditorT;
    using EditorDataT = typename ActionPolicy::EditorDataT;
    using ListArgsT = typename ActionPolicy::ListArgsT;
    using SelectionT = typename ActionPolicy::SelectionT;
    using ListT = typename ActionPolicy::ListT;
    using index_type = typename ListT::size_type;
    using value_type = typename ListT::value_type;

    template <auto FieldPtr, typename = std::enable_if_t<std::is_member_object_pointer_v<decltype(FieldPtr)>>>
    class EditVariantItemFieldAction final : public ListActions<ActionPolicy>::BaseAction {
        using ClassT = typename member_class<decltype(FieldPtr)>::type;
        using FieldT = typename remove_member_pointer<decltype(FieldPtr)>::type;

    private:
        const index_type index;

        // set by firstDo()
        FieldT newValue;
        FieldT oldValue;

        std::tuple<ClassT&, ClassT&> getProjectAndEditorData(Project::ProjectFile& projectFile) const
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(projectList.size() == editorList.size());
            assert(index < projectList.size());

            return { std::get<ClassT>(projectList.at(index)),
                     std::get<ClassT>(editorList.at(index)) };
        }

    public:
        EditVariantItemFieldAction(EditorT* editor,
                                   const ListArgsT& listArgs,
                                   const index_type index)
            : ListActions<ActionPolicy>::BaseAction(editor, listArgs)
            , index(index)
        {
        }
        virtual ~EditVariantItemFieldAction() = default;

        virtual bool firstDo(Project::ProjectFile& projectFile) final
        {
            auto [pd, ed] = getProjectAndEditorData(projectFile);

            oldValue = pd.*FieldPtr;
            newValue = ed.*FieldPtr;

            pd.*FieldPtr = newValue;

            return !(oldValue == newValue);
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            auto [pd, ed] = getProjectAndEditorData(projectFile);

            ed.*FieldPtr = oldValue;
            ed.*FieldPtr = oldValue;
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            auto [pd, ed] = getProjectAndEditorData(projectFile);

            ed.*FieldPtr = newValue;
            ed.*FieldPtr = newValue;
        }
    };

    template <auto FieldPtr>
    static void variantFieldEdited(EditorT* editor, const ListArgsT& listArgs, const index_type index)
    {
        const ListT* list = ListActions<ActionPolicy>::getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (index < list->size()) {
            editor->addAction(
                std::make_unique<EditVariantItemFieldAction<FieldPtr>>(editor, listArgs, index));
        }
    }

    template <auto FieldPtr, typename LA_ = ListArgsT, typename = std::enable_if_t<std::is_same_v<LA_, std::tuple<>>>>
    static void variantFieldEdited(EditorT* editor, const index_type index)
    {
        const std::tuple<> listArgs = std::make_tuple();

        variantFieldEdited<FieldPtr>(editor, listArgs, index);
    }
};
}
