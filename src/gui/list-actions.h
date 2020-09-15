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

template <typename ListT, typename FieldT>
const FieldT* getListField(const ListT* list, const typename ListT::size_type index,
                           FieldT ListT::value_type::*const ptr)
{
    if (list) {
        if (index < list->size()) {
            return &(list->at(index).*ptr);
        }
    }
    return nullptr;
}

template <class ListT, class SelectionT>
std::vector<std::pair<typename ListT::size_type, typename ListT::value_type>>
indexesAndDataForMultipleSelection(const ListT& list, const SelectionT& sel)
{
    using size_type = typename ListT::size_type;
    using value_type = typename ListT::value_type;

    std::vector<std::pair<size_type, value_type>> values;

    const size_type end = std::min<size_type>(list.size(), SelectionT::MAX_SIZE);
    for (size_t i = 0; i < end; i++) {
        if (sel.isSelected(i)) {
            values.emplace_back(i, list.at(i));
        }
    }

    return values;
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

    protected:
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
                std::apply(&SelectionT::setSelected,
                           std::tuple_cat(std::forward_as_tuple(this->selection()), this->listArgs, std::make_tuple(index)));
            }
            else {
                // ::TODO update selection::
                this->selection().clearSelection();
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
            this->selection().clearSelection();
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
                this->selection().clearSelection();
            }
            else {
                // ::TODO update selection::
                this->selection().clearSelection();
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
            this->selection().clearSelection();
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

    class EditMultipleItemsAction final : public BaseAction {
    private:
        const std::vector<index_type> _indexes;
        std::vector<value_type> _newValues;
        std::vector<value_type> _oldValues;

    private:
        void setValues(Project::ProjectFile& projectFile, const std::vector<value_type>& values) const
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(_indexes.size() == values.size());

            auto it = values.begin();
            for (const index_type index : _indexes) {
                const value_type value = *it++;

                projectList.at(index) = value;
                editorList.at(index) = value;
            }
            assert(it == values.end());
        }

    public:
        EditMultipleItemsAction(EditorT* editor,
                                const ListArgsT& listArgs,
                                const std::vector<index_type>&& indexes)
            : BaseAction(editor, listArgs)
            , _indexes(std::move(indexes))
        {
        }
        virtual ~EditMultipleItemsAction() = default;

        virtual bool firstDo(Project::ProjectFile& projectFile) final
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(_oldValues.empty());
            assert(_newValues.empty());

            _oldValues.reserve(_indexes.size());
            _newValues.reserve(_indexes.size());

            bool changed = false;
            for (const index_type index : _indexes) {
                value_type& projectValue = projectList.at(index);
                const value_type& editorValue = editorList.at(index);

                _oldValues.push_back(projectValue);
                _newValues.push_back(editorValue);

                if (projectValue != editorValue) {
                    projectValue = editorValue;
                    changed = true;
                }
            }

            return changed;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            setValues(projectFile, _oldValues);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            setValues(projectFile, _newValues);
        }
    };

    template <auto FieldPtr>
    class EditAllItemsInListFieldAction final : public BaseAction {
        using FieldT = typename remove_member_pointer<decltype(FieldPtr)>::type;

    private:
        std::vector<FieldT> _newValues;
        std::vector<FieldT> _oldValues;

    private:
        void setValues(Project::ProjectFile& projectFile, const std::vector<FieldT>& values) const
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(projectList.size() == values.size());
            assert(editorList.size() == values.size());

            auto pIt = projectList.begin();
            auto eIt = editorList.begin();
            for (const FieldT& value : values) {
                (*eIt++).*FieldPtr = value;
                (*pIt++).*FieldPtr = value;
            }
            assert(pIt == projectList.end());
            assert(eIt == editorList.end());
        }

    public:
        EditAllItemsInListFieldAction(EditorT* editor,
                                      const ListArgsT& listArgs)
            : BaseAction(editor, listArgs)
        {
        }
        virtual ~EditAllItemsInListFieldAction() = default;

        virtual bool firstDo(Project::ProjectFile& projectFile) final
        {
            const ListT& projectList = this->getProjectList(projectFile);
            const ListT& editorList = this->getEditorList();

            assert(projectList.size() == editorList.size());
            assert(_oldValues.empty());
            assert(_newValues.empty());

            _oldValues.reserve(projectList.size());
            for (const auto& item : projectList) {
                _oldValues.push_back(item.*FieldPtr);
            }

            _newValues.reserve(editorList.size());
            for (const auto& item : editorList) {
                _newValues.push_back(item.*FieldPtr);
            }

            return _oldValues != _newValues;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            setValues(projectFile, _oldValues);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            setValues(projectFile, _newValues);
        }
    };

    class EditMultipleNestedItems final : public EditorUndoAction {
    public:
        struct IndexAndValues {
            ListArgsT listArgs;
            std::vector<std::pair<index_type, value_type>> childIndexesAndValues;
        };

    private:
        EditorT* const editor;
        const std::vector<IndexAndValues> indexesAndNewValues;
        std::vector<value_type> oldValues;

    private:
        static ListT& getList(EditorDataT* data, const ListArgsT& listArgs)
        {
            assert(data != nullptr);
            ListT* list = std::apply(&ActionPolicy::getList,
                                     std::tuple_cat(std::forward_as_tuple(*data), listArgs));
            assert(list != nullptr);
            return *list;
        }

    public:
        EditMultipleNestedItems(EditorT* editor,
                                const std::vector<IndexAndValues>&& indexesAndValues)
            : EditorUndoAction()
            , editor(editor)
            , indexesAndNewValues(std::move(indexesAndValues))
        {
        }
        virtual ~EditMultipleNestedItems() = default;

        virtual bool firstDo(Project::ProjectFile& projectFile) final
        {
            EditorDataT* projectData = ActionPolicy::getEditorData(projectFile, editor->itemIndex());
            EditorDataT* editorData = ActionPolicy::getEditorData(*editor);
            assert(projectData);
            assert(editorData);

            assert(oldValues.empty());

            bool changed = false;

            for (const IndexAndValues& childValues : indexesAndNewValues) {
                ListT& projectList = getList(projectData, childValues.listArgs);
                const ListT& editorList = getList(editorData, childValues.listArgs);

                assert(projectList.size() == editorList.size());

                for (const auto& [index, editorValue] : childValues.childIndexesAndValues) {
                    assert(index >= 0 && index < projectList.size());

                    value_type& projectValue = projectList.at(index);

                    oldValues.push_back(projectValue);

                    changed |= projectValue != editorValue;
                    projectValue = editorValue;
                }
            }

            return changed;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            EditorDataT* projectData = ActionPolicy::getEditorData(projectFile, editor->itemIndex());
            EditorDataT* editorData = ActionPolicy::getEditorData(*editor);
            assert(projectData);
            assert(editorData);

            auto it = oldValues.begin();

            for (const IndexAndValues& childValues : indexesAndNewValues) {
                ListT& projectList = getList(projectData, childValues.listArgs);
                ListT& editorList = getList(editorData, childValues.listArgs);

                assert(projectList.size() == editorList.size());

                for (const auto& civ : childValues.childIndexesAndValues) {
                    const index_type index = civ.first;
                    assert(index >= 0 && index < projectList.size());

                    const value_type& oldValue = *it++;

                    projectList.at(index) = oldValue;
                    editorList.at(index) = oldValue;
                }
            }
            assert(it == oldValues.end());
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            EditorDataT* projectData = ActionPolicy::getEditorData(projectFile, editor->itemIndex());
            EditorDataT* editorData = ActionPolicy::getEditorData(*editor);
            assert(projectData);
            assert(editorData);

            for (const IndexAndValues& childValues : indexesAndNewValues) {
                ListT& projectList = getList(projectData, childValues.listArgs);
                ListT& editorList = getList(editorData, childValues.listArgs);

                assert(projectList.size() == editorList.size());

                for (const auto& [index, newValue] : childValues.childIndexesAndValues) {
                    assert(index >= 0 && index < projectList.size());

                    projectList.at(index) = newValue;
                    editorList.at(index) = newValue;
                }
            }
        }
    };

private:
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
            const index_type i = sel.selectedIndex();
            if (i < list->size()) {
                editor->addAction(
                    std::make_unique<RemoveAction>(editor, listArgs, i, list->at(i)));
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
            auto values = indexesAndDataForMultipleSelection(*list, sel);
            if (!values.empty()) {
                editor->addAction(
                    std::make_unique<RemoveMultipleAction>(editor, listArgs, std::move(values)));
            }
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
            auto values = indexesAndDataForMultipleSelection(*list, sel);
            if (!values.empty()) {
                editor->addAction(
                    std::make_unique<RemoveMultipleAction>(editor, listArgs, std::move(values)));
            }
        } break;
        }
    }

    template <typename T = SelectionT>
    static std::enable_if_t<std::is_same_v<T, GroupMultipleSelection>>
    editList(EditorT* editor, EditListAction action)
    {
        using ParentActionPolicy = typename ActionPolicy::ParentActionPolicy;

        static_assert(ParentActionPolicy::MAX_SIZE <= GroupMultipleSelection::MAX_GROUP_SIZE);
        static_assert(std::is_same_v<ListArgsT, std::tuple<unsigned>>);

        const GroupMultipleSelection& sel = getSelection(editor);

        switch (action) {
        case EditListAction::ADD: {
            // Can only add an item to a group if it is selected
            const SingleSelection& parentSel = editor->*ParentActionPolicy::SelectionPtr;
            const ListArgsT listArgs = std::make_tuple(parentSel.selectedIndex());
            if (const ListT* list = getEditorListPtr(editor, listArgs)) {
                editor->addAction(
                    std::make_unique<AddAction>(editor, listArgs, list->size()));
            }
        } break;

        case EditListAction::REMOVE: {
            editor->startMacro();
            for (unsigned groupIndex = 0; groupIndex < sel.MAX_GROUP_SIZE; groupIndex++) {
                const ListArgsT listArgs = std::make_tuple(groupIndex);
                if (const ListT* list = getEditorListPtr(editor, listArgs)) {
                    auto& childSel = sel.childSel(groupIndex);
                    auto values = indexesAndDataForMultipleSelection(*list, childSel);
                    if (!values.empty()) {
                        editor->addAction(
                            std::make_unique<RemoveMultipleAction>(editor, listArgs, std::move(values)));
                    }
                }
            }
            editor->endMacro();
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

        _editListItem(editor, listArgs, sel.selectedIndex());
    }

    // Only enable if ListArgsT is not empty
    template <typename LA_ = ListArgsT, typename = std::enable_if_t<!std::is_same_v<LA_, std::tuple<>>>>
    static void selectedListItemEdited(EditorT* editor, const index_type index)
    {
        const SelectionT& sel = getSelection(editor);
        const ListArgsT listArgs = sel.listArgs();

        _editListItem(editor, listArgs, index);
    }

    // ::TODO replace indexes with 256 bit bitfield::
    static void selectedListItemsEdited(EditorT* editor, std::vector<index_type> indexes)
    {
        const SelectionT& sel = getSelection(editor);
        const ListArgsT listArgs = sel.listArgs();

        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        indexes.erase(std::remove_if(indexes.begin(), indexes.end(),
                                     [&](auto i) { return i >= list->size(); }),
                      indexes.end());

        if (!indexes.empty()) {
            editor->addAction(
                std::make_unique<EditMultipleItemsAction>(editor, listArgs, std::move(indexes)));
        }
    }

    // ::TODO find some way to simplify this mess::
    template <typename ST_ = SelectionT>
    static std::enable_if_t<std::is_same_v<ST_, MultipleSelection> || std::is_same_v<ST_, MultipleChildSelection>>
    selectedItemsEdited(EditorT* editor)
    {
        const SelectionT& sel = getSelection(editor);
        const ListArgsT listArgs = sel.listArgs();

        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        const index_type end = std::min<index_type>(list->size(), SelectionT::MAX_SIZE);

        std::vector<index_type> indexes;
        indexes.reserve(end);

        for (size_t i = 0; i < end; i++) {
            if (sel.isSelected(i)) {
                indexes.push_back(i);
            }
        }

        if (!indexes.empty()) {
            editor->addAction(
                std::make_unique<EditMultipleItemsAction>(editor, listArgs, std::move(indexes)));
        }
    }

    template <typename ST_ = SelectionT>
    static std::enable_if_t<std::is_same_v<ST_, GroupMultipleSelection>>
    selectedItemsEdited(EditorT* editor)
    {
        const GroupMultipleSelection& sel = getSelection(editor);

        std::vector<typename EditMultipleNestedItems::IndexAndValues> values;

        for (unsigned groupIndex = 0; groupIndex < sel.MAX_GROUP_SIZE; groupIndex++) {
            const ListArgsT listArgs = std::make_tuple(groupIndex);
            if (const ListT* list = getEditorListPtr(editor, listArgs)) {
                const auto& childSel = sel.childSel(groupIndex);
                auto childIndexesAndValues = indexesAndDataForMultipleSelection(*list, childSel);

                if (!childIndexesAndValues.empty()) {
                    values.push_back({ groupIndex, std::move(childIndexesAndValues) });
                }
            }
        }
        if (!values.empty()) {
            editor->addAction(
                std::make_unique<EditMultipleNestedItems>(editor, std::move(values)));
        }
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

        _editListField<FieldPtr>(editor, listArgs, sel.selectedIndex());
    }

    template <auto FieldPtr, typename ST_ = SelectionT, typename = std::enable_if_t<std::is_same_v<ST_, MultipleChildSelection>>>
    static void selectedItemFieldEdited(EditorT* editor, const index_type index)
    {
        const SelectionT& sel = getSelection(editor);
        const std::tuple<unsigned> listArgs = sel.listArgs();

        _editListField<FieldPtr>(editor, listArgs, index);
    }

    template <auto FieldPtr>
    static void allItemsInSelectedListFieldEdited(EditorT* editor)
    {
        const SelectionT& sel = getSelection(editor);
        const ListArgsT listArgs = sel.listArgs();

        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }
        if (list->empty()) {
            return;
        }

        editor->addAction(
            std::make_unique<EditAllItemsInListFieldAction<FieldPtr>>(editor, listArgs));
    }
};

}
