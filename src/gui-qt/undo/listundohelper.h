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
    using size_type = typename AccessorT::size_type;
    using ArgsT = typename AccessorT::ArgsT;

private:
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
        ListT* getList()
        {
            auto f = std::mem_fn(&AccessorT::getList);
            return mem_fn_call(f, _accessor, _args);
        }

        void emitDataChanged(size_type index)
        {
            auto f = std::mem_fn(&AccessorT::dataChanged);
            mem_fn_call(f, _accessor, _args, index);

            _accessor->resourceItem()->dataChanged();
        }
    };

    class EditCommand : public BaseCommand {
    private:
        const size_type _index;
        const DataT _oldValue;
        const DataT _newValue;

    public:
        EditCommand(AccessorT* accessor, const ArgsT& args, size_type index,
                    const DataT& oldValue, const DataT& newValue)
            : BaseCommand(accessor, args,
                          QCoreApplication::tr("Edit %1").arg(accessor->typeName()))
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
        const size_type _index;
        const FieldT _oldValue;
        const FieldT _newValue;
        const std::function<FieldT&(DataT&)> _getter;

    public:
        EditFieldCommand(AccessorT* accessor, const ArgsT& args, size_type index,
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
    QUndoCommand* editCommand(size_type index, const DataT& newValue)
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

    void edit(size_type index, const DataT& newValue)
    {
        QUndoCommand* e = editCommand(index, newValue);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
    }

    // will return nullptr if data cannot be accessed or is equal to newValue
    template <typename FieldT>
    QUndoCommand* editFieldCommand(size_type index, const FieldT& newValue,
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
    void editField(size_type index, const FieldT& newValue,
                   const QString& text,
                   typename std::function<FieldT&(DataT&)> getter)
    {
        QUndoCommand* e = editFieldCommand(index, newValue, text, getter);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
    }
};
}
}
}
