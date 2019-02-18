/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/call.h"
#include "models/common/namedlist.h"
#include <QCoreApplication>
#include <QUndoCommand>
#include <functional>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

template <class T>
class NamedListAndSelectionUndoHelper;

template <class AccessorT>
class NamedListUndoHelper {

public:
    using DataT = typename AccessorT::DataT;
    using ListT = UnTech::NamedList<DataT>;
    using index_type = typename ListT::size_type;

    constexpr static index_type max_size = AccessorT::max_size;

    static_assert(std::is_same<index_type, typename AccessorT::index_type>::value, "Invalid index_type in Accessor");

    friend class NamedListAndSelectionUndoHelper<AccessorT>;

private:
    static inline QString tr(const char* s)
    {
        return QCoreApplication::tr(s);
    }

    struct EmptySignalFunction {
        inline void operator()(AccessorT*, index_type) const {}
    };

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    class EditFieldCommand : public QUndoCommand {
    private:
        AccessorT* const _accessor;
        const index_type _index;
        const FieldT _oldValue;
        const FieldT _newValue;
        const UnaryFunction _getter;
        const ExtraSignalsFunction _signalEmitter;

    public:
        EditFieldCommand(AccessorT* accessor, index_type index,
                         const FieldT& oldValue, const FieldT& newValue,
                         const QString& text,
                         UnaryFunction getter, ExtraSignalsFunction signalEmitter)
            : QUndoCommand(text)
            , _accessor(accessor)
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
            ListT* list = _accessor->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            DataT& item = list->at(_index);
            FieldT& field = _getter(item);
            field = _oldValue;

            emit _accessor->dataChanged(_index);
            _signalEmitter(_accessor, _index);

            emit _accessor->resourceItem()->dataChanged();
        }

        virtual void redo() final
        {
            ListT* list = _accessor->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            DataT& item = list->at(_index);
            FieldT& field = _getter(item);
            field = _newValue;

            emit _accessor->dataChanged(_index);
            _signalEmitter(_accessor, _index);

            emit _accessor->resourceItem()->dataChanged();
        }
    };

    template <typename FieldT, typename UnaryFunction>
    class EditFieldIncompleteCommand : public QUndoCommand {
    private:
        AccessorT* const _accessor;
        const index_type _index;
        const FieldT _oldValue;
        FieldT _newValue;
        const UnaryFunction _getter;

    public:
        EditFieldIncompleteCommand(AccessorT* accessor, index_type index,
                                   const FieldT& oldValue,
                                   const QString& text,
                                   UnaryFunction getter)
            : QUndoCommand(text)
            , _accessor(accessor)
            , _index(index)
            , _oldValue(oldValue)
            , _newValue(oldValue)
            , _getter(getter)
        {
        }
        ~EditFieldIncompleteCommand() = default;

        virtual void undo() final
        {
            ListT* list = _accessor->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            DataT& item = list->at(_index);
            FieldT& field = _getter(item);
            field = _oldValue;

            emit _accessor->dataChanged(_index);
            emit _accessor->resourceItem()->dataChanged();
        }

        virtual void redo() final
        {
            ListT* list = this->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            DataT& item = *list->at(_index);
            FieldT& field = _getter(item);
            field = _newValue;

            emit _accessor->dataChanged(_index);
            emit _accessor->resourceItem()->dataChanged();
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

    class AddRemoveCommand : public QUndoCommand {
    private:
        AccessorT* const _accessor;
        const index_type _index;
        DataT _value;

    protected:
        AddRemoveCommand(AccessorT* accessor, index_type index, DataT value,
                         const QString& text)
            : QUndoCommand(text)
            , _accessor(accessor)
            , _index(index)
            , _value(std::move(value))
        {
        }
        ~AddRemoveCommand() = default;

        void addItem()
        {
            ListT* list = _accessor->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index <= list->size());

            emit _accessor->listAboutToChange();

            list->insert(_index, _value);

            emit _accessor->itemAdded(_index);
            emit _accessor->listChanged();
            emit _accessor->resourceItem()->dataChanged();
        }

        void removeItem()
        {
            ListT* list = _accessor->getList();
            Q_ASSERT(list);
            Q_ASSERT(_index >= 0 && _index < list->size());

            emit _accessor->listAboutToChange();
            emit _accessor->itemAboutToBeRemoved(_index);

            list->remove(_index);

            emit _accessor->listChanged();
            emit _accessor->resourceItem()->dataChanged();
        }
    };

    class AddCommand : public AddRemoveCommand {
    public:
        AddCommand(AccessorT* accessor, index_type index, DataT data)
            : AddRemoveCommand(accessor, index, std::move(data),
                               tr("Add %1").arg(accessor->typeName()))
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
        RemoveCommand(AccessorT* accessor, index_type index, DataT value)
            : AddRemoveCommand(accessor, index, std::move(value),
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

    class MoveCommand : public QUndoCommand {
    private:
        AccessorT* const _accessor;
        const index_type _fromIndex;
        const index_type _toIndex;

    public:
        MoveCommand(AccessorT* accessor,
                    index_type fromIndex, index_type toIndex,
                    const QString& text)
            : QUndoCommand(text)
            , _accessor(accessor)
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
            ListT* list = _accessor->getList();
            Q_ASSERT(list);
            Q_ASSERT(from != to);
            Q_ASSERT(from >= 0 && from < list->size());
            Q_ASSERT(to >= 0 && to < list->size());

            emit _accessor->listAboutToChange();

            list->moveItem(from, to);

            emit _accessor->itemMoved(from, to);
            emit _accessor->listChanged();
            emit _accessor->resourceItem()->dataChanged();
        }
    };

private:
    AccessorT* const _accessor;

public:
    NamedListUndoHelper(AccessorT* accessor)
        : _accessor(accessor)
    {
    }

private:
    inline const ListT* getList()
    {
        return _accessor->list();
    }

    // MUST ONLY this function when it's absolutely necessary
    inline ListT* getList_NO_CONST()
    {
        return _accessor->getList();
    }

public:
    // will return nullptr if data cannot be accessed or is equal to newValue
    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    QUndoCommand* editFieldCommand(index_type index, const FieldT& newValue,
                                   const QString& text,
                                   UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        ListT* list = getList_NO_CONST();
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }
        DataT& item = list->at(index);
        const FieldT& oldValue = getter(item);

        if (oldValue == newValue) {
            return nullptr;
        }
        return new EditFieldCommand<FieldT, UnaryFunction, ExtraSignalsFunction>(
            _accessor, index, oldValue, newValue, text, getter, extraSignals);
    }

    // will return nullptr if data cannot be accessed or is equal to newValue
    template <typename FieldT, typename UnaryFunction>
    QUndoCommand* editFieldCommand(index_type index, const FieldT& newValue,
                                   const QString& text,
                                   UnaryFunction getter)
    {
        return editFieldCommand(index, newValue, text, getter, EmptySignalFunction());
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    bool editField(index_type index, const FieldT& newValue,
                   const QString& text,
                   UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        QUndoCommand* e = editFieldCommand(index, newValue, text, getter, extraSignals);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    template <typename FieldT, typename UnaryFunction>
    bool editField(index_type index, const FieldT& newValue,
                   const QString& text,
                   UnaryFunction getter)
    {
        QUndoCommand* e = editFieldCommand(index, newValue, text, getter);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    // The caller is responsible for setting the new value of the command and
    // releasing it into the undo stack.
    // Will return nullptr if field cannot be accessed
    template <typename FieldT, typename UnaryFunction>
    std::unique_ptr<EditFieldIncompleteCommand<FieldT, UnaryFunction>>
    editFieldIncompleteCommand(index_type index,
                               const QString& text,
                               UnaryFunction getter)
    {
        ListT* list = getList_NO_CONST();
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }
        DataT& item = *list->at(index);
        const FieldT& oldValue = getter(item);

        return std::make_unique<EditFieldIncompleteCommand<FieldT, UnaryFunction>>(
            _accessor, index, oldValue, text, getter);
    }

    bool renameCommand(index_type index, const idstring& newName)
    {
        return this->editFieldCommand(
            index, newName,
            tr("Edit name"),
            [](DataT& d) -> idstring& { return d.name; },
            [](AccessorT* a, index_type i) { emit a->nameChanged(i); });
    }

    bool renameItem(index_type index, const idstring& newName)
    {
        return this->editField(
            index, newName,
            tr("Edit name"),
            [](DataT& d) -> idstring& { return d.name; },
            [](AccessorT* a, index_type i) { emit a->nameChanged(i); });
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* addCommand(index_type index)
    {
        const ListT* list = getList();
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index > list->size()) {
            return nullptr;
        }
        if (list->size() >= max_size) {
            return nullptr;
        }

        return new AddCommand(_accessor, index, DataT{});
    }

    bool addItem()
    {
        const ListT* list = getList();
        if (list == nullptr) {
            return false;
        }
        index_type index = list->size();

        QUndoCommand* c = addCommand(index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    bool addItem(index_type index)
    {
        QUndoCommand* c = addCommand(index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* addCommand(index_type index, const idstring& name)
    {
        const ListT* list = getList();
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index > list->size()) {
            return nullptr;
        }
        if (list->size() >= max_size) {
            return nullptr;
        }

        DataT value;
        value.name = name;

        return new AddCommand(_accessor, index, std::move(value));
    }

    bool addItem(const idstring& name)
    {
        const ListT* list = getList();
        if (list == nullptr) {
            return false;
        }
        index_type index = list->size();

        QUndoCommand* c = addCommand(index, name);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    bool addItem(index_type index, const idstring& name)
    {
        QUndoCommand* c = addCommand(index, name);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* removeCommand(index_type index)
    {
        const ListT* list = getList();
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }

        return new RemoveCommand(_accessor, index, list->at(index));
    }

    bool removeItem(index_type index)
    {
        QUndoCommand* c = removeCommand(index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* cloneCommand(index_type index)
    {
        const ListT* list = getList();
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }
        if (list->size() >= max_size) {
            return nullptr;
        }

        return new AddCommand(_accessor, index + 1, list->at(index));
    }

    bool cloneItem(index_type index)
    {
        QUndoCommand* c = cloneCommand(index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* cloneCommand(index_type index, const idstring& name)
    {
        const ListT* list = getList();
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }
        if (list->size() >= max_size) {
            return nullptr;
        }

        DataT item = list->at(index);
        item.name = name;

        return new AddCommand(_accessor, index + 1, std::move(item));
    }

    bool cloneItem(index_type index, const idstring& name)
    {
        QUndoCommand* c = cloneCommand(index, name);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if list cannot be accessed or indexes are invalid
    QUndoCommand* moveCommand(index_type from, index_type to,
                              const QString& text)
    {
        const ListT* list = getList();
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

        return new MoveCommand(_accessor, from, to, text);
    }

    QUndoCommand* moveCommand(index_type from, index_type to)
    {
        return moveCommand(from, to,
                           tr("Move %1").arg(_accessor->typeName()));
    }

    bool moveItem(index_type from, index_type to)
    {
        QUndoCommand* c = moveCommand(from, to);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    bool moveItem(index_type from, index_type to, const QString& text)
    {
        QUndoCommand* c = moveCommand(from, to, text);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }
};

template <class AccessorT>
class NamedListAndSelectionUndoHelper : public NamedListUndoHelper<AccessorT> {
public:
    using DataT = typename AccessorT::DataT;
    using ListT = typename AccessorT::ListT;
    using index_type = typename AccessorT::index_type;

public:
    NamedListAndSelectionUndoHelper(AccessorT* accessor)
        : NamedListUndoHelper<AccessorT>(accessor)
    {
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    bool editSelectedItemField(const FieldT& newValue,
                               const QString& text,
                               UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        const index_type index = this->_accessor->selectedIndex();

        return this->editField(index, newValue, text, getter, extraSignals);
    }

    template <typename FieldT, typename UnaryFunction>
    bool editSelectedItemField(const FieldT& newValue,
                               const QString& text,
                               UnaryFunction getter)
    {
        const index_type index = this->_accessor->selectedIndex();

        return this->editField(index, newValue, text, getter);
    }

    // will return nullptr if data cannot be accessed
    template <typename FieldT, typename UnaryFunction>
    auto editSelectedFieldIncompleteCommand(const QString& text,
                                            UnaryFunction getter)
    {
        const index_type index = this->_accessor->selectedIndex();

        return this->template editFieldIncompleteCommand<FieldT>(
            index, text, getter);
    }

    bool renameSelectedItem(const idstring& newName)
    {
        const index_type index = this->_accessor->selectedIndex();
        return this->renameItem(index, newName);
    }

    bool addItem()
    {
        bool s = NamedListUndoHelper<AccessorT>::addItem();
        if (s) {
            auto* list = this->_accessor->list();
            if (list) {
                this->_accessor->setSelectedIndex(list->size() - 1);
            }
        }
        return s;
    }

    bool addItem(const idstring& name)
    {
        bool s = NamedListUndoHelper<AccessorT>::addItem(name);
        if (s) {
            auto* list = this->_accessor->list();
            if (list) {
                this->_accessor->setSelectedIndex(list->size() - 1);
            }
        }
        return s;
    }

    bool cloneSelectedItem()
    {
        const index_type index = this->_accessor->selectedIndex();

        bool s = this->cloneItem(index);
        if (s) {
            this->_accessor->setSelectedIndex(index + 1);
        }
        return s;
    }

    bool cloneSelectedItem(const idstring& name)
    {
        const index_type index = this->_accessor->selectedIndex();

        bool s = this->cloneItem(index, name);
        if (s) {
            this->_accessor->setSelectedIndex(index + 1);
        }
        return s;
    }

    bool removeSelectedItem()
    {
        const index_type index = this->_accessor->selectedIndex();

        return this->removeItem(index);
    }

    bool raiseSelectedItemToTop()
    {
        const index_type index = this->_accessor->selectedIndex();

        if (index > 0) {
            return this->moveItem(index, 0,
                                  this->tr("Raise To Top"));
        }
        else {
            return false;
        }
    }

    bool raiseSelectedItem()
    {
        const index_type index = this->_accessor->selectedIndex();

        if (index > 0) {
            return this->moveItem(index, index - 1,
                                  this->tr("Raise"));
        }
        else {
            return false;
        }
    }

    bool lowerSelectedItem()
    {
        const index_type index = this->_accessor->selectedIndex();

        return this->moveItem(index, index + 1,
                              this->tr("Lower"));
    }

    bool lowerSelectedItemToBottom()
    {
        const index_type index = this->_accessor->selectedIndex();

        const ListT* list = this->getList();
        if (list == nullptr) {
            return false;
        }

        index_type list_size = list->size();
        if (list_size > 1) {
            return this->moveItem(index, list_size - 1,
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
