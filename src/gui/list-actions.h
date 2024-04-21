/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstract-editor.h"
#include "editor-actions-notify-gui.h"
#include "selection.h"
#include "models/common/iterators.h"
#include "models/common/type-traits.h"
#include <algorithm>
#include <gsl/gsl>

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
class AbstractListActions {
    using EditorT = typename ActionPolicy::EditorT;
    using EditorDataT = typename ActionPolicy::EditorDataT;
    using ListArgsT = typename ActionPolicy::ListArgsT;
    using SelectionT = typename ActionPolicy::SelectionT;
    using ListT = typename ActionPolicy::ListT;
    using index_type = typename ListT::size_type;
    using value_type = typename ListT::value_type;

    using NotNullEditorPtr = gsl::not_null<std::shared_ptr<EditorT>>;

    constexpr static index_type MAX_SIZE = ActionPolicy::MAX_SIZE;

    static_assert(MAX_SIZE <= SelectionT::MAX_SIZE);

public:
    [[nodiscard]] static const ListT* getEditorListPtr(const NotNullEditorPtr& editor, const ListArgsT& listArgs)
    {
        EditorDataT* data = ActionPolicy::getEditorData(*editor);
        assert(data != nullptr);
        return std::apply(&ActionPolicy::getList,
                          std::tuple_cat(std::forward_as_tuple(*data), listArgs));
    }

    [[nodiscard]] static const SelectionT& getSelection(const EditorT& editor)
    {
        return editor.*(ActionPolicy::SelectionPtr);
    }

    [[nodiscard]] static SelectionT& selection(EditorT& editor)
    {
        return editor.*(ActionPolicy::SelectionPtr);
    }

protected:
    class BaseAction : public UndoAction {
    private:
        std::weak_ptr<EditorT> _editor;
        ListArgsT listArgs;

    public:
        ~BaseAction() override = default;

    protected:
        BaseAction(const NotNullEditorPtr& editor,
                   const ListArgsT& listArgs_)
            : UndoAction()
            , _editor(editor.get())
            , listArgs{ listArgs_ }
        {
        }

        [[nodiscard]] inline NotNullEditorPtr getEditor() const
        {
            auto e = _editor.lock();
            assert(e != nullptr);
            return e;
        }

        [[nodiscard]] ListT& getProjectList(Project::ProjectFile& projectFile, EditorT& editor) const
        {
            EditorDataT* data = ActionPolicy::getEditorData(projectFile, editor.itemIndex());
            assert(data != nullptr);
            ListT* list = std::apply(&ActionPolicy::getList,
                                     std::tuple_cat(std::forward_as_tuple(*data), listArgs));
            assert(list != nullptr);
            return *list;
        }

        [[nodiscard]] ListT& getEditorList(EditorT& editor) const
        {
            EditorDataT* data = ActionPolicy::getEditorData(editor);
            assert(data != nullptr);
            ListT* list = std::apply(&ActionPolicy::getList,
                                     std::tuple_cat(std::forward_as_tuple(*data), listArgs));
            assert(list != nullptr);
            return *list;
        }

        void updateSelection_setSelected(EditorT& editor, index_type index) const
        {
            std::apply(&SelectionT::setSelected,
                       std::tuple_cat(std::forward_as_tuple(selection(editor)), this->listArgs, std::make_tuple(index)));
        }

        void updateSelection_clearSelection(EditorT& editor) const
        {
            selection(editor).clearSelection();
        }

        void updateSelection_appendSelection(EditorT& editor, index_type index) const
        {
            auto f = [&](auto... args) {
                selection(editor).appendSelection(args...);
            };
            std::apply(f, std::tuple_cat(this->listArgs, std::make_tuple(index)));
        }

        void updateSelection_itemAdded(EditorT& editor, index_type index) const
        {
            std::apply(&SelectionT::itemAdded,
                       std::tuple_cat(std::forward_as_tuple(selection(editor)), this->listArgs, std::make_tuple(index)));
        }

        void updateSelection_itemRemoved(EditorT& editor, index_type index) const
        {
            std::apply(&SelectionT::itemRemoved,
                       std::tuple_cat(std::forward_as_tuple(selection(editor)), this->listArgs, std::make_tuple(index)));
        }

        void updateSelection_itemMoved(EditorT& editor, index_type from, index_type to) const
        {
            std::apply(&SelectionT::itemMoved,
                       std::tuple_cat(std::forward_as_tuple(selection(editor)), this->listArgs, std::make_tuple(from, to)));
        }

    public:
        void notifyGui(AbstractEditorGui* gui) const final
        {
            editorUndoAction_notifyGui<ActionPolicy>(gui);
        }
    };

    // Only `AbstractListActions` are allowed to create add/remove/move actions.
    //
    // This ensures the indexes and listArgs are correctly bounds checked before the actions are created.
private:
    class AddRemoveAction : public BaseAction {
    private:
        index_type index;
        value_type value;

    public:
        AddRemoveAction(const NotNullEditorPtr& editor,
                        const ListArgsT& listArgs,
                        const index_type index,
                        const value_type& value)
            : BaseAction(std::move(editor), listArgs)
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

        void updateSelection_ItemAdded_first(EditorT& editor) const
        {
            this->updateSelection_setSelected(editor, index);
        }

        void updateSelection_ItemAdded(EditorT& editor) const
        {
            this->updateSelection_itemAdded(editor, index);
        }

        void updateSelection_ItemRemoved(EditorT& editor) const
        {
            this->updateSelection_itemRemoved(editor, index);
        }
    };

private:
    class AddAction final : public AddRemoveAction {
    public:
        AddAction(const NotNullEditorPtr& editor,
                  const ListArgsT& listArgs,
                  const index_type index)
            : AddRemoveAction(editor, listArgs, index, value_type())
        {
        }

        AddAction(const NotNullEditorPtr& editor,
                  const ListArgsT& listArgs,
                  const index_type index,
                  const value_type& value)
            : AddRemoveAction(editor, listArgs, index, value)
        {
        }
        virtual ~AddAction() = default;

        virtual void firstDo_editorData() const final
        {
            auto e = this->getEditor();

            this->addItem(this->getEditorList(*e));

            this->updateSelection_ItemAdded_first(*e);
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            auto e = this->getEditor();

            this->addItem(this->getProjectList(projectFile, *e));

            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            this->removeItem(this->getEditorList(*e));
            this->removeItem(this->getProjectList(projectFile, *e));

            this->updateSelection_ItemRemoved(*e);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            this->addItem(this->getEditorList(*e));
            this->addItem(this->getProjectList(projectFile, *e));

            this->updateSelection_ItemAdded(*e);
        }
    };

private:
    class RemoveAction final : public AddRemoveAction {
    public:
        RemoveAction(const NotNullEditorPtr& editor,
                     const ListArgsT& listArgs,
                     const index_type index,
                     const value_type& value)
            : AddRemoveAction(editor, listArgs, index, value)
        {
        }
        virtual ~RemoveAction() = default;

        virtual void firstDo_editorData() const final
        {
            auto e = this->getEditor();

            this->removeItem(this->getEditorList(*e));

            this->updateSelection_ItemRemoved(*e);
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            auto e = this->getEditor();

            this->removeItem(this->getProjectList(projectFile, *e));

            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            this->addItem(this->getEditorList(*e));
            this->addItem(this->getProjectList(projectFile, *e));

            this->updateSelection_ItemAdded(*e);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            this->removeItem(this->getEditorList(*e));
            this->removeItem(this->getProjectList(projectFile, *e));

            this->updateSelection_ItemRemoved(*e);
        }
    };

private:
    class MoveAction final : public BaseAction {
    private:
        index_type fromIndex;
        index_type toIndex;

        void moveItem(Project::ProjectFile& projectFile, EditorT& e, index_type from, index_type to) const
        {
            ListT& projectList = this->getProjectList(projectFile, e);
            ListT& editorList = this->getEditorList(e);

            assert(projectList.size() == editorList.size());

            assert(from < projectList.size());
            assert(to < projectList.size());

            moveListItem(from, to, projectList);
            moveListItem(from, to, editorList);

            this->updateSelection_itemMoved(e, from, to);
        }

    public:
        MoveAction(const NotNullEditorPtr& editor,
                   const ListArgsT& listArgs,
                   const index_type from,
                   const index_type to)
            : BaseAction(std::move(editor), listArgs)
            , fromIndex(from)
            , toIndex(to)
        {
        }
        virtual ~MoveAction() = default;

        virtual void firstDo_editorData() const final
        {
            auto e = this->getEditor();

            moveListItem(fromIndex, toIndex, this->getEditorList(*e));

            this->updateSelection_itemMoved(*e, fromIndex, toIndex);
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            auto e = this->getEditor();

            moveListItem(fromIndex, toIndex, this->getProjectList(projectFile, *e));

            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            moveItem(projectFile, *e, toIndex, fromIndex);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            moveItem(projectFile, *e, fromIndex, toIndex);
        }
    };

private:
    class AddRemoveMultipleAction : public BaseAction {
    private:
        std::vector<std::pair<index_type, value_type>> _values;

    public:
        // values vector must be sorted by index_type and not contain any duplicate indexes
        AddRemoveMultipleAction(const NotNullEditorPtr& editor,
                                const ListArgsT& listArgs,
                                const std::vector<std::pair<index_type, value_type>>&& values)
            : BaseAction(std::move(editor), listArgs)
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

        void updateSelection_setSelection(EditorT& e) const
        {
            this->updateSelection_clearSelection(e);
            for (const auto& [index, value] : _values) {
                this->updateSelection_appendSelection(e, index);
            }
        }

        void updateSelection_ItemsAdded(EditorT& e) const
        {
            for (const auto& [index, value] : _values) {
                this->updateSelection_itemAdded(e, index);
            }
        }

        void updateSelection_ItemsRemoved(EditorT& e) const
        {
            for (const auto& [index, value] : reverse(_values)) {
                this->updateSelection_itemRemoved(e, index);
            }
        }
    };

private:
    class AddMultipleAction final : public AddRemoveMultipleAction {
    public:
        // values vector must be sorted by index_type and not contain any duplicate indexes
        AddMultipleAction(const NotNullEditorPtr& editor,
                          const ListArgsT& listArgs,
                          const std::vector<std::pair<index_type, value_type>>&& values)
            : AddRemoveMultipleAction(editor, listArgs, std::move(values))
        {
        }
        virtual ~AddMultipleAction() = default;

        virtual void firstDo_editorData() const final
        {
            auto e = this->getEditor();

            this->addItems(this->getEditorList(*e));

            this->updateSelection_setSelection(*e);
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            auto e = this->getEditor();

            this->addItems(this->getProjectList(projectFile, *e));

            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            this->removeItems(this->getEditorList(*e));
            this->removeItems(this->getProjectList(projectFile, *e));

            this->updateSelection_ItemsRemoved(*e);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            this->addItems(this->getEditorList(*e));
            this->addItems(this->getProjectList(projectFile, *e));

            this->updateSelection_ItemsAdded(*e);
        }
    };

private:
    class RemoveMultipleAction final : public AddRemoveMultipleAction {
    public:
        // values vector must be sorted by index_type and not contain any duplicate indexes
        RemoveMultipleAction(const NotNullEditorPtr& editor,
                             const ListArgsT& listArgs,
                             const std::vector<std::pair<index_type, value_type>>&& values)
            : AddRemoveMultipleAction(editor, listArgs, std::move(values))
        {
        }
        virtual ~RemoveMultipleAction() = default;

        virtual void firstDo_editorData() const final
        {
            auto e = this->getEditor();

            this->removeItems(this->getEditorList(*e));

            this->updateSelection_ItemsRemoved(*e);
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            auto e = this->getEditor();

            this->removeItems(this->getProjectList(projectFile, *e));

            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            this->addItems(this->getEditorList(*e));
            this->addItems(this->getProjectList(projectFile, *e));

            this->updateSelection_ItemsAdded(*e);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            this->removeItems(this->getEditorList(*e));
            this->removeItems(this->getProjectList(projectFile, *e));

            this->updateSelection_ItemsRemoved(*e);
        }
    };

private:
    class MoveMultipleAction : public BaseAction {
    private:
        std::vector<index_type> indexes;
        EditListAction direction;

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

        void redo_updateSelection(EditorT& editor, const index_type listSize) const
        {
            redo_helper(listSize, [&](index_type from, index_type to) {
                this->updateSelection_itemMoved(editor, from, to);
            });
        }

        void undo_updateSelection(EditorT& editor, const index_type listSize) const
        {
            undo_helper(listSize, [&](index_type from, index_type to) {
                this->updateSelection_itemMoved(editor, from, to);
            });
        }

    public:
        MoveMultipleAction(const NotNullEditorPtr& editor,
                           const ListArgsT& listArgs,
                           const std::vector<index_type>&& indexes,
                           const EditListAction direction)
            : BaseAction(std::move(editor), listArgs)
            , indexes(std::move(indexes))
            , direction(direction)
        {
        }
        virtual ~MoveMultipleAction() = default;

        virtual void firstDo_editorData() const final
        {
            auto e = this->getEditor();

            ListT& list = this->getEditorList(*e);

            redo_updateSelection(*e, list.size());
            redo_list(list);
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            auto e = this->getEditor();

            redo_list(this->getProjectList(projectFile, *e));

            return true;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            ListT& editorList = this->getEditorList(*e);
            ListT& projectList = this->getProjectList(projectFile, *e);

            assert(editorList.size() == projectList.size());

            undo_updateSelection(*e, editorList.size());
            undo_list(editorList);
            undo_list(projectList);
        }

        virtual void
        redo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            ListT& editorList = this->getEditorList(*e);
            ListT& projectList = this->getProjectList(projectFile, *e);

            assert(editorList.size() == projectList.size());

            redo_updateSelection(*e, editorList.size());
            redo_list(editorList);
            redo_list(projectList);
        }
    };

protected:
    class EditItemAction final : public BaseAction {
    private:
        index_type index;
        value_type newValue;
        // set by firstDo()
        value_type oldValue;

        const value_type& getEditorValue(EditorT& editor) const
        {
            ListT& editorList = this->getEditorList(editor);
            assert(index < editorList.size());
            return editorList.at(index);
        }

    public:
        EditItemAction(const NotNullEditorPtr& editor,
                       const ListArgsT& listArgs,
                       const index_type index)
            : BaseAction(editor, listArgs)
            , index(index)
            , newValue(getEditorValue(*editor))
        {
        }
        virtual ~EditItemAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            auto e = this->getEditor();

            ListT& projectList = this->getProjectList(projectFile, *e);

            assert(index < projectList.size());

            oldValue = projectList.at(index);

            projectList.at(index) = newValue;

            // operator!= may not implemented in a few of my structs
            return !(oldValue == newValue);
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            ListT& projectList = this->getProjectList(projectFile, *e);
            ListT& editorList = this->getEditorList(*e);

            assert(projectList.size() == editorList.size());
            assert(index < projectList.size());

            projectList.at(index) = oldValue;
            editorList.at(index) = oldValue;
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            ListT& projectList = this->getProjectList(projectFile, *e);
            ListT& editorList = this->getEditorList(*e);

            assert(projectList.size() == editorList.size());
            assert(index < projectList.size());

            projectList.at(index) = newValue;
            editorList.at(index) = newValue;
        }
    };

protected:
    template <auto FieldPtr>
    class EditItemFieldAction final : public BaseAction {
        static_assert(std::is_member_object_pointer_v<decltype(FieldPtr)>);

        using FieldT = typename remove_member_pointer<decltype(FieldPtr)>::type;

    private:
        index_type index;
        FieldT newValue;
        // set by firstDo()
        FieldT oldValue;

        const FieldT& getEditorValue(EditorT& e) const
        {
            ListT& editorList = this->getEditorList(e);
            assert(index < editorList.size());
            return editorList.at(index).*FieldPtr;
        }

    public:
        EditItemFieldAction(const NotNullEditorPtr& editor,
                            const ListArgsT& listArgs,
                            const index_type index)
            : BaseAction(editor, listArgs)
            , index(index)
            , newValue(getEditorValue(*editor))
        {
        }
        virtual ~EditItemFieldAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            auto e = this->getEditor();

            ListT& projectList = this->getProjectList(projectFile, *e);

            assert(index < projectList.size());

            oldValue = projectList.at(index).*FieldPtr;

            projectList.at(index).*FieldPtr = newValue;

            return oldValue != newValue;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            ListT& projectList = this->getProjectList(projectFile, *e);
            ListT& editorList = this->getEditorList(*e);

            assert(projectList.size() == editorList.size());
            assert(index < projectList.size());

            projectList.at(index).*FieldPtr = oldValue;
            editorList.at(index).*FieldPtr = oldValue;
        }

        virtual void
        redo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            ListT& projectList = this->getProjectList(projectFile, *e);
            ListT& editorList = this->getEditorList(*e);

            assert(projectList.size() == editorList.size());
            assert(index < projectList.size());

            projectList.at(index).*FieldPtr = newValue;
            editorList.at(index).*FieldPtr = newValue;
        }
    };

protected:
    class EditMultipleItemsAction final : public BaseAction {
    private:
        std::vector<index_type> _indexes;
        std::vector<value_type> _newValues;
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
            std::vector<value_type> ret(indexes.size());
            std::transform(indexes.begin(), indexes.end(),
                           ret.begin(),
                           [&](const index_type i) { return list.at(i); });

            return ret;
        }

    public:
        EditMultipleItemsAction(const NotNullEditorPtr& editor,
                                const ListArgsT& listArgs,
                                const std::vector<index_type>&& indexes)
            : BaseAction(editor, listArgs)
            , _indexes(std::move(indexes))
            , _newValues(getValues(this->getEditorList(*editor), _indexes))
        {
        }
        virtual ~EditMultipleItemsAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            auto e = this->getEditor();

            ListT& projectList = this->getProjectList(projectFile, *e);

            _oldValues = getValues(projectList, _indexes);

            setValues(projectList, _newValues);

            return _oldValues != _newValues;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            setValues(this->getEditorList(*e), _oldValues);
            setValues(this->getProjectList(projectFile, *e), _oldValues);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            setValues(this->getEditorList(*e), _newValues);
            setValues(this->getProjectList(projectFile, *e), _newValues);
        }
    };

protected:
    template <auto FieldPtr>
    class EditAllItemsInListFieldAction final : public BaseAction {
        using FieldT = typename remove_member_pointer<decltype(FieldPtr)>::type;

    private:
        std::vector<FieldT> _newValues;
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
            std::vector<FieldT> ret(list.size());
            std::transform(list.begin(), list.end(),
                           ret.begin(),
                           [](const value_type& d) { return d.*FieldPtr; });

            return ret;
        }

    public:
        EditAllItemsInListFieldAction(const NotNullEditorPtr& editor,
                                      const ListArgsT& listArgs)
            : BaseAction(editor, listArgs)
            , _newValues(getValues(this->getEditorList(*editor)))
        {
        }
        virtual ~EditAllItemsInListFieldAction() = default;

        virtual void firstDo_editorData() const final
        {
        }

        virtual bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            auto e = this->getEditor();

            const ListT& projectList = this->getProjectList(projectFile, *e);

            _oldValues = getValues(projectList);

            setValues(this->getProjectList(projectFile), _newValues);

            return _oldValues != _newValues;
        }

        virtual void undo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            setValues(this->getEditorList(*e), _oldValues);
            setValues(this->getProjectList(projectFile, *e), _oldValues);
        }

        virtual void redo(Project::ProjectFile& projectFile) const final
        {
            auto e = this->getEditor();

            setValues(this->getEditorList(*e), _newValues);
            setValues(this->getProjectList(projectFile, *e), _newValues);
        }
    };

protected:
    class EditMultipleNestedItems final : public UndoAction {
    public:
        struct IndexAndValues {
            ListArgsT listArgs;
            std::vector<std::pair<index_type, value_type>> childIndexesAndValues;
        };

    private:
        std::weak_ptr<EditorT> _editor;
        std::vector<IndexAndValues> indexesAndNewValues;
        std::vector<value_type> oldValues;

    private:
        [[nodiscard]] inline NotNullEditorPtr getEditor() const
        {
            auto e = _editor.lock();
            assert(e != nullptr);
            return e;
        }

        static ListT& getList(EditorDataT& data, const ListArgsT& listArgs)
        {
            ListT* list = std::apply(&ActionPolicy::getList,
                                     std::tuple_cat(std::forward_as_tuple(data), listArgs));
            assert(list != nullptr);
            return *list;
        }

    public:
        EditMultipleNestedItems(const NotNullEditorPtr& editor,
                                const std::vector<IndexAndValues>&& indexesAndValues)
            : UndoAction()
            , _editor(editor.get())
            , indexesAndNewValues(std::move(indexesAndValues))
        {
        }
        ~EditMultipleNestedItems() override = default;

        void firstDo_editorData() const final
        {
        }

        bool firstDo_projectFile(Project::ProjectFile& projectFile) final
        {
            auto e = getEditor();

            EditorDataT* projectData = ActionPolicy::getEditorData(projectFile, e->itemIndex());
            EditorDataT* editorData = ActionPolicy::getEditorData(*e);
            assert(projectData);
            assert(editorData);

            assert(oldValues.empty());

            bool changed = false;

            for (const IndexAndValues& childValues : indexesAndNewValues) {
                ListT& projectList = getList(*projectData, childValues.listArgs);

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

        void undo(Project::ProjectFile& projectFile) const final
        {
            auto e = getEditor();

            EditorDataT* projectData = ActionPolicy::getEditorData(projectFile, e->itemIndex());
            EditorDataT* editorData = ActionPolicy::getEditorData(*e);
            assert(projectData);
            assert(editorData);

            auto it = oldValues.begin();

            for (const IndexAndValues& childValues : indexesAndNewValues) {
                ListT& projectList = getList(*projectData, childValues.listArgs);
                ListT& editorList = getList(*editorData, childValues.listArgs);

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

        void redo(Project::ProjectFile& projectFile) const final
        {
            auto e = getEditor();

            EditorDataT* projectData = ActionPolicy::getEditorData(projectFile, e->itemIndex());
            EditorDataT* editorData = ActionPolicy::getEditorData(*e);
            assert(projectData);
            assert(editorData);

            for (const IndexAndValues& childValues : indexesAndNewValues) {
                ListT& projectList = getList(*projectData, childValues.listArgs);
                ListT& editorList = getList(*editorData, childValues.listArgs);

                assert(projectList.size() == editorList.size());

                for (const auto& [index, newValue] : childValues.childIndexesAndValues) {
                    assert(index >= 0 && index < projectList.size());

                    projectList.at(index) = newValue;
                    editorList.at(index) = newValue;
                }
            }
        }

    public:
        void notifyGui(AbstractEditorGui* gui) const final
        {
            editorUndoAction_notifyGui<ActionPolicy>(gui);
        }
    };

protected:
    template <auto FieldPtr>
    static void _editListField(const NotNullEditorPtr& editor, const ListArgsT& listArgs, const index_type index)
    {
        editor->undoStack().addAction(
            std::make_unique<EditItemFieldAction<FieldPtr>>(
                editor, listArgs, index));
    }

    // `values` MUST BE sorted by index (ie, created by `indexesAndDataForMultipleSelection`)
    static void cloneMultiple(const NotNullEditorPtr& editor, const ListArgsT& listArgs,
                              std::vector<std::pair<index_type, value_type>>&& values)
    {
        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (list->size() == 0
            || list->size() + values.size() > MAX_SIZE
            || values.empty()
            || values.front().first < 0
            || values.back().first >= list->size()) {

            return;
        }

        unsigned i = list->size();
        for (auto& [index, v] : values) {
            index = i++;
        }
        editor->undoStack().addAction(
            std::make_unique<AddMultipleAction>(editor, listArgs, std::move(values)));
    }

    // `values` MUST BE sorted by index (ie, created by `indexesAndDataForMultipleSelection`)
    static void removeMultiple(const NotNullEditorPtr& editor, const ListArgsT& listArgs,
                               std::vector<std::pair<index_type, value_type>>&& values)
    {
        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (list->size() == 0
            || values.empty()
            || values.front().first < 0
            || values.back().first >= list->size()) {

            return;
        }

        editor->undoStack().addAction(
            std::make_unique<RemoveMultipleAction>(editor, listArgs, std::move(values)));
    }

    // `indexes` MUST be sorted (ie, created by `indexesForMultipleSelection`)
    static void moveMultiple(const NotNullEditorPtr& editor, const ListArgsT& listArgs, const std::vector<index_type>&& indexes, EditListAction action)
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
                editor->undoStack().addAction(
                    std::make_unique<MoveMultipleAction>(editor, listArgs, std::move(indexes), action));
            }
        } break;

        case EditListAction::RAISE: {
            if (indexes.front() > 0) {
                editor->undoStack().addAction(
                    std::make_unique<MoveMultipleAction>(editor, listArgs, std::move(indexes), action));
            }
        } break;

        case EditListAction::LOWER: {
            if (indexes.back() + 1 < list->size()) {
                editor->undoStack().addAction(
                    std::make_unique<MoveMultipleAction>(editor, listArgs, std::move(indexes), action));
            }
        } break;

        case EditListAction::LOWER_TO_BOTTOM: {
            if (indexes.front() < list->size() - indexes.size()) {
                editor->undoStack().addAction(
                    std::make_unique<MoveMultipleAction>(editor, listArgs, std::move(indexes), action));
            }
        } break;
        }
    }

public:
    static void addItemToSelectedList(const NotNullEditorPtr& editor, const value_type& value)
    {
        const SelectionT& sel = getSelection(*editor);
        const ListArgsT listArgs = sel.listArgs();

        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (list->size() < MAX_SIZE) {
            editor->undoStack().addAction(
                std::make_unique<AddAction>(editor, listArgs, list->size(), value));
        }
    }

    static void addItem(const NotNullEditorPtr& editor, const ListArgsT& listArgs)
    {
        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (list->size() < MAX_SIZE) {
            editor->undoStack().addAction(
                std::make_unique<AddAction>(editor, listArgs, list->size()));
        }
    }

    static void addItem(const NotNullEditorPtr& editor, const ListArgsT& listArgs, const value_type& value)
    {
        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (list->size() < MAX_SIZE) {
            editor->undoStack().addAction(
                std::make_unique<AddAction>(editor, listArgs, list->size(), value));
        }
    }

    static void cloneItem(const NotNullEditorPtr& editor, const ListArgsT& listArgs, const index_type index)
    {
        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (index < list->size()) {
            if (list->size() < MAX_SIZE) {
                const auto& item = list->at(index);
                editor->undoStack().addAction(
                    std::make_unique<AddAction>(editor, listArgs, index + 1, item));
            }
        }
    }

    static void removeItem(const NotNullEditorPtr& editor, const ListArgsT& listArgs, const index_type index)
    {
        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (index < list->size()) {
            const auto& item = list->at(index);
            editor->undoStack().addAction(
                std::make_unique<RemoveAction>(editor, listArgs, index, item));
        }
    }

    static void moveItem(const NotNullEditorPtr& editor, const ListArgsT& listArgs,
                         const index_type fromIndex, const index_type toIndex)
    {
        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (fromIndex != toIndex
            && fromIndex >= 0 && fromIndex < list->size()
            && toIndex >= 0 && toIndex < list->size()) {

            editor->undoStack().addAction(
                std::make_unique<MoveAction>(editor, listArgs, fromIndex, toIndex));
        }
    }

    static void moveItemToBottom(const NotNullEditorPtr& editor, const ListArgsT& listArgs, const index_type index)
    {
        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (index >= 0 && index + 1 < list->size()) {
            editor->undoStack().addAction(
                std::make_unique<MoveAction>(editor, listArgs, index, list->size() - 1));
        }
    }

    static void itemEdited(const NotNullEditorPtr& editor, const ListArgsT& listArgs, const index_type index)
    {
        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        if (index < list->size()) {
            editor->undoStack().addAction(
                std::make_unique<EditItemAction>(editor, listArgs, index));
        }
    }

    static void selectedListItemEdited(const NotNullEditorPtr& editor, const index_type index)
    {
        const SelectionT& sel = getSelection(editor);
        const ListArgsT listArgs = sel.listArgs();

        itemEdited(editor, listArgs, index);
    }

    static void selectedListItemsEdited(const NotNullEditorPtr& editor, std::vector<index_type> indexes)
    {
        const SelectionT& sel = getSelection(*editor);
        const ListArgsT listArgs = sel.listArgs();

        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        indexes.erase(std::remove_if(indexes.begin(), indexes.end(),
                                     [&](auto i) { return i >= list->size(); }),
                      indexes.end());

        if (!indexes.empty()) {
            editor->undoStack().addAction(
                std::make_unique<EditMultipleItemsAction>(editor, listArgs, std::move(indexes)));
        }
    }

    template <auto FieldPtr>
    static void allItemsInSelectedListFieldEdited(const NotNullEditorPtr& editor)
    {
        const SelectionT& sel = getSelection(*editor);
        const ListArgsT listArgs = sel.listArgs();

        const ListT* list = getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }
        if (list->empty()) {
            return;
        }

        editor->undoStack().addAction(
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

    using LA = AbstractListActions<ActionPolicy>;

    using NotNullEditorPtr = gsl::not_null<std::shared_ptr<EditorT>>;

public:
    static void editList(const NotNullEditorPtr& editor, EditListAction action)
    {
        const SelectionT& sel = LA::getSelection(*editor);
        const ListArgsT listArgs = sel.listArgs();

        const index_type i = sel.selectedIndex();

        switch (action) {
        case EditListAction::ADD: {
            LA::addItem(editor, listArgs);
        } break;

        case EditListAction::CLONE: {
            LA::cloneItem(editor, listArgs, sel.selectedIndex());
        } break;

        case EditListAction::REMOVE: {
            LA::removeItem(editor, listArgs, sel.selectedIndex());
        } break;

        case EditListAction::RAISE_TO_TOP: {
            LA::moveItem(editor, listArgs, i, 0);
        } break;

        case EditListAction::RAISE: {
            if (i > 0) {
                LA::moveItem(editor, listArgs, i, i - 1);
            }
        } break;

        case EditListAction::LOWER: {
            LA::moveItem(editor, listArgs, i, i + 1);
        } break;

        case EditListAction::LOWER_TO_BOTTOM: {
            LA::moveItemToBottom(editor, listArgs, i);
        } break;
        }
    }

    static void selectedItemEdited(const NotNullEditorPtr& editor)
    {
        const auto& sel = LA::getSelection(*editor);
        LA::itemEdited(editor, sel.listArgs(), sel.selectedIndex());
    }

    template <auto FieldPtr>
    static void selectedFieldEdited(const NotNullEditorPtr& editor)
    {
        const auto& sel = LA::getSelection(*editor);
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

    using NotNullEditorPtr = gsl::not_null<std::shared_ptr<EditorT>>;

    using LA = AbstractListActions<ActionPolicy>;

public:
    static void editList(const NotNullEditorPtr& editor, EditListAction action)
    {
        const SelectionT& sel = LA::getSelection(*editor);
        const ListArgsT listArgs = sel.listArgs();

        const ListT* list = LA::getEditorListPtr(editor, listArgs);
        if (list == nullptr) {
            return;
        }

        switch (action) {
        case EditListAction::ADD: {
            LA::addItem(editor, listArgs);
        } break;

        case EditListAction::CLONE: {
            LA::cloneMultiple(editor, listArgs,
                              indexesAndDataForMultipleSelection(*list, sel));
        } break;

        case EditListAction::REMOVE: {
            LA::removeMultiple(editor, listArgs,
                               indexesAndDataForMultipleSelection(*list, sel));
        } break;

        case EditListAction::RAISE_TO_TOP:
        case EditListAction::RAISE:
        case EditListAction::LOWER:
        case EditListAction::LOWER_TO_BOTTOM: {
            LA::moveMultiple(editor, listArgs,
                             indexesForMultipleSelection(*list, sel),
                             action);
        } break;
        }
    }

    static void selectedItemsEdited(const NotNullEditorPtr& editor)
    {
        using EditMultipleItemsAction = typename LA::EditMultipleItemsAction;

        const SelectionT& sel = LA::getSelection(*editor);
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
            editor->undoStack().addAction(
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

    using NotNullEditorPtr = gsl::not_null<std::shared_ptr<EditorT>>;

    using LA = SingleSelListActions<ActionPolicy>;

public:
    static void addItem(const NotNullEditorPtr& editor, const value_type& value)
    {
        LA::addItem(editor, std::make_tuple(), value);
    }

    static void itemEdited(const NotNullEditorPtr& editor, const index_type index)
    {
        LA::itemEdited(editor, std::make_tuple<>(), index);
    }

    template <auto FieldPtr>
    static void fieldEdited(const NotNullEditorPtr& editor, const index_type index)
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

    using NotNullEditorPtr = gsl::not_null<std::shared_ptr<EditorT>>;

    using LA = MultipleSelListActions<ActionPolicy>;

public:
    static void addItem(const NotNullEditorPtr& editor, const value_type& value)
    {
        LA::addItem(editor, std::make_tuple(), value);
    }

    static void itemEdited(const NotNullEditorPtr& editor, const index_type index)
    {
        LA::itemEdited(editor, std::make_tuple(), index);
    }

    template <auto FieldPtr>
    static void fieldEdited(const NotNullEditorPtr& editor, const index_type index)
    {
        LA::template _editListField<FieldPtr>(editor, std::make_tuple(), index);
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

    using NotNullEditorPtr = gsl::not_null<std::shared_ptr<EditorT>>;

    using LA = AbstractListActions<ActionPolicy>;

public:
    static void editList(const NotNullEditorPtr& editor, EditListAction action)
    {
        using ParentActionPolicy = typename ActionPolicy::ParentActionPolicy;

        static_assert(ParentActionPolicy::MAX_SIZE <= GroupMultipleSelection::MAX_GROUP_SIZE);
        static_assert(std::is_same_v<ListArgsT, std::tuple<unsigned>>);

        const GroupMultipleSelection& sel = LA::getSelection(*editor);

        switch (action) {
        case EditListAction::ADD: {
            // Can only add an item to a group if the parent is selected
            const SingleSelection& parentSel = (*editor).*(ParentActionPolicy::SelectionPtr);
            const ListArgsT listArgs = std::make_tuple(parentSel.selectedIndex());

            LA::addItem(editor, listArgs);
        } break;

        case EditListAction::CLONE: {
            editor->undoStack().startMacro();
            for (const auto groupIndex : range(sel.MAX_GROUP_SIZE)) {
                const auto& childSel = sel.childSel(groupIndex);
                const ListArgsT listArgs = std::make_tuple(groupIndex);
                if (const ListT* list = LA::getEditorListPtr(editor, listArgs)) {
                    LA::cloneMultiple(editor, listArgs,
                                      indexesAndDataForMultipleSelection(*list, childSel));
                }
            }
            editor->undoStack().endMacro();
        } break;

        case EditListAction::REMOVE: {
            editor->undoStack().startMacro();
            for (const auto groupIndex : range(sel.MAX_GROUP_SIZE)) {
                const ListArgsT listArgs = std::make_tuple(groupIndex);
                if (const ListT* list = LA::getEditorListPtr(editor, listArgs)) {
                    const auto& childSel = sel.childSel(groupIndex);
                    LA::removeMultiple(editor, listArgs,
                                       indexesAndDataForMultipleSelection(*list, childSel));
                }
            }
            editor->undoStack().endMacro();
        } break;

        case EditListAction::RAISE_TO_TOP:
        case EditListAction::RAISE:
        case EditListAction::LOWER:
        case EditListAction::LOWER_TO_BOTTOM: {
            editor->undoStack().startMacro();
            for (const auto groupIndex : range(sel.MAX_GROUP_SIZE)) {
                const ListArgsT listArgs = std::make_tuple(groupIndex);
                if (const ListT* list = LA::getEditorListPtr(editor, listArgs)) {
                    const auto& childSel = sel.childSel(groupIndex);
                    LA::moveMultiple(editor, listArgs,
                                     indexesForMultipleSelection(*list, childSel),
                                     action);
                }
            }
            editor->undoStack().endMacro();
        } break;
        }
    }

    static void selectedItemsEdited(const NotNullEditorPtr& editor)
    {
        const GroupMultipleSelection& sel = LA::getSelection(*editor);

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
            editor->undoStack().addAction(
                std::make_unique<typename LA::EditMultipleNestedItems>(editor, std::move(values)));
        }
    }
};

template <class ActionPolicy>
using ListActions = ListActionsImpl<ActionPolicy, typename ActionPolicy::SelectionT>;
}
