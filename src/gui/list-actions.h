/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstract-editor.h"
#include "selection.h"
#include "models/common/iterators.h"
#include "models/common/type-traits.h"
#include <algorithm>

namespace UnTech::Gui {

enum EditListAction {
    ADD,
    CLONE,
    REMOVE,
    RAISE_TO_TOP,
    RAISE,
    LOWER,
    LOWER_TO_BOTTOM,
};

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
std::vector<typename ListT::size_type>
indexesForMultipleSelection(const ListT& list, const SelectionT& sel)
{
    using size_type = typename ListT::size_type;

    std::vector<size_type> values;
    const size_type end = std::min<size_type>(list.size(), SelectionT::MAX_SIZE);
    for (const auto i : range(end)) {
        if (sel.isSelected(i)) {
            values.emplace_back(i);
        }
    }
    return values;
}

template <class ListT, class SelectionT>
std::vector<std::pair<typename ListT::size_type, typename ListT::value_type>>
indexesAndDataForMultipleSelection(const ListT& list, const SelectionT& sel)
{
    using size_type = typename ListT::size_type;
    using value_type = typename ListT::value_type;

    std::vector<std::pair<size_type, value_type>> values;

    const size_type end = std::min<size_type>(list.size(), SelectionT::MAX_SIZE);
    for (const auto i : range(end)) {
        if (sel.isSelected(i)) {
            values.emplace_back(i, list.at(i));
        }
    }

    return values;
}

template <typename ActionPolicy>
struct AbstractListActions {
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

    private:
        SelectionT& selection() const
        {
            return editor->*ActionPolicy::SelectionPtr;
        }

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

        void updateSelection_setSelected(index_type index) const
        {
            std::apply(&SelectionT::setSelected,
                       std::tuple_cat(std::forward_as_tuple(selection()), this->listArgs, std::make_tuple(index)));
        }

        void updateSelection_clearSelection() const
        {
            selection().clearSelection();
        }

        void updateSelection_appendSelection(index_type index) const
        {
            auto f = [&](auto... args) {
                selection().appendSelection(args...);
            };
            std::apply(f, std::tuple_cat(this->listArgs, std::make_tuple(index)));
        }

        void updateSelection_itemAdded(index_type index) const
        {
            std::apply(&SelectionT::itemAdded,
                       std::tuple_cat(std::forward_as_tuple(selection()), this->listArgs, std::make_tuple(index)));
        }

        void updateSelection_itemRemoved(index_type index) const
        {
            std::apply(&SelectionT::itemRemoved,
                       std::tuple_cat(std::forward_as_tuple(selection()), this->listArgs, std::make_tuple(index)));
        }

        void updateSelection_itemMoved(index_type from, index_type to) const
        {
            std::apply(&SelectionT::itemMoved,
                       std::tuple_cat(std::forward_as_tuple(selection()), this->listArgs, std::make_tuple(from, to)));
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
        void addItem(ListT& list) const
        {
            assert(index <= list.size());
            list.insert(list.begin() + index, value);
        }

        void removeItem(ListT& list) const
        {
            assert(index < list.size());
            list.erase(list.begin() + index);
        }

        void updateSelection_ItemAdded_first() const
        {
            this->updateSelection_setSelected(index);
        }

        void updateSelection_ItemAdded() const
        {
            this->updateSelection_itemAdded(index);
        }

        void updateSelection_ItemRemoved() const
        {
            this->updateSelection_itemRemoved(index);
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

        virtual void firstDo_editorData() const final
        {
            this->addItem(this->getEditorList());

            this->updateSelection_ItemAdded_first();
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            this->addItem(this->getProjectList(projectFile));

            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            this->removeItem(this->getEditorList());
            this->removeItem(this->getProjectList(projectFile));

            this->updateSelection_ItemRemoved();
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            this->addItem(this->getEditorList());
            this->addItem(this->getProjectList(projectFile));

            this->updateSelection_ItemAdded();
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

        virtual void firstDo_editorData() const final
        {
            this->removeItem(this->getEditorList());

            this->updateSelection_ItemRemoved();
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            this->removeItem(this->getProjectList(projectFile));

            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            this->addItem(this->getEditorList());
            this->addItem(this->getProjectList(projectFile));

            this->updateSelection_ItemAdded();
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            this->removeItem(this->getEditorList());
            this->removeItem(this->getProjectList(projectFile));

            this->updateSelection_ItemRemoved();
        }
    };

    class MoveAction final : public BaseAction {
    private:
        const index_type fromIndex;
        const index_type toIndex;

        void moveItem(Project::ProjectFile& projectFile, index_type from, index_type to) const
        {
            ListT& projectList = this->getProjectList(projectFile);
            ListT& editorList = this->getEditorList();

            assert(projectList.size() == editorList.size());

            assert(from < projectList.size());
            assert(to < projectList.size());

            moveListItem(from, to, projectList);
            moveListItem(from, to, editorList);

            this->updateSelection_itemMoved(from, to);
        }

    public:
        MoveAction(EditorT* editor,
                   const ListArgsT& listArgs,
                   const index_type from,
                   const index_type to)
            : BaseAction(editor, listArgs)
            , fromIndex(from)
            , toIndex(to)
        {
        }
        virtual ~MoveAction() = default;

        virtual void firstDo_editorData() const final
        {
            moveListItem(fromIndex, toIndex, this->getEditorList());

            this->updateSelection_itemMoved(fromIndex, toIndex);
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            moveListItem(fromIndex, toIndex, this->getProjectList(projectFile));

            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            moveItem(projectFile, toIndex, fromIndex);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            moveItem(projectFile, fromIndex, toIndex);
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
        void addItems(ListT& list) const
        {
            for (const auto& [index, value] : _values) {
                assert(index >= 0 && index <= list.size());

                list.insert(list.begin() + index, value);
            }
        }

        void removeItems(ListT& list) const
        {
            for (const auto& [index, value] : reverse(_values)) {
                assert(index >= 0 && index < list.size());

                list.erase(list.begin() + index);
            }
        }

        void updateSelection_setSelection() const
        {
            this->updateSelection_clearSelection();
            for (const auto& [index, value] : _values) {
                this->updateSelection_appendSelection(index);
            }
        }

        void updateSelection_ItemsAdded() const
        {
            for (const auto& [index, value] : _values) {
                this->updateSelection_itemAdded(index);
            }
        }

        void updateSelection_ItemsRemoved() const
        {
            for (const auto& [index, value] : reverse(_values)) {
                this->updateSelection_itemRemoved(index);
            }
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

        virtual void firstDo_editorData() const final
        {
            this->addItems(this->getEditorList());

            this->updateSelection_setSelection();
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            this->addItems(this->getProjectList(projectFile));

            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            this->removeItems(this->getEditorList());
            this->removeItems(this->getProjectList(projectFile));

            this->updateSelection_ItemsRemoved();
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            this->addItems(this->getEditorList());
            this->addItems(this->getProjectList(projectFile));

            this->updateSelection_ItemsAdded();
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

        virtual void firstDo_editorData() const final
        {
            this->removeItems(this->getEditorList());

            this->updateSelection_ItemsRemoved();
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            this->removeItems(this->getProjectList(projectFile));

            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            this->addItems(this->getEditorList());
            this->addItems(this->getProjectList(projectFile));

            this->updateSelection_ItemsAdded();
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            this->removeItems(this->getEditorList());
            this->removeItems(this->getProjectList(projectFile));

            this->updateSelection_ItemsRemoved();
        }
    };

    class MoveMultipleAction : public BaseAction {
    private:
        const std::vector<index_type> indexes;
        const EditListAction direction;

        template <typename FunctionT>
        void redo_helper(const index_type listSize, FunctionT move) const
        {
            switch (direction) {
            case EditListAction::ADD:
            case EditListAction::CLONE:
            case EditListAction::REMOVE: {
                abort();
            } break;

            case EditListAction::RAISE_TO_TOP: {
                assert(indexes.front() >= 0);
                assert(indexes.back() < listSize);
                assert(listSize >= indexes.size());

                index_type to = 0;
                for (const index_type& from : indexes) {
                    if (from != to) {
                        move(from, to);
                    }
                    to++;
                }
            } break;

            case EditListAction::RAISE: {
                assert(indexes.front() > 0);
                assert(indexes.back() < listSize);

                for (const index_type& i : indexes) {
                    move(i, i - 1);
                }
            } break;

            case EditListAction::LOWER: {
                assert(indexes.front() >= 0);
                assert(indexes.back() + 1 < listSize);

                for (const index_type& i : reverse(indexes)) {
                    move(i, i + 1);
                }

            } break;

            case EditListAction::LOWER_TO_BOTTOM: {
                assert(indexes.front() >= 0);
                assert(indexes.back() < listSize);
                assert(listSize >= indexes.size());

                index_type to = listSize - 1;
                for (const index_type& from : reverse(indexes)) {
                    if (from != to) {
                        move(from, to);
                    }
                    to--;
                }
            } break;
            }
        }

        template <typename FunctionT>
        void undo_helper(const index_type listSize, FunctionT move) const
        {
            switch (direction) {
            case EditListAction::ADD:
            case EditListAction::CLONE:
            case EditListAction::REMOVE: {
                abort();
            } break;

            case EditListAction::RAISE_TO_TOP: {
                assert(indexes.front() >= 0);
                assert(indexes.back() < listSize);
                assert(listSize >= indexes.size());

                index_type from = indexes.size() - 1;
                for (const index_type& to : reverse(indexes)) {
                    if (from != to) {
                        move(from, to);
                    }
                    from--;
                }
            } break;

            case EditListAction::RAISE: {
                assert(indexes.front() > 0);
                assert(indexes.back() < listSize);

                for (const index_type& i : reverse(indexes)) {
                    move(i - 1, i);
                }
            } break;

            case EditListAction::LOWER: {
                assert(indexes.front() >= 0);
                assert(indexes.back() + 1 < listSize);

                for (const index_type i : indexes) {
                    move(i + 1, i);
                }
            } break;

            case EditListAction::LOWER_TO_BOTTOM: {
                assert(indexes.front() >= 0);
                assert(indexes.back() < listSize);
                assert(listSize >= indexes.size());

                index_type from = listSize - indexes.size();
                for (const index_type to : indexes) {
                    if (from != to) {
                        move(from, to);
                    }
                    from++;
                }
            } break;
            }
        }

        void redo_list(ListT& list) const
        {
            redo_helper(list.size(), [&](index_type from, index_type to) {
                moveListItem(from, to, list);
            });
        }

        void undo_list(ListT& list) const
        {
            undo_helper(list.size(), [&](index_type from, index_type to) {
                moveListItem(from, to, list);
            });
        }

        void redo_updateSelection(const index_type listSize) const
        {
            redo_helper(listSize, [&](index_type from, index_type to) {
                this->updateSelection_itemMoved(from, to);
            });
        }

        void undo_updateSelection(const index_type listSize) const
        {
            undo_helper(listSize, [&](index_type from, index_type to) {
                this->updateSelection_itemMoved(from, to);
            });
        }

    public:
        MoveMultipleAction(EditorT* editor,
                           const ListArgsT& listArgs,
                           const std::vector<index_type>&& indexes,
                           const EditListAction direction)
            : BaseAction(editor, listArgs)
            , indexes(std::move(indexes))
            , direction(direction)
        {
        }
        virtual ~MoveMultipleAction() = default;

        virtual void firstDo_editorData() const final
        {
            ListT& list = this->getEditorList();

            redo_updateSelection(list.size());
            redo_list(list);
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            redo_list(this->getProjectList(projectFile));

            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            ListT& editorList = this->getEditorList();
            ListT& projectList = this->getProjectList(projectFile);

            assert(editorList.size() == projectList.size());

            undo_updateSelection(editorList.size());
            undo_list(editorList);
            undo_list(projectList);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            ListT& editorList = this->getEditorList();
            ListT& projectList = this->getProjectList(projectFile);

            assert(editorList.size() == projectList.size());

            redo_updateSelection(editorList.size());
            redo_list(editorList);
            redo_list(projectList);
        }
    };

    class EditItemAction final : public BaseAction {
    private:
        const index_type index;
        const value_type newValue;
        // set by firstDo()
        value_type oldValue;

        const value_type& getEditorValue() const
        {
            ListT& editorList = this->getEditorList();
            assert(index < editorList.size());
            return editorList.at(index);
        }

    public:
        EditItemAction(EditorT* editor,
                       const ListArgsT& listArgs,
                       const index_type index)
            : BaseAction(editor, listArgs)
            , index(index)
            , newValue(getEditorValue())
        {
        }
        virtual ~EditItemAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            ListT& projectList = this->getProjectList(projectFile);

            assert(index < projectList.size());

            oldValue = projectList.at(index);

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

    template <auto FieldPtr>
    class EditItemFieldAction final : public BaseAction {
        static_assert(std::is_member_object_pointer_v<decltype(FieldPtr)>);

        using FieldT = typename remove_member_pointer<decltype(FieldPtr)>::type;

    private:
        const index_type index;
        const FieldT newValue;
        // set by firstDo()
        FieldT oldValue;

        const FieldT& getEditorValue() const
        {
            ListT& editorList = this->getEditorList();
            assert(index < editorList.size());
            return editorList.at(index).*FieldPtr;
        }

    public:
        EditItemFieldAction(EditorT* editor,
                            const ListArgsT& listArgs,
                            const index_type index)
            : BaseAction(editor, listArgs)
            , index(index)
            , newValue(getEditorValue())
        {
        }
        virtual ~EditItemFieldAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            ListT& projectList = this->getProjectList(projectFile);

            assert(index < projectList.size());

            oldValue = projectList.at(index).*FieldPtr;

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
        const std::vector<value_type> _newValues;
        std::vector<value_type> _oldValues;

    private:
        void setValues(ListT& list, const std::vector<value_type>& values) const
        {
            assert(values.size() == _indexes.size());

            auto it = values.begin();
            for (const index_type index : _indexes) {
                const value_type value = *it++;

                list.at(index) = value;
            }
            assert(it == values.end());
        }

        static std::vector<value_type> getValues(const ListT& list, const std::vector<index_type>& indexes)
        {
            std::vector<value_type> ret;
            ret.reserve(indexes.size());

            for (const index_type index : indexes) {
                ret.push_back(list.at(index));
            }

            return ret;
        }

    public:
        EditMultipleItemsAction(EditorT* editor,
                                const ListArgsT& listArgs,
                                const std::vector<index_type>&& indexes)
            : BaseAction(editor, listArgs)
            , _indexes(std::move(indexes))
            , _newValues(getValues(this->getEditorList(), _indexes))
        {
        }
        virtual ~EditMultipleItemsAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            ListT& projectList = this->getProjectList(projectFile);

            _oldValues = getValues(projectList, _indexes);

            setValues(projectList, _newValues);

            return _oldValues != _newValues;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            setValues(this->getEditorList(), _oldValues);
            setValues(this->getProjectList(projectFile), _oldValues);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            setValues(this->getEditorList(), _newValues);
            setValues(this->getProjectList(projectFile), _newValues);
        }
    };

    template <auto FieldPtr>
    class EditAllItemsInListFieldAction final : public BaseAction {
        using FieldT = typename remove_member_pointer<decltype(FieldPtr)>::type;

    private:
        const std::vector<FieldT> _newValues;
        std::vector<FieldT> _oldValues;

    private:
        void setValues(ListT& list, const std::vector<FieldT>& values) const
        {
            assert(list.size() == values.size());

            auto it = list.begin();
            for (const FieldT& value : values) {
                (*it++).*FieldPtr = value;
            }
            assert(it == list.end());
        }

        static std::vector<FieldT> getValues(const ListT& list)
        {
            std::vector<FieldT> ret;
            ret.reserve(list.size());

            for (const value_type& d : list) {
                ret.push_back(d.*FieldPtr);
            }

            return ret;
        }

    public:
        EditAllItemsInListFieldAction(EditorT* editor,
                                      const ListArgsT& listArgs)
            : BaseAction(editor, listArgs)
            , _newValues(getValues(this->getEditorList()))
        {
        }
        virtual ~EditAllItemsInListFieldAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            const ListT& projectList = this->getProjectList(projectFile);

            _oldValues = getValues(projectList);

            setValues(this->getProjectList(projectFile), _newValues);

            return _oldValues != _newValues;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            setValues(this->getEditorList(), _oldValues);
            setValues(this->getProjectList(projectFile), _oldValues);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            setValues(this->getEditorList(), _newValues);
            setValues(this->getProjectList(projectFile), _newValues);
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

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            EditorDataT* projectData = ActionPolicy::getEditorData(projectFile, editor->itemIndex());
            EditorDataT* editorData = ActionPolicy::getEditorData(*editor);
            assert(projectData);
            assert(editorData);

            assert(oldValues.empty());

            bool changed = false;

            for (const IndexAndValues& childValues : indexesAndNewValues) {
                ListT& projectList = getList(projectData, childValues.listArgs);

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

protected:
    template <auto FieldPtr>
    static void _editListField(EditorT* editor, const ListArgsT& listArgs, const index_type index)
    {
        editor->addAction(
            std::make_unique<EditItemFieldAction<FieldPtr>>(
                editor, listArgs, index));
    }

    static void cloneMultiple(EditorT* editor, const ListArgsT& listArgs, const ListT& list,
                              std::vector<std::pair<index_type, value_type>>&& values)
    {
        if (list.size() + values.size() < MAX_SIZE) {
            unsigned i = list.size();
            for (auto& [index, v] : values) {
                index = i++;
            }
            editor->addAction(
                std::make_unique<AddMultipleAction>(editor, listArgs, std::move(values)));
        }
    }

    static void moveMultiple(EditorT* editor, const ListArgsT& listArgs, const std::vector<index_type>&& indexes, EditListAction action)
    {
        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (list->size() == 0
            || indexes.empty()
            || indexes.front() < 0
            || indexes.back() >= list->size()) {

            return;
        }

        switch (action) {
        case EditListAction::ADD:
        case EditListAction::CLONE:
        case EditListAction::REMOVE: {
        } break;

        case EditListAction::RAISE_TO_TOP: {
            if (indexes.back() >= indexes.size()) {
                editor->addAction(
                    std::make_unique<MoveMultipleAction>(editor, listArgs, std::move(indexes), action));
            }
        } break;

        case EditListAction::RAISE: {
            if (indexes.front() > 0) {
                editor->addAction(
                    std::make_unique<MoveMultipleAction>(editor, listArgs, std::move(indexes), action));
            }
        } break;

        case EditListAction::LOWER: {
            if (indexes.back() + 1 < list->size()) {
                editor->addAction(
                    std::make_unique<MoveMultipleAction>(editor, listArgs, std::move(indexes), action));
            }
        } break;

        case EditListAction::LOWER_TO_BOTTOM: {
            if (indexes.front() < list->size() - indexes.size()) {
                editor->addAction(
                    std::make_unique<MoveMultipleAction>(editor, listArgs, std::move(indexes), action));
            }
        } break;
        }
    }

public:
    static void addItemToSelectedList(EditorT* editor, const value_type& value)
    {
        const SelectionT& sel = getSelection(editor);
        const ListArgsT listArgs = sel.listArgs();

        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (list->size() < MAX_SIZE) {
            editor->addAction(
                std::make_unique<AddAction>(editor, listArgs, list->size(), value));
        }
    }

    static void addItem(EditorT* editor, const ListArgsT& listArgs, const value_type& value)
    {
        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (list->size() < MAX_SIZE) {
            editor->addAction(
                std::make_unique<AddAction>(editor, listArgs, list->size(), value));
        }
    }

    static void itemEdited(EditorT* editor, const ListArgsT& listArgs, const index_type index)
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

    static void selectedListItemEdited(EditorT* editor, const index_type index)
    {
        const SelectionT& sel = getSelection(editor);
        const ListArgsT listArgs = sel.listArgs();

        itemEdited(editor, listArgs, index);
    }

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

template <class ActionPolicy>
class SingleSelListActions : public AbstractListActions<ActionPolicy> {
    using EditorT = typename ActionPolicy::EditorT;
    using SelectionT = typename ActionPolicy::SelectionT;
    using ListArgsT = typename ActionPolicy::ListArgsT;
    using ListT = typename ActionPolicy::ListT;
    using index_type = typename ListT::size_type;

    constexpr static index_type MAX_SIZE = ActionPolicy::MAX_SIZE;

    using LA = AbstractListActions<ActionPolicy>;

public:
    static void editList(EditorT* editor, EditListAction action)
    {
        const SelectionT& sel = LA::getSelection(editor);
        const ListArgsT listArgs = sel.listArgs();

        const ListT* list = LA::getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        switch (action) {
        case EditListAction::ADD: {
            if (list->size() < MAX_SIZE) {
                editor->addAction(
                    std::make_unique<typename LA::AddAction>(editor, listArgs, list->size()));
            }
        } break;

        case EditListAction::CLONE: {
            const index_type i = sel.selectedIndex();
            if (i < list->size()) {
                if (list->size() < MAX_SIZE) {
                    auto& item = list->at(i);
                    editor->addAction(
                        std::make_unique<typename LA::AddAction>(editor, listArgs, i + 1, item));
                }
            }
        } break;

        case EditListAction::REMOVE: {
            const index_type i = sel.selectedIndex();
            if (i < list->size()) {
                editor->addAction(
                    std::make_unique<typename LA::RemoveAction>(editor, listArgs, i, list->at(i)));
            }
        } break;

        case EditListAction::RAISE_TO_TOP: {
            const index_type i = sel.selectedIndex();
            if (i > 0 && i < list->size()) {
                editor->addAction(
                    std::make_unique<typename LA::MoveAction>(editor, listArgs, i, 0));
            }
        } break;

        case EditListAction::RAISE: {
            const index_type i = sel.selectedIndex();
            if (i > 0 && i < list->size()) {
                editor->addAction(
                    std::make_unique<typename LA::MoveAction>(editor, listArgs, i, i - 1));
            }
        } break;

        case EditListAction::LOWER: {
            const index_type i = sel.selectedIndex();
            if (i + 1 < list->size()) {
                editor->addAction(
                    std::make_unique<typename LA::MoveAction>(editor, listArgs, i, i + 1));
            }
        } break;

        case EditListAction::LOWER_TO_BOTTOM: {
            const index_type i = sel.selectedIndex();
            if (i + 1 < list->size()) {
                editor->addAction(
                    std::make_unique<typename LA::MoveAction>(editor, listArgs, i, list->size() - 1));
            }
        } break;
        }
    }

    static void selectedItemEdited(EditorT* editor)
    {
        const auto& sel = LA::getSelection(editor);
        LA::itemEdited(editor, sel.listArgs(), sel.selectedIndex());
    }

    template <auto FieldPtr>
    static void selectedFieldEdited(EditorT* editor)
    {
        const auto& sel = LA::getSelection(editor);
        LA::template _editListField<FieldPtr>(editor, sel.listArgs(), sel.selectedIndex());
    }
};

template <class ActionPolicy>
class MultipleSelListActions : public AbstractListActions<ActionPolicy> {
    using EditorT = typename ActionPolicy::EditorT;
    using SelectionT = typename ActionPolicy::SelectionT;
    using ListArgsT = typename ActionPolicy::ListArgsT;
    using ListT = typename ActionPolicy::ListT;
    using index_type = typename ListT::size_type;

    constexpr static index_type MAX_SIZE = ActionPolicy::MAX_SIZE;

    using LA = AbstractListActions<ActionPolicy>;

public:
    static void editList(EditorT* editor, EditListAction action)
    {
        const SelectionT& sel = LA::getSelection(editor);
        const ListArgsT listArgs = sel.listArgs();

        const ListT* list = LA::getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        switch (action) {
        case EditListAction::ADD: {
            if (list->size() < MAX_SIZE) {
                editor->addAction(
                    std::make_unique<typename LA::AddAction>(editor, listArgs, list->size()));
            }
        } break;

        case EditListAction::CLONE: {
            LA::cloneMultiple(editor, listArgs, *list,
                              indexesAndDataForMultipleSelection(*list, sel));
        } break;

        case EditListAction::REMOVE: {
            auto values = indexesAndDataForMultipleSelection(*list, sel);
            if (!values.empty()) {
                editor->addAction(
                    std::make_unique<typename LA::RemoveMultipleAction>(editor, listArgs, std::move(values)));
            }
        } break;

        case EditListAction::RAISE_TO_TOP:
        case EditListAction::RAISE:
        case EditListAction::LOWER:
        case EditListAction::LOWER_TO_BOTTOM: {
            auto indexes = indexesForMultipleSelection(*list, sel);
            LA::moveMultiple(editor, listArgs, std::move(indexes), action);
        } break;
        }
    }

    static void selectedItemsEdited(EditorT* editor)
    {
        using EditMultipleItemsAction = typename LA::EditMultipleItemsAction;

        const SelectionT& sel = LA::getSelection(editor);
        const ListArgsT listArgs = sel.listArgs();

        const ListT* list = LA::getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        const index_type end = std::min<index_type>(list->size(), sel.MAX_SIZE);

        std::vector<index_type> indexes;
        indexes.reserve(end);

        for (const auto i : range(end)) {
            if (sel.isSelected(i)) {
                indexes.push_back(i);
            }
        }

        if (!indexes.empty()) {
            editor->addAction(
                std::make_unique<EditMultipleItemsAction>(editor, listArgs, std::move(indexes)));
        }
    }
};

template <class ActionPolicy, class SelectionT>
class ListActionsImpl;

template <class ActionPolicy>
class ListActionsImpl<ActionPolicy, SingleSelection> : public SingleSelListActions<ActionPolicy> {
    using EditorT = typename ActionPolicy::EditorT;
    using ListT = typename ActionPolicy::ListT;
    using index_type = typename ListT::size_type;
    using value_type = typename ListT::value_type;

    using LA = SingleSelListActions<ActionPolicy>;

public:
    static void addItem(EditorT* editor, const value_type& value)
    {
        LA::addItem(editor, std::make_tuple(), value);
    }

    static void itemEdited(EditorT* editor, const index_type index)
    {
        LA::itemEdited(editor, std::make_tuple<>(), index);
    }

    template <auto FieldPtr>
    static void fieldEdited(EditorT* editor, const index_type index)
    {
        LA::template _editListField<FieldPtr>(editor, std::make_tuple<>(), index);
    }
};

template <class ActionPolicy>
class ListActionsImpl<ActionPolicy, NodeSelection> : public SingleSelListActions<ActionPolicy> {
};

template <class ActionPolicy>
class ListActionsImpl<ActionPolicy, MultipleSelection> : public MultipleSelListActions<ActionPolicy> {
    using EditorT = typename ActionPolicy::EditorT;
    using ListT = typename ActionPolicy::ListT;
    using index_type = typename ListT::size_type;
    using value_type = typename ListT::value_type;

    using LA = MultipleSelListActions<ActionPolicy>;

    constexpr static index_type MAX_SIZE = ActionPolicy::MAX_SIZE;

public:
    static void addItem(EditorT* editor, const value_type& value)
    {
        LA::addItem(editor, std::make_tuple(), value);
    }

    static void itemEdited(EditorT* editor, const index_type index)
    {
        LA::itemEdited(editor, std::make_tuple(), index);
    }

    template <auto FieldPtr>
    static void fieldEdited(EditorT* editor, const index_type index)
    {
        LA::_editListField<FieldPtr>(editor, std::make_tuple(), index);
    }
};

template <class ActionPolicy>
class ListActionsImpl<ActionPolicy, MultipleChildSelection> : public MultipleSelListActions<ActionPolicy> {
    // No need to implement addItem, itemEdited or fieldEdited.
    // Implicit conversion of `unsigned` to `ListArgsT` will automatically generate these functions for me.
};

template <class ActionPolicy>
class ListActionsImpl<ActionPolicy, GroupMultipleSelection> : public AbstractListActions<ActionPolicy> {
    using EditorT = typename ActionPolicy::EditorT;
    using ListArgsT = typename ActionPolicy::ListArgsT;
    using ListT = typename ActionPolicy::ListT;
    using index_type = typename ListT::size_type;

    constexpr static index_type MAX_SIZE = ActionPolicy::MAX_SIZE;

    using LA = AbstractListActions<ActionPolicy>;

public:
    static void editList(EditorT* editor, EditListAction action)
    {
        using ParentActionPolicy = typename ActionPolicy::ParentActionPolicy;

        static_assert(ParentActionPolicy::MAX_SIZE <= GroupMultipleSelection::MAX_GROUP_SIZE);
        static_assert(std::is_same_v<ListArgsT, std::tuple<unsigned>>);

        const GroupMultipleSelection& sel = LA::getSelection(editor);

        switch (action) {
        case EditListAction::ADD: {
            // Can only add an item to a group if it is selected
            const SingleSelection& parentSel = editor->*ParentActionPolicy::SelectionPtr;
            const ListArgsT listArgs = std::make_tuple(parentSel.selectedIndex());
            if (const ListT* list = LA::getEditorListPtr(editor, listArgs)) {
                editor->addAction(
                    std::make_unique<typename LA::AddAction>(editor, listArgs, list->size()));
            }
        } break;

        case EditListAction::CLONE: {
            editor->startMacro();
            for (const auto groupIndex : range(sel.MAX_GROUP_SIZE)) {
                const auto& childSel = sel.childSel(groupIndex);
                const ListArgsT listArgs = std::make_tuple(groupIndex);
                if (const ListT* list = LA::getEditorListPtr(editor, listArgs)) {
                    LA::cloneMultiple(editor, listArgs, *list,
                                      indexesAndDataForMultipleSelection(*list, childSel));
                }
            }
            editor->endMacro();
        } break;

        case EditListAction::REMOVE: {
            editor->startMacro();
            for (const auto groupIndex : range(sel.MAX_GROUP_SIZE)) {
                const ListArgsT listArgs = std::make_tuple(groupIndex);
                if (const ListT* list = LA::getEditorListPtr(editor, listArgs)) {
                    auto& childSel = sel.childSel(groupIndex);
                    auto values = indexesAndDataForMultipleSelection(*list, childSel);
                    if (!values.empty()) {
                        editor->addAction(
                            std::make_unique<typename LA::RemoveMultipleAction>(editor, listArgs, std::move(values)));
                    }
                }
            }
            editor->endMacro();
        } break;

        case EditListAction::RAISE_TO_TOP:
        case EditListAction::RAISE:
        case EditListAction::LOWER:
        case EditListAction::LOWER_TO_BOTTOM: {
            editor->startMacro();
            for (const auto groupIndex : range(sel.MAX_GROUP_SIZE)) {
                const ListArgsT listArgs = std::make_tuple(groupIndex);
                if (const ListT* list = LA::getEditorListPtr(editor, listArgs)) {
                    auto& childSel = sel.childSel(groupIndex);
                    auto indexes = indexesForMultipleSelection(*list, childSel);
                    LA::moveMultiple(editor, listArgs, std::move(indexes), action);
                }
            }
            editor->endMacro();
        } break;
        }
    }

    static void selectedItemsEdited(EditorT* editor)
    {
        const GroupMultipleSelection& sel = LA::getSelection(editor);

        std::vector<typename LA::EditMultipleNestedItems::IndexAndValues> values;

        for (const auto groupIndex : range(sel.MAX_GROUP_SIZE)) {
            const ListArgsT listArgs = std::make_tuple(groupIndex);
            if (const ListT* list = LA::getEditorListPtr(editor, listArgs)) {
                const auto& childSel = sel.childSel(groupIndex);
                auto childIndexesAndValues = indexesAndDataForMultipleSelection(*list, childSel);

                if (!childIndexesAndValues.empty()) {
                    values.push_back({ groupIndex, std::move(childIndexesAndValues) });
                }
            }
        }
        if (!values.empty()) {
            editor->addAction(
                std::make_unique<typename LA::EditMultipleNestedItems>(editor, std::move(values)));
        }
    }
};

template <class ActionPolicy>
using ListActions = ListActionsImpl<ActionPolicy, typename ActionPolicy::SelectionT>;

}
