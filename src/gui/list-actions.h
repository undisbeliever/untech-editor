/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstract-editor.h"
#include "list-helpers.h"
#include "selection.h"
#include "gui/common/type-traits.h"

namespace UnTech::Gui {

template <typename ListT>
typename ListT::value_type* getListItem(ListT* list, const typename ListT::size_type index)
{
    if (list) {
        if (index < list->size()) {
            return &list->at(index);
        }
    }
    return nullptr;
}

template <typename ListT, typename FieldT>
FieldT* getListField(ListT* list, const typename ListT::size_type index,
                     FieldT ListT::value_type::*const ptr)
{
    if (list) {
        if (index < list->size()) {
            return &(list->at(index).*ptr);
        }
    }
    return nullptr;
}

template <typename ActionPolicy>
struct ListActions {
    using EditorT = typename ActionPolicy::EditorT;
    using EditorDataT = typename ActionPolicy::EditorDataT;
    using ListArgsT = typename ActionPolicy::ListArgsT;
    using SelectionT = typename ActionPolicy::SelectionT;
    using ListT = typename ActionPolicy::ListT;
    using index_type = typename ListT::size_type;
    using value_type = typename ListT::value_type;

    constexpr static index_type MAX_SIZE = ActionPolicy::MAX_SIZE;

    static_assert(MAX_SIZE <= SelectionT::MAX_SIZE);

    static const ListT* getEditorListPtr(EditorT* editor, const ListArgsT& listArgs)
    {
        EditorDataT* data = ActionPolicy::getEditorData(*editor);
        assert(data != nullptr);
        return std::apply(&ActionPolicy::getList,
                          std::tuple_cat(std::forward_as_tuple(*data), listArgs));
    }

    static const SelectionT& getSelection(const EditorT* editor)
    {
        return editor->*ActionPolicy::SelectionPtr;
    }

    class BaseAction : public EditorUndoAction {
    private:
        EditorT* const editor;
        const ListArgsT listArgs;

    protected:
        BaseAction(EditorT* editor,
                   const ListArgsT& listArgs)
            : editor(editor)
            , listArgs(listArgs)
        {
            assert(editor != nullptr);
        }
        virtual ~BaseAction() = default;

        ListT& getProjectList(Project::ProjectFile& projectFile) const
        {
            EditorDataT* data = ActionPolicy::getEditorData(projectFile, editor->itemIndex());
            assert(data != nullptr);
            ListT* list = std::apply(&ActionPolicy::getList,
                                     std::tuple_cat(std::forward_as_tuple(*data), listArgs));
            assert(list != nullptr);
            return *list;
        }

        ListT& getEditorList() const
        {
            EditorDataT* data = ActionPolicy::getEditorData(*editor);
            assert(data != nullptr);
            ListT* list = std::apply(&ActionPolicy::getList,
                                     std::tuple_cat(std::forward_as_tuple(*data), listArgs));
            assert(list != nullptr);
            return *list;
        }

        SelectionT& selection() const
        {
            return editor->*ActionPolicy::SelectionPtr;
        }
    };

    class AddRemoveAction : public BaseAction {
    private:
        const index_type index;
        const value_type value;

    public:
        AddRemoveAction(EditorT* editor,
                        const ListArgsT& listArgs,
                        const index_type index,
                        const value_type& value)
            : BaseAction(editor, listArgs)
            , index(index)
            , value(value)
        {
        }
        virtual ~AddRemoveAction() = default;

    protected:
        void addItem(Project::ProjectFile& projectFile, bool first = false) const
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(projectList.size() == editorList.size());

            assert(index <= projectList.size());
            projectList.insert(projectList.begin() + index, value);
            editorList.insert(editorList.begin() + index, value);

            if (first) {
                // ::TODO set selection::
                this->selection().selected = SelectionT::NO_SELECTION;
            }
            else {
                // ::TODO update selection::
                this->selection().selected = SelectionT::NO_SELECTION;
            }
        }

        void removeItem(Project::ProjectFile& projectFile) const
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(projectList.size() == editorList.size());

            assert(index < projectList.size());
            projectList.erase(projectList.begin() + index);
            editorList.erase(editorList.begin() + index);

            // ::TODO update selection::
            this->selection().selected = SelectionT::NO_SELECTION;
        }
    };

    class AddAction final : public AddRemoveAction {
    public:
        AddAction(EditorT* editor,
                  const ListArgsT& listArgs,
                  const index_type index)
            : AddRemoveAction(editor, listArgs, index, value_type())
        {
        }

        AddAction(EditorT* editor,
                  const ListArgsT& listArgs,
                  const index_type index,
                  const value_type& value)
            : AddRemoveAction(editor, listArgs, index, value)
        {
        }
        virtual ~AddAction() = default;

        virtual bool firstDo(Project::ProjectFile& projectFile) final
        {
            this->addItem(projectFile, true);
            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            this->removeItem(projectFile);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            this->addItem(projectFile, false);
        }
    };

    class RemoveAction final : public AddRemoveAction {
    public:
        RemoveAction(EditorT* editor,
                     const ListArgsT& listArgs,
                     const index_type index,
                     const value_type& value)
            : AddRemoveAction(editor, listArgs, index, value)
        {
        }
        virtual ~RemoveAction() = default;

        virtual bool firstDo(Project::ProjectFile& projectFile) final
        {
            this->removeItem(projectFile);
            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            this->addItem(projectFile, false);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            this->removeItem(projectFile);
        }
    };

    class AddRemoveMultipleAction : public BaseAction {
    private:
        const std::vector<std::pair<index_type, value_type>> _values;

    public:
        // values vector must be sorted by index_type and not contain any duplicate indexes
        AddRemoveMultipleAction(EditorT* editor,
                                const ListArgsT& listArgs,
                                const std::vector<std::pair<index_type, value_type>>&& values)
            : BaseAction(editor, listArgs)
            , _values(std::move(values))
        {
        }
        virtual ~AddRemoveMultipleAction() = default;

    protected:
        void addItems(Project::ProjectFile& projectFile, bool first = false) const
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(projectList.size() == editorList.size());

            for (const auto& [index, value] : _values) {
                assert(index >= 0 && index <= projectList.size());

                projectList.insert(projectList.begin() + index, value);
                editorList.insert(editorList.begin() + index, value);
            }

            if (first) {
                // ::TODO set selection::
                this->selection().selected = SelectionT::NO_SELECTION;
            }
            else {
                // ::TODO update selection::
                this->selection().selected = SelectionT::NO_SELECTION;
            }
        }

        void removeItems(Project::ProjectFile& projectFile) const
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(projectList.size() == editorList.size());

            for (auto it = _values.rbegin(); it != _values.rend(); it++) {
                const index_type& index = it->first;
                assert(index >= 0 && index < projectList.size());

                projectList.erase(projectList.begin() + index);
                editorList.erase(editorList.begin() + index);
            }

            // ::TODO update selection::
            this->selection().selected = SelectionT::NO_SELECTION;
        }
    };

    class AddMultipleAction final : public AddRemoveMultipleAction {
    public:
        // values vector must be sorted by index_type and not contain any duplicate indexes
        AddMultipleAction(EditorT* editor,
                          const ListArgsT& listArgs,
                          const std::vector<std::pair<index_type, value_type>>&& values)
            : AddRemoveMultipleAction(editor, listArgs, std::move(values))
        {
        }
        virtual ~AddMultipleAction() = default;

        virtual bool firstDo(Project::ProjectFile& projectFile) final
        {
            this->addItems(projectFile, true);
            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            this->removeItems(projectFile);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            this->addItems(projectFile, false);
        }
    };

    class RemoveMultipleAction final : public AddRemoveMultipleAction {
    public:
        // values vector must be sorted by index_type and not contain any duplicate indexes
        RemoveMultipleAction(EditorT* editor,
                             const ListArgsT& listArgs,
                             const std::vector<std::pair<index_type, value_type>>&& values)
            : AddRemoveMultipleAction(editor, listArgs, std::move(values))
        {
        }
        virtual ~RemoveMultipleAction() = default;

        virtual bool firstDo(Project::ProjectFile& projectFile) final
        {
            this->removeItems(projectFile);
            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            this->addItems(projectFile, false);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            this->removeItems(projectFile);
        }
    };

    class EditItemAction final : public BaseAction {
    private:
        const index_type index;

        // set by firstDo()
        value_type newValue;
        value_type oldValue;

    public:
        EditItemAction(EditorT* editor,
                       const ListArgsT& listArgs,
                       const index_type index)
            : BaseAction(editor, listArgs)
            , index(index)
        {
        }
        virtual ~EditItemAction() = default;

        virtual bool firstDo(Project::ProjectFile& projectFile) final
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(projectList.size() == editorList.size());
            assert(index < projectList.size());

            oldValue = projectList.at(index);
            newValue = editorList.at(index);

            projectList.at(index) = newValue;

            // operator!= may not implemented in a few of my structs
            return !(oldValue == newValue);
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(projectList.size() == editorList.size());
            assert(index < projectList.size());

            projectList.at(index) = oldValue;
            editorList.at(index) = oldValue;
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(projectList.size() == editorList.size());
            assert(index < projectList.size());

            projectList.at(index) = newValue;
            editorList.at(index) = newValue;
        }
    };

    template <auto FieldPtr, typename = std::enable_if_t<std::is_member_object_pointer_v<decltype(FieldPtr)>>>
    class EditItemFieldAction final : public BaseAction {
        using FieldT = typename remove_member_pointer<decltype(FieldPtr)>::type;

    private:
        const index_type index;

        // set by firstDo()
        FieldT newValue;
        FieldT oldValue;

    public:
        EditItemFieldAction(EditorT* editor,
                            const ListArgsT& listArgs,
                            const index_type index)
            : BaseAction(editor, listArgs)
            , index(index)
        {
        }
        virtual ~EditItemFieldAction() = default;

        virtual bool firstDo(Project::ProjectFile& projectFile) final
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(projectList.size() == editorList.size());
            assert(index < projectList.size());

            oldValue = projectList.at(index).*FieldPtr;
            newValue = editorList.at(index).*FieldPtr;

            projectList.at(index).*FieldPtr = newValue;

            return oldValue != newValue;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(projectList.size() == editorList.size());
            assert(index < projectList.size());

            projectList.at(index).*FieldPtr = oldValue;
            editorList.at(index).*FieldPtr = oldValue;
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(projectList.size() == editorList.size());
            assert(index < projectList.size());

            projectList.at(index).*FieldPtr = newValue;
            editorList.at(index).*FieldPtr = newValue;
        }
    };

private:
    static void _removeMultipleBitfieldSelection(EditorT* editor, const ListArgsT& listArgs,
                                                 const ListT& list, const uint64_t selected)
    {
        if (selected == 0
            || list.empty()) {

            return;
        }

        std::vector<std::pair<index_type, value_type>> values;

        const index_type end = std::min<index_type>(list.size(), SelectionT::MAX_SIZE);
        for (size_t i = 0; i < end; i++) {
            if (selected & (uint64_t(1) << i)) {
                values.emplace_back(i, list.at(i));
            }
        }

        if (!values.empty()) {
            editor->addAction(
                std::make_unique<RemoveMultipleAction>(editor, listArgs, std::move(values)));
        }
    }

    static void _editListItem(EditorT* editor, const ListArgsT& listArgs, const index_type index)
    {
        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (index < list->size()) {
            editor->addAction(
                std::make_unique<EditItemAction>(editor, listArgs, index));
        }
    }

    template <auto FieldPtr>
    static void _editListField(EditorT* editor, const ListArgsT& listArgs, const index_type index)
    {
        editor->addAction(
            std::make_unique<EditItemFieldAction<FieldPtr>>(
                editor, listArgs, index));
    }

public:
    template <typename T = SelectionT>
    static std::enable_if_t<std::is_same_v<T, SingleSelection>>
    editList(EditorT* editor, EditListAction action)
    {
        const SingleSelection& sel = getSelection(editor);
        const ListArgsT listArgs = sel.listArgs();

        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        switch (action) {
        case EditListAction::ADD: {
            if (list->size() < MAX_SIZE) {
                editor->addAction(
                    std::make_unique<AddAction>(editor, listArgs, list->size()));
            }
        } break;

        case EditListAction::REMOVE: {
            if (sel.selected < list->size()) {
                editor->addAction(
                    std::make_unique<RemoveAction>(editor, listArgs, sel.selected, list->at(sel.selected)));
            }
        } break;
        }
    }

    template <typename T = SelectionT>
    static std::enable_if_t<std::is_same_v<T, MultipleSelection>>
    editList(EditorT* editor, EditListAction action)
    {
        const MultipleSelection& sel = getSelection(editor);
        const ListArgsT listArgs = sel.listArgs();

        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        switch (action) {
        case EditListAction::ADD: {
            if (list->size() < MAX_SIZE) {
                editor->addAction(
                    std::make_unique<AddAction>(editor, listArgs, list->size()));
            }
        } break;

        case EditListAction::REMOVE: {
            _removeMultipleBitfieldSelection(editor, listArgs, *list, sel.selected);
        } break;
        }
    }

    template <typename T = SelectionT>
    static std::enable_if_t<std::is_same_v<T, MultipleChildSelection>>
    editList(EditorT* editor, EditListAction action)
    {
        const MultipleChildSelection& sel = getSelection(editor);
        const ListArgsT listArgs = sel.listArgs();

        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        switch (action) {
        case EditListAction::ADD: {
            if (list->size() < MAX_SIZE) {
                editor->addAction(
                    std::make_unique<AddAction>(editor, listArgs, list->size()));
            }
        } break;

        case EditListAction::REMOVE: {
            _removeMultipleBitfieldSelection(editor, listArgs, *list, sel.selected);
        } break;
        }
    }

    template <typename LA_ = ListArgsT, typename = std::enable_if_t<std::is_same_v<LA_, std::tuple<>>>>
    static void itemEdited(EditorT* editor, const index_type index)
    {
        const std::tuple<> listArgs = std::make_tuple();
        _editListItem(editor, listArgs, index);
    }

    template <typename LA_ = ListArgsT, typename = std::enable_if_t<std::is_same_v<LA_, std::tuple<unsigned>>>>
    static void itemEdited(EditorT* editor, const unsigned parentIndex, const index_type index)
    {
        const std::tuple<unsigned> listArgs = std::make_tuple(parentIndex);
        _editListItem(editor, listArgs, index);
    }

    template <typename ST_ = SelectionT, typename = std::enable_if_t<std::is_same_v<ST_, SingleSelection>>>
    static void selectedItemEdited(EditorT* editor)
    {
        const SingleSelection& sel = getSelection(editor);
        const std::tuple<> listArgs = sel.listArgs();

        _editListItem(editor, listArgs, sel.selected);
    }

    // Only enable if ListArgsT is not empty
    template <typename LA_ = ListArgsT, typename = std::enable_if_t<!std::is_same_v<LA_, std::tuple<>>>>
    static void selectedListItemEdited(EditorT* editor, const index_type index)
    {
        const SelectionT& sel = getSelection(editor);
        const ListArgsT listArgs = sel.listArgs();

        _editListItem(editor, listArgs, index);
    }

    template <auto FieldPtr, typename LA_ = ListArgsT, typename = std::enable_if_t<std::is_same_v<LA_, std::tuple<>>>>
    static void fieldEdited(EditorT* editor, const index_type index)
    {
        const std::tuple<> listArgs = std::make_tuple();

        _editListField<FieldPtr>(editor, listArgs, index);
    }

    template <auto FieldPtr, typename LA_ = ListArgsT, typename = std::enable_if_t<std::is_same_v<LA_, std::tuple<unsigned>>>>
    static void fieldEdited(EditorT* editor, const unsigned parentIndex, const index_type index)
    {
        const std::tuple<unsigned> listArgs = std::make_tuple(parentIndex);

        _editListField<FieldPtr>(editor, listArgs, index);
    }

    template <auto FieldPtr, typename ST_ = SelectionT, typename = std::enable_if<std::is_same_v<ST_, SingleSelection>>>
    static void selectedFieldEdited(EditorT* editor)
    {
        const SingleSelection& sel = getSelection(editor);
        const std::tuple<> listArgs = sel.listArgs();

        _editListField<FieldPtr>(editor, listArgs, sel.selected);
    }

    template <auto FieldPtr, typename LA_ = ListArgsT, typename = std::enable_if<std::is_same_v<LA_, std::tuple<unsigned>>>>
    static void selectedItemFieldEdited(EditorT* editor, const index_type index)
    {
        const SelectionT& sel = getSelection(editor);
        const std::tuple<unsigned> listArgs = sel.listArgs();

        _editListField<FieldPtr>(editor, listArgs, index);
    }
};

}
