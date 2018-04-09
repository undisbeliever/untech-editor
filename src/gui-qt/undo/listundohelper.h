/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/call.h"
#include "models/common/vector-helpers.h"
#include <QCoreApplication>
#include <QUndoCommand>
#include <functional>

namespace UnTech {
namespace GuiQt {
namespace Undo {

template <class AccessorT>
class ListUndoHelper {

public:
    using DataT = typename AccessorT::DataT;
    using ListT = typename AccessorT::ListT;
    using index_type = typename AccessorT::index_type;
    using ArgsT = typename AccessorT::ArgsT;

    constexpr static index_type max_size = AccessorT::max_size;

private:
    static inline QString tr(const char* s)
    {
        return QCoreApplication::tr(s);
    }

    class BaseCommand : public QUndoCommand {
    protected:
        AccessorT* _accessor;
        const ArgsT _args;

    public:
        BaseCommand(AccessorT* accessor, const ArgsT& args,
                    const QString& text)
            : QUndoCommand(text)
            , _accessor(accessor)
            , _args(args)
        {
        }
        ~BaseCommand() = default;

    protected:
        inline ListT* getList()
        {
            auto f = std::mem_fn(&AccessorT::getList);
            return mem_fn_call(f, _accessor, _args);
        }

        inline void emitDataChanged(index_type index)
        {
            auto f = std::mem_fn(&AccessorT::dataChanged);
            mem_fn_call(f, _accessor, _args, index);

            _accessor->resourceItem()->dataChanged();
        }

        inline void emitListChanged()
        {
            auto f = std::mem_fn(&AccessorT::listChanged);
            mem_fn_call(f, _accessor, _args);

            _accessor->resourceItem()->dataChanged();
        }

        // When this signal is emitted you MUST close all editors
        // accessing the list to prevent data corruption
        inline void emitListAboutToChange()
        {
            auto f = std::mem_fn(&AccessorT::listAboutToChange);
            mem_fn_call(f, _accessor, _args);
        }

        inline void emitItemAdded(index_type index)
        {
            auto f = std::mem_fn(&AccessorT::itemAdded);
            mem_fn_call(f, _accessor, _args, index);
        }

        inline void emitItemAboutToBeRemoved(index_type index)
        {
            auto f = std::mem_fn(&AccessorT::itemAboutToBeRemoved);
            mem_fn_call(f, _accessor, _args, index);
        }

        inline void emitItemMoved(index_type from, index_type to)
        {
            auto f = std::mem_fn(&AccessorT::itemMoved);
            mem_fn_call(f, _accessor, _args, from, to);
        }
    };

    class EditCommand : public BaseCommand {
    private:
        const index_type _index;
        const DataT _oldValue;
        const DataT _newValue;

    public:
        EditCommand(AccessorT* accessor, const ArgsT& args, index_type index,
                    const DataT& oldValue, const DataT& newValue)
            : BaseCommand(accessor, args,
                          tr("Edit %1").arg(accessor->typeName()))
            , _index(index)
            , _oldValue(oldValue)
            , _newValue(newValue)
        {
        }
        ~EditCommand() = default;

        virtual void undo() final
        {
            ListT* list = this->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            list->at(_index) = _oldValue;

            this->emitDataChanged(_index);
        }

        virtual void redo() final
        {
            ListT* list = this->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            list->at(_index) = _newValue;

            this->emitDataChanged(_index);
        }
    };

    template <typename FieldT>
    class EditFieldCommand : public BaseCommand {
    private:
        const index_type _index;
        const FieldT _oldValue;
        const FieldT _newValue;
        const std::function<FieldT&(DataT&)> _getter;

    public:
        EditFieldCommand(AccessorT* accessor, const ArgsT& args, index_type index,
                         const FieldT& oldValue, const FieldT& newValue,
                         const QString& text,
                         typename std::function<FieldT&(DataT&)> getter)
            : BaseCommand(accessor, args, text)
            , _index(index)
            , _oldValue(oldValue)
            , _newValue(newValue)
            , _getter(getter)
        {
        }
        ~EditFieldCommand() = default;

        virtual void undo() final
        {
            ListT* list = this->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            _getter(list->at(_index)) = _oldValue;

            this->emitDataChanged(_index);
        }

        virtual void redo() final
        {
            ListT* list = this->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            _getter(list->at(_index)) = _newValue;

            this->emitDataChanged(_index);
        }
    };

    class AddRemoveCommand : public BaseCommand {
    private:
        const index_type _index;
        const DataT _value;

    protected:
        AddRemoveCommand(AccessorT* accessor, const ArgsT& args, index_type index,
                         const DataT& value,
                         const QString& text)
            : BaseCommand(accessor, args, text)
            , _index(index)
            , _value(value)
        {
        }
        ~AddRemoveCommand() = default;

        void addItem()
        {
            ListT* list = this->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index <= list->size());

            this->emitListAboutToChange();

            list->insert(list->begin() + _index, _value);

            if (this->_accessor->selectedListTuple() == this->_args) {
                index_type sel = this->_accessor->selectedIndex();

                if (sel >= _index) {
                    this->_accessor->setSelectedIndex(sel + 1);
                }
            }

            this->emitItemAdded(_index);
            this->emitListChanged();
        }

        void removeItem()
        {
            ListT* list = this->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            this->emitListAboutToChange();

            if (this->_accessor->selectedListTuple() == this->_args) {
                index_type sel = this->_accessor->selectedIndex();

                if (sel == _index) {
                    this->_accessor->unselectItem();
                }
                else if (sel > _index) {
                    this->_accessor->setSelectedIndex(sel - 1);
                }
            }

            this->emitItemAboutToBeRemoved(_index);

            list->erase(list->begin() + _index);

            this->emitListChanged();
        }
    };

    class AddCommand : public AddRemoveCommand {
    public:
        AddCommand(AccessorT* accessor, const ArgsT& args, index_type index)
            : AddRemoveCommand(accessor, args, index, DataT(),
                               tr("Add %1").arg(accessor->typeName()))
        {
        }

        AddCommand(AccessorT* accessor, const ArgsT& args, index_type index, const DataT& value)
            : AddRemoveCommand(accessor, args, index, value,
                               tr("Clone %1").arg(accessor->typeName()))
        {
        }
        ~AddCommand() = default;

        virtual void undo() final
        {
            this->removeItem();
        }

        virtual void redo() final
        {
            this->addItem();
        }
    };

    class RemoveCommand : public AddRemoveCommand {
    public:
        RemoveCommand(AccessorT* accessor, const ArgsT& args, index_type index, const DataT& value)
            : AddRemoveCommand(accessor, args, index, value,
                               tr("Remove %1").arg(accessor->typeName()))
        {
        }
        ~RemoveCommand() = default;

        virtual void undo() final
        {
            this->addItem();
        }

        virtual void redo() final
        {
            this->removeItem();
        }
    };

    class MoveCommand : public BaseCommand {
    private:
        const index_type _fromIndex;
        const index_type _toIndex;

    public:
        MoveCommand(AccessorT* accessor, const ArgsT& args,
                    index_type fromIndex, index_type toIndex,
                    const QString& text)
            : BaseCommand(accessor, args, text)
            , _fromIndex(fromIndex)
            , _toIndex(toIndex)
        {
        }
        ~MoveCommand() = default;

        void undo()
        {
            moveItem(_toIndex, _fromIndex);
        }

        void redo()
        {
            moveItem(_fromIndex, _toIndex);
        }

    private:
        void moveItem(index_type from, index_type to)
        {
            ListT* list = this->getList();
            Q_ASSERT(list);
            Q_ASSERT(from != to);
            Q_ASSERT(from >= 0 && from < list->size());
            Q_ASSERT(to >= 0 && to < list->size());

            index_type selected = this->_accessor->selectedIndex();

            this->emitListAboutToChange();

            moveListItem(from, to, *list);

            this->emitItemMoved(from, to);
            this->emitListChanged();

            if (this->_accessor->selectedListTuple() == this->_args) {
                if (selected == from) {
                    this->_accessor->setSelectedIndex(to);
                }
                else if (selected > from && selected <= to) {
                    this->_accessor->setSelectedIndex(selected - 1);
                }
                else if (selected >= to && selected < from) {
                    this->_accessor->setSelectedIndex(selected + 1);
                }
            }
        }
    };

private:
    AccessorT* const _accessor;

public:
    ListUndoHelper(AccessorT* accessor)
        : _accessor(accessor)
    {
    }

private:
    inline ListT* getList(const ArgsT& listArgs)
    {
        auto f = std::mem_fn(&AccessorT::getList);
        return mem_fn_call(f, _accessor, listArgs);
    }

public:
    // will return nullptr if data cannot be accessed or is equal to newValue
    QUndoCommand* editCommand(const ArgsT& listArgs, index_type index, const DataT& newValue)
    {
        ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }
        const DataT& oldValue = list->at(index);

        if (oldValue == newValue) {
            return nullptr;
        }
        return new EditCommand(_accessor, listArgs, index, oldValue, newValue);
    }

    void edit(const ArgsT& listArgs, index_type index, const DataT& newValue)
    {
        QUndoCommand* e = editCommand(listArgs, index, newValue);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
    }

    void editSelectedItem(const DataT& newValue)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        const index_type index = _accessor->selectedIndex();
        edit(listArgs, index, newValue);
    }

    // will return nullptr if data cannot be accessed or is equal to newValue
    template <typename FieldT>
    QUndoCommand* editFieldCommand(const ArgsT& listArgs, index_type index, const FieldT& newValue,
                                   const QString& text,
                                   typename std::function<FieldT&(DataT&)> getter)
    {
        ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }
        const FieldT& oldValue = getter(list->at(index));

        if (oldValue == newValue) {
            return nullptr;
        }
        return new EditFieldCommand<FieldT>(
            _accessor, listArgs, index, oldValue, newValue, text, getter);
    }

    template <typename FieldT>
    void editField(const ArgsT& listArgs, index_type index, const FieldT& newValue,
                   const QString& text,
                   typename std::function<FieldT&(DataT&)> getter)
    {
        QUndoCommand* e = editFieldCommand(listArgs, index, newValue, text, getter);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
    }

    template <typename FieldT>
    void editSelectedItemField(const FieldT& newValue,
                               const QString& text,
                               typename std::function<FieldT&(DataT&)> getter)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        const index_type index = _accessor->selectedIndex();
        editField(listArgs, index, newValue, text, getter);
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* addCommand(const ArgsT& listArgs, index_type index)
    {
        ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index > list->size()) {
            return nullptr;
        }
        if (list->size() >= max_size) {
            return nullptr;
        }

        return new AddCommand(_accessor, listArgs, index);
    }

    void addItem(const ArgsT& listArgs)
    {
        ListT* list = getList(listArgs);
        if (list == nullptr) {
            return;
        }
        index_type index = list->size();

        QUndoCommand* c = addCommand(listArgs, index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
    }

    void addItem(const ArgsT& listArgs, index_type index)
    {
        QUndoCommand* c = addCommand(listArgs, index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
    }

    void addItemToSelectedList(index_type index)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();

        QUndoCommand* c = addCommand(listArgs, index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);

            _accessor->setSelectedIndex(index);
        }
    }

    void addItemToSelectedList()
    {
        const ArgsT listArgs = _accessor->selectedListTuple();

        ListT* list = getList(listArgs);
        if (list == nullptr) {
            return;
        }
        index_type index = list->size();

        QUndoCommand* c = addCommand(listArgs, index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);

            _accessor->setSelectedIndex(index);
        }
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* cloneCommand(const ArgsT& listArgs, index_type index)
    {
        ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }
        if (list->size() >= max_size) {
            return nullptr;
        }

        return new AddCommand(_accessor, listArgs, index, list->at(index));
    }

    void cloneItem(const ArgsT& listArgs, index_type index)
    {
        QUndoCommand* c = cloneCommand(listArgs, index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
    }

    void cloneSelectedItem()
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        const index_type index = _accessor->selectedIndex();

        QUndoCommand* c = cloneCommand(listArgs, index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
            _accessor->setSelectedIndex(index + 1);
        }
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* removeCommand(const ArgsT& listArgs, index_type index)
    {
        ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }

        return new RemoveCommand(_accessor, listArgs, index, list->at(index));
    }

    void removeItem(const ArgsT& listArgs, index_type index)
    {
        QUndoCommand* c = removeCommand(listArgs, index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
    }

    void removeSelectedItem()
    {
        removeItem(_accessor->selectedListTuple(), _accessor->selectedIndex());
    }

    // will return nullptr if list cannot be accessed or indexes are invalid
    QUndoCommand* moveCommand(const ArgsT& listArgs, index_type from, index_type to,
                              const QString& text)
    {
        ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (from == to) {
            return nullptr;
        }
        if (from < 0 || from >= list->size()) {
            return nullptr;
        }
        if (to < 0 || to >= list->size()) {
            return nullptr;
        }

        return new MoveCommand(_accessor, listArgs, from, to, text);
    }

    QUndoCommand* moveCommand(const ArgsT& listArgs, index_type from, index_type to)
    {
        return moveCommand(listArgs, from, to,
                           tr("Move %1").arg(_accessor->typeName()));
    }

    void moveItem(const ArgsT& listArgs, index_type from, index_type to)
    {
        QUndoCommand* c = moveCommand(listArgs, from, to);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
    }

    void moveItem(const ArgsT& listArgs, index_type from, index_type to, const QString& text)
    {
        QUndoCommand* c = moveCommand(listArgs, from, to, text);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
    }

    void raiseSelectedItemToTop()
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        const index_type index = _accessor->selectedIndex();

        if (index > 0) {
            moveItem(listArgs, index, 0, tr("Raise To Top"));
        }
    }

    void raiseSelectedItem()
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        const index_type index = _accessor->selectedIndex();

        if (index > 0) {
            moveItem(listArgs, index, index - 1, tr("Raise"));
        }
    }

    void lowerSelectedItem()
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        const index_type index = _accessor->selectedIndex();

        moveItem(listArgs, index, index + 1, tr("Lower"));
    }

    void lowerSelectedItemToBottom()
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        const index_type index = _accessor->selectedIndex();

        ListT* list = getList(listArgs);
        if (list == nullptr) {
            return;
        }

        index_type list_size = list->size();
        if (list_size > 1) {
            moveItem(listArgs, index, list_size - 1, tr("Lower To Bottom"));
        }
    }
};
}
}
}
