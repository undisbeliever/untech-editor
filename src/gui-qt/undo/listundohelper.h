/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/call.h"
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

            list->insert(list->begin() + _index, _value);

            this->emitItemAdded(_index);
            this->emitListChanged();
        }

        void removeItem()
        {
            ListT* list = this->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

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

private:
    AccessorT* const _accessor;
    const ArgsT _args;

public:
    template <typename... A>
    ListUndoHelper(AccessorT* accessor, A... args)
        : _accessor(accessor)
        , _args(std::tie(args...))
    {
    }

private:
    inline ListT* getList()
    {
        auto f = std::mem_fn(&AccessorT::getList);
        return mem_fn_call(f, _accessor, _args);
    }

public:
    // will return nullptr if data cannot be accessed or is equal to newValue
    QUndoCommand* editCommand(index_type index, const DataT& newValue)
    {
        ListT* list = getList();
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
        return new EditCommand(_accessor, _args, index, oldValue, newValue);
    }

    void edit(index_type index, const DataT& newValue)
    {
        QUndoCommand* e = editCommand(index, newValue);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
    }

    // will return nullptr if data cannot be accessed or is equal to newValue
    template <typename FieldT>
    QUndoCommand* editFieldCommand(index_type index, const FieldT& newValue,
                                   const QString& text,
                                   typename std::function<FieldT&(DataT&)> getter)
    {
        ListT* list = getList();
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
            _accessor, _args, index, oldValue, newValue, text, getter);
    }

    template <typename FieldT>
    void editField(index_type index, const FieldT& newValue,
                   const QString& text,
                   typename std::function<FieldT&(DataT&)> getter)
    {
        QUndoCommand* e = editFieldCommand(index, newValue, text, getter);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* addCommand(index_type index)
    {
        ListT* list = getList();
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index > list->size()) {
            return nullptr;
        }
        if (list->size() >= max_size) {
            return nullptr;
        }

        return new AddCommand(_accessor, _args, index);
    }

    void addItem()
    {
        ListT* list = getList();
        if (list == nullptr) {
            return;
        }
        index_type index = list->size();

        QUndoCommand* c = addCommand(index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
    }

    void addItem(index_type index)
    {
        QUndoCommand* c = addCommand(index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* cloneCommand(index_type index)
    {
        ListT* list = getList();
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }
        if (list->size() >= max_size) {
            return nullptr;
        }

        return new AddCommand(_accessor, _args, index, list->at(index));
    }

    void cloneItem(index_type index)
    {
        QUndoCommand* c = cloneCommand(index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* removeCommand(index_type index)
    {
        ListT* list = getList();
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }

        return new RemoveCommand(_accessor, _args, index, list->at(index));
    }

    void removeItem(index_type index)
    {
        QUndoCommand* c = removeCommand(index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
    }
};
}
}
}
