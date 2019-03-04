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
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

template <class T>
class ListAndSelectionUndoHelper;
template <class T>
class ListAndMultipleSelectionUndoHelper;

template <class AccessorT>
class ListUndoHelper {

public:
    using DataT = typename AccessorT::DataT;
    using ListT = typename AccessorT::ListT;
    using index_type = typename AccessorT::index_type;
    using ArgsT = typename AccessorT::ArgsT;

    friend class ListAndSelectionUndoHelper<AccessorT>;
    friend class ListAndMultipleSelectionUndoHelper<AccessorT>;

private:
    static inline QString tr(const char* s)
    {
        return QCoreApplication::tr(s);
    }

    class BaseCommand : public QUndoCommand {
    protected:
        AccessorT* const _accessor;
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

        template <typename ExtraSignalsFunction>
        inline void emitDataChanged(index_type index, ExtraSignalsFunction extraSignals)
        {
            auto f = std::mem_fn(&AccessorT::dataChanged);
            mem_fn_call(f, _accessor, _args, index);

            auto extraSignalsArgs = std::tuple_cat(std::make_tuple(_accessor), _args, std::make_tuple(index));
            call(extraSignals, extraSignalsArgs);

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
        EditCommand(AccessorT* accessor, const ArgsT& args, index_type index,
                    const DataT& oldValue, const DataT& newValue,
                    const QString& text)
            : BaseCommand(accessor, args, text)
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

    // This class allows command merging
    class EditMergeCommand : public BaseCommand {
    private:
        const index_type _index;
        const DataT _oldValue;
        DataT _newValue;
        const bool _first;

    public:
        EditMergeCommand(AccessorT* accessor, const ArgsT& args, index_type index,
                         const DataT& oldValue, const DataT& newValue, const bool first)
            : BaseCommand(accessor, args,
                          tr("Edit %1").arg(accessor->typeName()))
            , _index(index)
            , _oldValue(oldValue)
            , _newValue(newValue)
            , _first(first)
        {
        }
        ~EditMergeCommand() = default;

        virtual int id() const final
        {
            return 0x1337;
        }

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

        virtual bool mergeWith(const QUndoCommand* cmd) final
        {
            const EditMergeCommand* command = dynamic_cast<const EditMergeCommand*>(cmd);

            if (command
                && command->_first == false
                && command->_args == this->_args
                && command->_index == this->_index
                && command->_oldValue == this->_newValue) {

                _newValue = command->_newValue;

                return true;
            }
            else {
                return false;
            }
        }
    };

    template <typename>
    struct EmptySignalFunction;

    template <typename... SignalArgsT>
    struct EmptySignalFunction<std::tuple<SignalArgsT...>> {
        inline void operator()(AccessorT*, SignalArgsT..., size_t) const {}
    };

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    class EditFieldCommand : public BaseCommand {
    private:
        const index_type _index;
        const FieldT _oldValue;
        const FieldT _newValue;
        const UnaryFunction _getter;
        const ExtraSignalsFunction _signalEmitter;

    public:
        EditFieldCommand(AccessorT* accessor, const ArgsT& args, index_type index,
                         const FieldT& oldValue, const FieldT& newValue,
                         const QString& text,
                         UnaryFunction getter, ExtraSignalsFunction signalEmitter)
            : BaseCommand(accessor, args, text)
            , _index(index)
            , _oldValue(oldValue)
            , _newValue(newValue)
            , _getter(getter)
            , _signalEmitter(signalEmitter)
        {
        }
        ~EditFieldCommand() = default;

        virtual void undo() final
        {
            ListT* list = this->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            _getter(list->at(_index)) = _oldValue;

            this->emitDataChanged(_index, _signalEmitter);
        }

        virtual void redo() final
        {
            ListT* list = this->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            _getter(list->at(_index)) = _newValue;

            this->emitDataChanged(_index, _signalEmitter);
        }
    };

    template <typename UnaryFunction, typename... FieldT>
    class EditMultipleFieldsCommand : public BaseCommand {
    private:
        const index_type _index;
        const std::tuple<FieldT...> _oldValues;
        const std::tuple<FieldT...> _newValues;
        const UnaryFunction _getter;

    public:
        EditMultipleFieldsCommand(AccessorT* accessor, const ArgsT& args, index_type index,
                                  const std::tuple<FieldT&...> oldValues, const std::tuple<FieldT...>& newValues,
                                  const QString& text,
                                  UnaryFunction getter)
            : BaseCommand(accessor, args, text)
            , _index(index)
            , _oldValues(oldValues)
            , _newValues(newValues)
            , _getter(getter)
        {
        }
        ~EditMultipleFieldsCommand() = default;

        virtual void undo() final
        {
            ListT* list = this->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            DataT& item = list->at(_index);
            std::tuple<FieldT&...> fields = _getter(item);
            fields = _oldValues;

            this->emitDataChanged(_index);
        }

        virtual void redo() final
        {
            ListT* list = this->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            DataT& item = list->at(_index);
            std::tuple<FieldT&...> fields = _getter(item);
            fields = _newValues;

            this->emitDataChanged(_index);
        }
    };

    template <typename FieldT, typename UnaryFunction>
    class EditFieldIncompleteCommand : public BaseCommand {
    private:
        const index_type _index;
        const FieldT _oldValue;
        FieldT _newValue;
        const UnaryFunction _getter;

    public:
        EditFieldIncompleteCommand(AccessorT* accessor, const ArgsT& args, index_type index,
                                   const FieldT& oldValue,
                                   const QString& text,
                                   UnaryFunction getter)
            : BaseCommand(accessor, args, text)
            , _index(index)
            , _oldValue(oldValue)
            , _newValue(oldValue)
            , _getter(getter)
        {
        }
        ~EditFieldIncompleteCommand() = default;

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

        void setValue(const FieldT& v)
        {
            _newValue = v;
        }

        bool hasValueChanged() const
        {
            return _newValue != _oldValue;
        }
    };

    class EditMultipleCommand : public BaseCommand {
    private:
        const std::vector<index_type> _indexes;
        const std::vector<DataT> _oldValues;
        const std::vector<DataT> _newValues;

    public:
        EditMultipleCommand(AccessorT* accessor, const ArgsT& args,
                            std::vector<index_type>&& indexes,
                            std::vector<DataT>&& oldValues,
                            std::vector<DataT>&& newValues,
                            const QString& text)
            : BaseCommand(accessor, args, text)
            , _indexes(std::move(indexes))
            , _oldValues(std::move(oldValues))
            , _newValues(std::move(newValues))
        {
        }
        ~EditMultipleCommand() = default;

        virtual void undo() final
        {
            doEdit(_oldValues);
        }

        virtual void redo() final
        {
            doEdit(_newValues);
        }

    private:
        void doEdit(const std::vector<DataT>& values)
        {
            Q_ASSERT(_indexes.size() == values.size());

            ListT* list = this->getList();
            Q_ASSERT(list);

            for (size_t i = 0; i < _indexes.size(); i++) {
                const index_type& index = _indexes.at(i);
                const DataT& value = values.at(i);

                Q_ASSERT(index >= 0 && index < list->size());

                list->at(index) = value;

                this->emitDataChanged(index);
            }
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

            this->emitItemAdded(_index);
            this->emitListChanged();
        }

        void removeItem()
        {
            ListT* list = this->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            this->emitListAboutToChange();
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

        AddCommand(AccessorT* accessor, const ArgsT& args, index_type index, const DataT& value, const QString text)
            : AddRemoveCommand(accessor, args, index, value, text)
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

            this->emitListAboutToChange();

            moveListItem(from, to, *list);

            this->emitItemMoved(from, to);
            this->emitListChanged();
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
    inline const ArgsT selectedListTuple()
    {
        return _accessor->selectedListTuple();
    }

    inline const ListT* getList(const ArgsT& listArgs)
    {
        auto f = std::mem_fn(&AccessorT::getList);
        return mem_fn_call(f, _accessor, listArgs);
    }

    // MUST ONLY this function when it's absolutely necessary
    inline ListT* getList_NO_CONST(const ArgsT& listArgs)
    {
        auto f = std::mem_fn(&AccessorT::getList);
        return mem_fn_call(f, _accessor, listArgs);
    }

public:
    // will return nullptr if data cannot be accessed or is equal to newValue
    QUndoCommand* editCommand(const ArgsT& listArgs, index_type index, const DataT& newValue)
    {
        const ListT* list = getList(listArgs);
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

    bool edit(const ArgsT& listArgs, index_type index, const DataT& newValue)
    {
        QUndoCommand* e = editCommand(listArgs, index, newValue);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    bool editItemInSelectedList(index_type index, const DataT& newValue)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();

        return edit(listArgs, index, newValue);
    }

    // will return nullptr if data cannot be accessed or is equal to newValue
    QUndoCommand* editMergeCommand(const ArgsT& listArgs, index_type index,
                                   const DataT& newValue, bool first = false)
    {
        const ListT* list = getList(listArgs);
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

        return new EditMergeCommand(_accessor, listArgs, index, oldValue, newValue, first);
    }

    bool editMerge(const ArgsT& listArgs, index_type index, const DataT& newValue, bool first = false)
    {
        QUndoCommand* c = editMergeCommand(listArgs, index, newValue, first);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    bool editItemInSelectedListMerge(index_type index, const DataT& newValue, bool first = false)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();

        return editMerge(listArgs, index, newValue, first);
    }

    // will return nullptr if data cannot be accessed or is unchanged
    template <typename EditFunction>
    QUndoCommand* editItemInListCommand(const ArgsT& listArgs, index_type index,
                                        const QString& text,
                                        EditFunction editFunction)
    {
        ListT* list = getList_NO_CONST(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }

        const DataT& oldValue = list->at(index);
        DataT newValue = oldValue;

        editFunction(newValue);

        if (newValue == oldValue) {
            return nullptr;
        }
        return new EditCommand(_accessor, listArgs, index, oldValue, newValue, text);
    }

    template <typename EditFunction>
    bool editItemInList(const ArgsT& listArgs, index_type index,
                        const QString& text,
                        EditFunction editFunction)
    {
        QUndoCommand* e = editItemInListCommand(listArgs, index, text, editFunction);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    template <typename EditFunction>
    bool editItemInSelectedList(index_type index,
                                const QString& text,
                                EditFunction editFunction)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();

        return editItemInList(listArgs, index, text, editFunction);
    }

    // will return nullptr if data cannot be accessed or is equal to newValue
    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    QUndoCommand* editFieldCommand(const ArgsT& listArgs, index_type index, const FieldT& newValue,
                                   const QString& text,
                                   UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        ListT* list = getList_NO_CONST(listArgs);
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
        return new EditFieldCommand<FieldT, UnaryFunction, ExtraSignalsFunction>(
            _accessor, listArgs, index, oldValue, newValue, text, getter, extraSignals);
    }

    // will return nullptr if data cannot be accessed or is equal to newValue
    template <typename FieldT, typename UnaryFunction>
    QUndoCommand* editFieldCommand(const ArgsT& listArgs, index_type index, const FieldT& newValue,
                                   const QString& text,
                                   UnaryFunction getter)
    {
        return editFieldCommand(listArgs, index, newValue, text, getter, EmptySignalFunction<ArgsT>());
    }

    template <typename FieldT, typename UnaryFunction>
    bool editField(const ArgsT& listArgs, index_type index, const FieldT& newValue,
                   const QString& text,
                   UnaryFunction getter)
    {
        QUndoCommand* e = editFieldCommand(listArgs, index, newValue, text, getter);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    bool editField(const ArgsT& listArgs, index_type index, const FieldT& newValue,
                   const QString& text,
                   UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        QUndoCommand* e = editFieldCommand(listArgs, index, newValue, text, getter, extraSignals);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    template <typename FieldT, typename UnaryFunction>
    bool editFieldInSelectedList(index_type index, const FieldT& newValue,
                                 const QString& text,
                                 UnaryFunction getter)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        return editField(listArgs, index, newValue, text, getter);
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    bool editFieldInSelectedList(index_type index, const FieldT& newValue,
                                 const QString& text,
                                 UnaryFunction getter,
                                 ExtraSignalsFunction extraSignals)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        return editField(listArgs, index, newValue, text, getter, extraSignals);
    }

    // will return nullptr if data cannot be accessed or is equal to newValues
    template <typename... FieldT, typename UnaryFunction>
    QUndoCommand* editMulitpleFieldsCommand(const ArgsT& listArgs, index_type index,
                                            const std::tuple<FieldT...>& newValues,
                                            const QString& text,
                                            UnaryFunction getter)
    {
        ListT* list = getList_NO_CONST(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }
        DataT& item = list->at(index);
        const std::tuple<FieldT&...> oldValues = getter(item);

        if (oldValues == newValues) {
            return nullptr;
        }
        return new EditMultipleFieldsCommand<UnaryFunction, FieldT...>(
            _accessor, listArgs, index, oldValues, newValues, text, getter);
    }

    template <typename... FieldT, typename UnaryFunction>
    bool editMultipleFields(const ArgsT& listArgs, index_type index,
                            const std::tuple<FieldT...>& newValues,
                            const QString& text,
                            UnaryFunction getter)
    {
        QUndoCommand* c = editMulitpleFieldsCommand(listArgs, index, newValues, text, getter);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    template <typename... FieldT, typename UnaryFunction>
    bool editMultipleFieldsInSelectedList(index_type index, const std::tuple<FieldT...>& newValues,
                                          const QString& text,
                                          UnaryFunction getter)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        return editMultipleFields(listArgs, index, newValues, text, getter);
    }

    // The caller is responsible for setting the new value of the command and
    // releasing it into the undo stack.
    // Will return nullptr if field cannot be accessed
    template <typename FieldT, typename UnaryFunction>
    std::unique_ptr<EditFieldIncompleteCommand<FieldT, UnaryFunction>>
    editFieldIncompleteCommand(const ArgsT& listArgs, index_type index,
                               const QString& text,
                               UnaryFunction getter)
    {
        ListT* list = getList_NO_CONST(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }
        const FieldT& oldValue = getter(list->at(index));

        return std::make_unique<EditFieldIncompleteCommand<FieldT, UnaryFunction>>(
            _accessor, listArgs, index, oldValue, text, getter);
    }

    // will return nullptr if data cannot be accessed or unchanged
    template <typename EditFunction>
    QUndoCommand* editAllItemsInListCommand(const ArgsT& listArgs, const QString& text,
                                            EditFunction editFunction)
    {
        const ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }

        std::vector<index_type> indexesEdited;
        std::vector<DataT> oldValues;
        std::vector<DataT> newValues;

        indexesEdited.reserve(list->size());
        oldValues.reserve(list->size());
        newValues.reserve(list->size());

        for (index_type index = 0; index < list->size(); index++) {
            const DataT& oldValue = list->at(index);
            DataT newValue = oldValue;

            editFunction(newValue, index);

            if (newValue != oldValue) {
                indexesEdited.push_back(index);
                oldValues.push_back(oldValue);
                newValues.push_back(std::move(newValue));
            }
        }

        if (!indexesEdited.empty()) {
            return new typename ListUndoHelper<AccessorT>::EditMultipleCommand(
                this->_accessor, listArgs,
                std::move(indexesEdited), std::move(oldValues), std::move(newValues),
                text);
        }
        else {
            return nullptr;
        }
    }

    template <typename EditFunction>
    QUndoCommand* editAllItemsInSelectedListCommand(const QString& text, EditFunction editFunction)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        return editAllItemsCommand(listArgs, text, editFunction);
    }

    template <typename EditFunction>
    bool editAllItemsInList(const ArgsT& listArgs, const QString& text,
                            EditFunction editFunction)
    {
        QUndoCommand* c = editAllItemsInListCommand(listArgs, text, editFunction);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    template <typename EditFunction>
    bool editAllItemsInSelectedList(const QString& text, EditFunction editFunction)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();

        return editAllItemsInList(listArgs, text, editFunction);
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* addCommand(const ArgsT& listArgs, index_type index)
    {
        const ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index > list->size()) {
            return nullptr;
        }
        if (list->size() >= _accessor->maxSize()) {
            return nullptr;
        }

        return new AddCommand(_accessor, listArgs, index);
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* addCommand(const ArgsT& listArgs, index_type index, DataT item, const QString& text)
    {
        const ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index > list->size()) {
            return nullptr;
        }
        if (list->size() >= _accessor->maxSize()) {
            return nullptr;
        }

        return new AddCommand(_accessor, listArgs, index, item, text);
    }

    bool addItem(const ArgsT& listArgs)
    {
        const ListT* list = getList(listArgs);
        if (list == nullptr) {
            return false;
        }
        index_type index = list->size();

        QUndoCommand* c = addCommand(listArgs, index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    bool addItem(const ArgsT& listArgs, index_type index)
    {
        QUndoCommand* c = addCommand(listArgs, index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    bool addItem(const ArgsT& listArgs, index_type index, DataT item, const QString& text)
    {
        QUndoCommand* c = addCommand(listArgs, index, std::move(item), text);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    bool addItemToSelectedList()
    {
        const ArgsT listArgs = _accessor->selectedListTuple();

        return addItem(listArgs);
    }

    bool addItemToSelectedList(index_type index)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();

        return addItem(listArgs, index);
    }

    bool addItemToSelectedList(index_type index, DataT item, const QString& text)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();

        return addItem(listArgs, index, std::move(item), text);
    }

    bool addItemToSelectedList(index_type index, DataT item)
    {
        return addItemToSelectedList(index, item, tr("Add %1").arg(_accessor->typeName()));
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* cloneCommand(const ArgsT& listArgs, index_type index)
    {
        const ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }
        if (list->size() >= _accessor->maxSize()) {
            return nullptr;
        }

        return new AddCommand(_accessor, listArgs, index + 1, list->at(index),
                              tr("Clone %1").arg(_accessor->typeName()));
    }

    bool cloneItem(const ArgsT& listArgs, index_type index)
    {
        QUndoCommand* c = cloneCommand(listArgs, index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    bool cloneItemInSelectedList(index_type index)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();

        return cloneItem(listArgs, index);
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* removeCommand(const ArgsT& listArgs, index_type index)
    {
        const ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }

        return new RemoveCommand(_accessor, listArgs, index, list->at(index));
    }

    bool removeItem(const ArgsT& listArgs, index_type index)
    {
        QUndoCommand* c = removeCommand(listArgs, index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    bool removeItemFromSelectedList(index_type index)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();

        return removeItem(listArgs, index);
    }

    // will return nullptr if list cannot be accessed or indexes are invalid
    QUndoCommand* moveCommand(const ArgsT& listArgs, index_type from, index_type to,
                              const QString& text)
    {
        const ListT* list = getList(listArgs);
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

    bool moveItem(const ArgsT& listArgs, index_type from, index_type to)
    {
        QUndoCommand* c = moveCommand(listArgs, from, to);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    bool moveItem(const ArgsT& listArgs, index_type from, index_type to, const QString& text)
    {
        QUndoCommand* c = moveCommand(listArgs, from, to, text);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    bool moveItemInSelectedList(index_type from, index_type to)
    {
        const ArgsT listArgs = this->_accessor->selectedListTuple();

        return moveItem(listArgs, from, to);
    }

    bool moveItemInSelectedList(index_type from, index_type to, const QString& text)
    {
        const ArgsT listArgs = this->_accessor->selectedListTuple();

        return moveItem(listArgs, from, to, text);
    }
};

template <class AccessorT>
class ListAndSelectionUndoHelper : public ListUndoHelper<AccessorT> {
public:
    using DataT = typename AccessorT::DataT;
    using ListT = typename AccessorT::ListT;
    using index_type = typename AccessorT::index_type;
    using ArgsT = typename AccessorT::ArgsT;

public:
    ListAndSelectionUndoHelper(AccessorT* accessor)
        : ListUndoHelper<AccessorT>(accessor)
    {
    }

    bool editSelectedItem(const DataT& newValue)
    {
        const ArgsT listArgs = this->selectedListTuple();
        const index_type index = this->_accessor->selectedIndex();

        return this->edit(listArgs, index, newValue);
    }

    template <typename FieldT, typename UnaryFunction>
    bool editSelectedItemField(const FieldT& newValue,
                               const QString& text,
                               UnaryFunction getter)
    {
        const ArgsT listArgs = this->selectedListTuple();
        const index_type index = this->_accessor->selectedIndex();

        return this->editField(listArgs, index, newValue, text, getter);
    }

    template <typename FieldT, typename UnaryFunction>
    QUndoCommand* editSelectedItemFieldCommand(const FieldT& newValue,
                                               const QString& text,
                                               UnaryFunction getter)
    {
        const ArgsT listArgs = this->selectedListTuple();
        const index_type index = this->_accessor->selectedIndex();

        return this->editFieldCommand(listArgs, index, newValue, text, getter);
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    QUndoCommand* editSelectedItemFieldCommand(const FieldT& newValue,
                                               const QString& text,
                                               UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        const ArgsT listArgs = this->selectedListTuple();
        const index_type index = this->_accessor->selectedIndex();

        return this->editFieldCommand(listArgs, index, newValue, text, getter, extraSignals);
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    bool editSelectedItemField(const FieldT& newValue,
                               const QString& text,
                               UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        const ArgsT listArgs = this->selectedListTuple();
        const index_type index = this->_accessor->selectedIndex();

        return this->editField(listArgs, index, newValue, text, getter, extraSignals);
    }

    template <typename... FieldT, typename UnaryFunction>
    bool editSelectedItemMultipleFields(const std::tuple<FieldT...>& newValues,
                                        const QString& text,
                                        UnaryFunction getter)
    {
        const index_type index = this->_accessor->selectedIndex();
        return this->editMultipleFieldsInSelectedList(index, newValues, text, getter);
    }

    // will return nullptr if data cannot be accessed
    template <typename FieldT, typename UnaryFunction>
    auto editSelectedFieldIncompleteCommand(const QString& text,
                                            UnaryFunction getter)
    {
        const ArgsT listArgs = this->selectedListTuple();
        const index_type index = this->_accessor->selectedIndex();

        return this->template editFieldIncompleteCommand<FieldT>(
            listArgs, index, text, getter);
    }

    bool addItemToSelectedList(index_type index)
    {
        const ArgsT listArgs = this->selectedListTuple();

        bool s = this->addItem(listArgs, index);
        if (s) {
            this->_accessor->setSelectedIndex(index);
        }
        return s;
    }

    bool addItemToSelectedList()
    {
        const ArgsT listArgs = this->selectedListTuple();

        const ListT* list = this->getList(listArgs);
        if (list == nullptr) {
            return false;
        }
        index_type index = list->size();

        bool s = this->addItem(listArgs, index);
        if (s) {
            this->_accessor->setSelectedIndex(index);
        }
        return s;
    }

    bool cloneSelectedItem()
    {
        const ArgsT listArgs = this->selectedListTuple();
        const index_type index = this->_accessor->selectedIndex();

        bool s = this->cloneItem(listArgs, index);
        if (s) {
            this->_accessor->setSelectedIndex(index + 1);
        }
        return s;
    }

    bool removeSelectedItem()
    {
        const ArgsT listArgs = this->selectedListTuple();
        const index_type index = this->_accessor->selectedIndex();

        return this->removeItem(listArgs, index);
    }

    bool raiseSelectedItemToTop()
    {
        const ArgsT listArgs = this->selectedListTuple();
        const index_type index = this->_accessor->selectedIndex();

        if (index > 0) {
            return this->moveItem(listArgs, index, 0,
                                  this->tr("Raise To Top"));
        }
        else {
            return false;
        }
    }

    bool raiseSelectedItem()
    {
        const ArgsT listArgs = this->selectedListTuple();
        const index_type index = this->_accessor->selectedIndex();

        if (index > 0) {
            return this->moveItem(listArgs, index, index - 1,
                                  this->tr("Raise"));
        }
        else {
            return false;
        }
    }

    bool lowerSelectedItem()
    {
        const ArgsT listArgs = this->selectedListTuple();
        const index_type index = this->_accessor->selectedIndex();

        return this->moveItem(listArgs, index, index + 1,
                              this->tr("Lower"));
    }

    bool lowerSelectedItemToBottom()
    {
        const ArgsT listArgs = this->selectedListTuple();
        const index_type index = this->_accessor->selectedIndex();

        const ListT* list = this->getList(listArgs);
        if (list == nullptr) {
            return false;
        }

        index_type list_size = list->size();
        if (list_size > 1) {
            return this->moveItem(listArgs, index, list_size - 1,
                                  this->tr("Lower To Bottom"));
        }
        else {
            return false;
        }
    }
};
}
}
}
