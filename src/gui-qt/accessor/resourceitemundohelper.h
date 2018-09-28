/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idstring.h"
#include <QCoreApplication>
#include <QUndoCommand>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

template <class ResourceItemT>
class ResourceItemUndoHelper {

public:
    using DataT = typename ResourceItemT::DataT;

private:
    static inline QString tr(const char* s)
    {
        return QCoreApplication::tr(s);
    }

    class EditCommand : public QUndoCommand {
    protected:
        ResourceItemT* const _item;
        const DataT _oldData;
        const DataT _newData;

    public:
        EditCommand(ResourceItemT* item,
                    const DataT& oldData, const DataT& newData,
                    const QString& text)
            : QUndoCommand(text)
            , _item(item)
            , _oldData(oldData)
            , _newData(newData)
        {
        }
        ~EditCommand() = default;

        virtual void undo() final
        {
            _item->setData(_oldData);
        }

        virtual void redo() final
        {
            _item->setData(_newData);
        }
    };

    struct EmptySignalFunction {
        void operator()(ResourceItemT&) const {}
    };

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    class EditFieldCommand : public QUndoCommand {
    protected:
        ResourceItemT* const _item;
        const FieldT _oldValue;
        const FieldT _newValue;
        const UnaryFunction _getter;
        const ExtraSignalsFunction _signalEmitter;

    public:
        EditFieldCommand(ResourceItemT* item,
                         const FieldT& oldValue, const FieldT& newValue,
                         const QString& text,
                         UnaryFunction getter, ExtraSignalsFunction signalEmitter)
            : QUndoCommand(text)
            , _item(item)
            , _oldValue(oldValue)
            , _newValue(newValue)
            , _getter(getter)
            , _signalEmitter(signalEmitter)
        {
        }
        ~EditFieldCommand() = default;

        virtual void undo() final
        {
            setField(_oldValue);
        }

        virtual void redo() final
        {
            setField(_newValue);
        }

    private:
        inline void setField(const FieldT& value)
        {
            Q_ASSERT(_item);
            DataT* d = _item->dataEditable();
            Q_ASSERT(d);

            FieldT& f = _getter(*d);
            f = value;

            _signalEmitter(*_item);
            emit _item->dataChanged();
        }
    };

    class EditNameCommand : public QUndoCommand {
    protected:
        ResourceItemT* const _item;
        const idstring _oldName;
        const idstring _newName;

    public:
        EditNameCommand(ResourceItemT* item,
                        const idstring& oldName, const idstring& newName)
            : QUndoCommand(tr("Edit Name"))
            , _item(item)
            , _oldName(oldName)
            , _newName(newName)
        {
        }
        ~EditNameCommand() = default;

        virtual void undo() final
        {
            setName(_oldName);
        }

        virtual void redo() final
        {
            setName(_newName);
        }

    private:
        inline void setName(const idstring& name)
        {
            Q_ASSERT(_item);
            DataT* d = _item->dataEditable();
            Q_ASSERT(d);

            d->name = name;

            _item->setName(QString::fromStdString(d->name));
            emit _item->dataChanged();
        }
    };

private:
    ResourceItemT* const _resourceItem;

public:
    ResourceItemUndoHelper(ResourceItemT* item)
        : _resourceItem(item)
    {
        Q_ASSERT(item);
    }

public:
    // will return nullptr if oldData is equal to newData
    QUndoCommand* editCommand(const DataT& newData, const QString& text)
    {
        const DataT* oldData = _resourceItem->data();
        if (oldData == nullptr) {
            return nullptr;
        }

        if (*oldData == newData) {
            return nullptr;
        }
        return new EditCommand(_resourceItem, *oldData, newData, text);
    }

    bool edit(const DataT& newData, const QString& text)
    {
        QUndoCommand* c = editCommand(newData, text);
        if (c) {
            _resourceItem->undoStack()->push(c);
        }
        return c != nullptr;
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    QUndoCommand* editFieldCommand(const FieldT& newValue, const QString& text,
                                   UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        DataT* data = _resourceItem->dataEditable();
        if (data == nullptr) {
            return nullptr;
        }
        const FieldT& oldValue = getter(*data);

        if (oldValue == newValue) {
            return nullptr;
        }
        return new EditFieldCommand<FieldT, UnaryFunction, ExtraSignalsFunction>(
            _resourceItem, oldValue, newValue, text, getter, extraSignals);
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    bool editField(const FieldT& newValue, const QString& text,
                   UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        QUndoCommand* c = editFieldCommand(newValue, text, getter, extraSignals);
        if (c) {
            _resourceItem->undoStack()->push(c);
        }
        return c != nullptr;
    }

    template <typename FieldT, typename UnaryFunction>
    QUndoCommand* editFieldCommand(const FieldT& newValue, const QString& text,
                                   UnaryFunction getter)
    {
        return editFieldCommand(newValue, text, getter, EmptySignalFunction());
    }

    template <typename FieldT, typename UnaryFunction>
    bool editField(const FieldT& newValue, const QString& text,
                   UnaryFunction getter)
    {
        return editField(newValue, text, getter, EmptySignalFunction());
    }

    QUndoCommand* editNameCommand(const idstring& newName)
    {
        DataT* data = _resourceItem->dataEditable();
        if (data == nullptr) {
            return nullptr;
        }
        const idstring& oldName = data->name;

        if (oldName == newName) {
            return nullptr;
        }
        return new EditNameCommand(_resourceItem, oldName, newName);
    }

    bool editName(const idstring& newName)
    {
        QUndoCommand* c = editNameCommand(newName);
        if (c) {
            _resourceItem->undoStack()->push(c);
        }
        return c != nullptr;
    }
};
}
}
}
