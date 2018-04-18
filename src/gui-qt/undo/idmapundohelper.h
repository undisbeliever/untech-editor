/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/call.h"
#include "models/common/idmap.h"
#include <QCoreApplication>
#include <QUndoCommand>
#include <functional>
#include <type_traits>

namespace UnTech {
namespace GuiQt {
namespace Undo {

template <class AccessorT>
class IdmapUndoHelper {

public:
    using DataT = typename AccessorT::DataT;
    using MapT = typename AccessorT::MapT;

    static_assert(std::is_same<MapT, idmap<DataT>>::value, "Unexpeted map type");

    friend class IdmapAndSelectionUndoHelper<AccessorT>;

private:
    static inline QString tr(const char* s)
    {
        return QCoreApplication::tr(s);
    }

    struct EmptySignalFunction {
        void operator()(AccessorT*, const DataT*) const {}
    };

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    class EditFieldCommand : public QUndoCommand {
    private:
        AccessorT* const _accessor;
        DataT* const _item;
        const FieldT _oldValue;
        const FieldT _newValue;
        const UnaryFunction _getter;
        const ExtraSignalsFunction _signalEmitter;

    public:
        EditFieldCommand(AccessorT* accessor, DataT* item,
                         const FieldT& oldValue, const FieldT& newValue,
                         const QString& text,
                         UnaryFunction getter, ExtraSignalsFunction signalEmitter)
            : QUndoCommand(text)
            , _accessor(accessor)
            , _item(item)
            , _oldValue(oldValue)
            , _newValue(newValue)
            , _getter(getter)
            , _signalEmitter(signalEmitter)
        {
            Q_ASSERT(_item);
        }
        ~EditFieldCommand() = default;

        virtual void undo() final
        {
            _getter(*_item) = _oldValue;

            emit _accessor->dataChanged(static_cast<const DataT*>(_item));
            _signalEmitter(_accessor, static_cast<const DataT*>(_item));

            emit _accessor->resourceItem()->dataChanged();
        }

        virtual void redo() final
        {
            _getter(*_item) = _newValue;

            emit _accessor->dataChanged(static_cast<const DataT*>(_item));
            _signalEmitter(_accessor, static_cast<const DataT*>(_item));

            emit _accessor->resourceItem()->dataChanged();
        }
    };

    class AddRemoveCommand : public QUndoCommand {
    private:
        AccessorT* const _accessor;
        const idstring _id;
        std::unique_ptr<DataT> _item;

    protected:
        AddRemoveCommand(AccessorT* accessor, const idstring& id,
                         std::unique_ptr<DataT> value,
                         const QString& text)
            : QUndoCommand(text)
            , _accessor(accessor)
            , _id(id)
            , _item(std::move(value))
        {
        }
        ~AddRemoveCommand() = default;

        void addItem()
        {
            MapT* map = _accessor->getMap();
            Q_ASSERT(map);
            Q_ASSERT(_item != nullptr);
            Q_ASSERT(map->contains(_id) == false);

            emit _accessor->mapAboutToChange();

            map->insertInto(_id, std::move(_item));

            emit _accessor->itemAdded(_id);
            emit _accessor->mapChanged();
            emit _accessor->resourceItem()->dataChanged();
        }

        void removeItem()
        {
            MapT* map = _accessor->getMap();
            Q_ASSERT(map);
            Q_ASSERT(_item == nullptr);
            Q_ASSERT(map->contains(_id));

            emit _accessor->mapAboutToChange();
            emit _accessor->itemAboutToBeRemoved(_id);

            _item = map->extractFrom(_id);

            emit _accessor->mapChanged();
            emit _accessor->resourceItem()->dataChanged();
        }
    };

    class AddCommand : public AddRemoveCommand {
    public:
        AddCommand(AccessorT* accessor, const idstring& id)
            : AddRemoveCommand(accessor, id, std::make_unique<DataT>(),
                               tr("Add %1").arg(accessor->typeName()))
        {
        }

        AddCommand(AccessorT* accessor, const idstring& id, const DataT& value)
            : AddRemoveCommand(accessor, id, std::make_unique<DataT>(value),
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
        RemoveCommand(AccessorT* accessor, const idstring& id)
            : AddRemoveCommand(accessor, id, nullptr,
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

    class RenameCommand : public QUndoCommand {
    private:
        AccessorT* const _accessor;
        const idstring _fromId;
        const idstring _toId;

    public:
        RenameCommand(AccessorT* accessor, const idstring& fromId, const idstring toId)
            : QUndoCommand(tr("Rename %1").arg(accessor->typeName()))
            , _accessor(accessor)
            , _fromId(fromId)
            , _toId(toId)
        {
        }
        ~RenameCommand() = default;

        void undo()
        {
            renameItem(_toId, _fromId);
        }

        void redo()
        {
            renameItem(_fromId, _toId);
        }

    private:
        void renameItem(const idstring& from, const idstring& to)
        {
            MapT* map = _accessor->getMap();
            Q_ASSERT(map);
            Q_ASSERT(map->contains(from) == true);
            Q_ASSERT(map->contains(to) == false);

            emit _accessor->mapAboutToChange();

            map->rename(from, to);

            emit _accessor->itemRenamed(from, to);
            emit _accessor->mapChanged();
            emit _accessor->resourceItem()->dataChanged();
        }
    };

private:
    AccessorT* const _accessor;

public:
    IdmapUndoHelper(AccessorT* accessor)
        : _accessor(accessor)
    {
    }

private:
    inline MapT* getMap() { return _accessor->getMap(); }

public:
    // will return nullptr if data cannot be accessed or is equal to newValue
    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    QUndoCommand* editFieldCommand(const idstring& id, const FieldT& newValue,
                                   const QString& text,
                                   UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        MapT* map = _accessor->getMap();
        if (map == nullptr) {
            return nullptr;
        }
        DataT* item = map->getPtr(id);
        if (item == nullptr) {
            return nullptr;
        }
        const FieldT& oldValue = getter(*item);

        if (oldValue == newValue) {
            return nullptr;
        }
        return new EditFieldCommand<FieldT, UnaryFunction, ExtraSignalsFunction>(
            _accessor, item, oldValue, newValue, text, getter, extraSignals);
    }

    // will return nullptr if data cannot be accessed or is equal to newValue
    template <typename FieldT, typename UnaryFunction>
    QUndoCommand* editFieldCommand(const idstring& id, const FieldT& newValue,
                                   const QString& text,
                                   UnaryFunction getter)
    {
        return editFieldCommand(id, newValue, text, getter, EmptySignalFunction());
    }

    template <typename FieldT, typename UnaryFunction>
    bool editField(const idstring& id, const FieldT& newValue,
                   const QString& text,
                   UnaryFunction getter)
    {
        QUndoCommand* c = editFieldCommand(id, newValue, text, getter);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    bool editField(const idstring& id, const FieldT& newValue,
                   const QString& text,
                   UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        QUndoCommand* c = editFieldCommand(id, newValue, text, getter, extraSignals);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if map cannot be accessed, id is invalid,
    // or newId already exists
    QUndoCommand* addCommand(const idstring& newId)
    {
        if (newId.isValid() == false) {
            return nullptr;
        }

        MapT* map = _accessor->getMap();
        if (map == nullptr) {
            return nullptr;
        }
        if (map->contains(newId)) {
            return nullptr;
        }

        return new AddCommand(_accessor, newId);
    }

    bool addItem(const idstring& newId)
    {
        QUndoCommand* c = addCommand(newId);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if map cannot be accessed, id is invalid,
    // or id does not exist or newId already exists
    QUndoCommand* cloneCommand(const idstring& id, const idstring& newId)
    {
        if (newId.isValid() == false) {
            return nullptr;
        }

        MapT* map = _accessor->getMap();
        if (map == nullptr) {
            return nullptr;
        }
        if (map->contains(newId)) {
            return nullptr;
        }

        DataT* item = map->getPtr(id);
        if (item == nullptr) {
            return nullptr;
        }

        return new AddCommand(_accessor, newId, *item);
    }

    bool cloneItem(const idstring& id, const idstring& newId)
    {
        QUndoCommand* c = cloneCommand(id, newId);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if map cannot be accessed, id is invalid,
    // or id does not exist
    QUndoCommand* removeCommand(const idstring& id)
    {
        MapT* map = _accessor->getMap();
        if (map == nullptr) {
            return nullptr;
        }

        if (map->contains(id) == false) {
            return nullptr;
        }

        return new RemoveCommand(_accessor, id);
    }

    bool removeItem(const idstring& id)
    {
        QUndoCommand* c = removeCommand(id);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if map cannot be accessed, newId is invalid,
    // or id does not exist or newId already exists
    QUndoCommand* renameCommand(const idstring& id, const idstring& newId)
    {
        if (newId.isValid() == false) {
            return nullptr;
        }

        MapT* map = _accessor->getMap();
        if (map == nullptr) {
            return nullptr;
        }
        if (map->contains(newId)) {
            return nullptr;
        }
        if (map->contains(id) == false) {
            return nullptr;
        }

        return new RenameCommand(_accessor, id, newId);
    }

    bool renameItem(const idstring& id, const idstring& newId)
    {
        QUndoCommand* c = renameCommand(id, newId);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }
};

template <class AccessorT>
class IdmapAndSelectionUndoHelper : public IdmapUndoHelper<AccessorT> {
public:
    using DataT = typename AccessorT::DataT;
    using MapT = typename AccessorT::MapT;

public:
    IdmapAndSelectionUndoHelper(AccessorT* accessor)
        : IdmapUndoHelper<AccessorT>(accessor)
    {
    }

    template <typename FieldT, typename UnaryFunction>
    QUndoCommand* editSelectedFieldCommand(const FieldT& newValue,
                                           const QString& text,
                                           UnaryFunction getter)
    {
        const idstring id = this->_accessor->selectedId();

        return this->editFieldCommand(id, newValue, text, getter);
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    QUndoCommand* editSelectedFieldCommand(const FieldT& newValue,
                                           const QString& text,
                                           UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        const idstring id = this->_accessor->selectedId();

        return this->editFieldCommand(id, newValue, text, getter, extraSignals);
    }

    template <typename FieldT, typename UnaryFunction>
    bool editSelectedItemField(const FieldT& newValue,
                               const QString& text,
                               UnaryFunction getter)
    {
        const idstring id = this->_accessor->selectedId();

        return this->editField(id, newValue, text, getter);
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    bool editSelectedItemField(const FieldT& newValue, const QString& text,
                               UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        const idstring id = this->_accessor->selectedId();

        return this->editField(id, newValue, text, getter, extraSignals);
    }

    bool addItem(const idstring& newId)
    {
        bool s = IdmapUndoHelper<AccessorT>::addItem(newId);
        if (s) {
            this->_accessor->setSelectedId(newId);
        }
        return s;
    }

    bool cloneItem(const idstring& id, const idstring& newId)
    {
        bool s = IdmapUndoHelper<AccessorT>::cloneItem(id, newId);
        if (s) {
            this->_accessor->setSelectedId(newId);
        }
        return s;
    }

    bool cloneSelectedItem(const idstring& newId)
    {
        const idstring id = this->_accessor->selectedId();

        return cloneItem(id, newId);
    }

    bool removeSelectedItem()
    {
        const idstring id = this->_accessor->selectedId();

        return this->removeItem(id);
    }

    bool renameSelectedItem(const idstring& newId)
    {
        const idstring id = this->_accessor->selectedId();

        return this->renameItem(id, newId);
    }
};
}
}
}
