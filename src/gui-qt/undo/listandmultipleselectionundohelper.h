/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "listundohelper.h"
#include "models/common/vectorset.h"

namespace UnTech {
namespace GuiQt {
namespace Undo {

template <class AccessorT>
class ListAndMultipleSelectionUndoHelper : public ListUndoHelper<AccessorT> {

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

    using BaseCommand = typename ListUndoHelper<AccessorT>::BaseCommand;

    class AddRemoveMultipleCommand : public BaseCommand {
    private:
        const vectorset<index_type> _indexes;
        const std::vector<DataT> _values;

    public:
        const vectorset<index_type>& indexes() { return this->_indexes; }

    protected:
        AddRemoveMultipleCommand(AccessorT* accessor, const ArgsT& args,
                                 vectorset<index_type>&& indexes, std::vector<DataT>&& values,
                                 const QString& text)
            : BaseCommand(accessor, args, text)
            , _indexes(indexes)
            , _values(values)
        {
            Q_ASSERT(_indexes.empty() == false);
            Q_ASSERT(_indexes.size() == _values.size());
        }
        ~AddRemoveMultipleCommand() = default;

        void addItems()
        {
            ListT* list = this->getList();
            Q_ASSERT(list);

            this->emitListAboutToChange();

            auto iit = _indexes.begin();
            auto vit = _values.begin();

            for (; iit != _indexes.end(); iit++, vit++) {
                const auto& index = *iit;
                const auto& value = *vit;
                Q_ASSERT(index >= 0 && index <= list->size());

                list->insert(list->begin() + index, value);

                this->emitItemAdded(index);
            }

            this->emitListChanged();
        }

        void removeItems()
        {
            ListT* list = this->getList();
            Q_ASSERT(list);

            this->emitListAboutToChange();

            for (auto it = _indexes.rbegin(); it != _indexes.rend(); it++) {
                const auto& index = *it;
                Q_ASSERT(index >= 0 && index < list->size());

                this->emitItemAboutToBeRemoved(index);

                list->erase(list->begin() + index);
            }

            this->emitListChanged();
        }
    };

    class AddMultipleCommand : public AddRemoveMultipleCommand {
    public:
        AddMultipleCommand(AccessorT* accessor, const ArgsT& args,
                           vectorset<index_type>&& indexes, std::vector<DataT>&& values,
                           const QString& text)
            : AddRemoveMultipleCommand(accessor, args, std::move(indexes), std::move(values), text)
        {
        }
        ~AddMultipleCommand() = default;

        virtual void undo() final
        {
            this->removeItems();
        }

        virtual void redo() final
        {
            this->addItems();
        }
    };

    class RemoveMultipleCommand : public AddRemoveMultipleCommand {
    public:
        RemoveMultipleCommand(AccessorT* accessor, const ArgsT& args,
                              const vectorset<index_type>& indexes, std::vector<DataT>&& values,
                              const QString& text)
            : AddRemoveMultipleCommand(accessor, args, vectorset<index_type>(indexes), std::move(values), text)
        {
        }
        RemoveMultipleCommand(AccessorT* accessor, const ArgsT& args,
                              vectorset<index_type>&& indexes, std::vector<DataT>&& values,
                              const QString& text)
            : AddRemoveMultipleCommand(accessor, args, std::move(indexes), std::move(values), text)
        {
        }
        ~RemoveMultipleCommand() = default;

        virtual void undo() final
        {
            this->addItems();
        }

        virtual void redo() final
        {
            this->removeItems();
        }
    };

    class MoveMultipleCommand : public BaseCommand {
    private:
        const vectorset<index_type> _redoIndexes;
        const vectorset<index_type> _undoIndexes;
        const int _offset;

        static std::vector<index_type> moveSelectedIndexes(const vectorset<index_type>& indexes, int offset)
        {
            std::vector<index_type> ret;
            ret.reserve(indexes.size());

            for (const index_type& i : indexes) {
                ret.push_back(i + offset);
            }

            return ret;
        }

    public:
        MoveMultipleCommand(AccessorT* accessor, const ArgsT& args,
                            const vectorset<index_type>& indexes, int offset,
                            const QString& text)
            : BaseCommand(accessor, args, text)
            , _redoIndexes(indexes)
            , _undoIndexes(moveSelectedIndexes(indexes, offset))
            , _offset(offset)
        {
        }
        ~MoveMultipleCommand() = default;

        void undo()
        {
            moveItems(_undoIndexes, -_offset);
        }

        void redo()
        {
            moveItems(_redoIndexes, _offset);
        }

    private:
        void moveItems(const vectorset<index_type>& indexes, int offset)
        {
            Q_ASSERT(offset != 0);
            if (offset < 0) {
                moveItemsUp(indexes, -offset);
            }
            else if (offset > 0) {
                moveItemsDown(indexes, offset);
            }
        }

        void moveItemsUp(const vectorset<index_type>& indexes, unsigned offset)
        {
            ListT* list = this->getList();
            Q_ASSERT(list);

            Q_ASSERT(offset > 0);
            Q_ASSERT(indexes.empty() == false);
            Q_ASSERT(indexes.front() >= offset);
            Q_ASSERT(indexes.back() < list->size());

            this->emitListAboutToChange();

            for (auto it = indexes.begin(); it != indexes.end(); it++) {
                const auto& index = *it;

                moveListItem(index, index - offset, *list);

                this->emitItemMoved(index, index - offset);
            }

            this->emitListChanged();
        }

        void moveItemsDown(const vectorset<index_type>& indexes, unsigned offset)
        {
            ListT* list = this->getList();
            Q_ASSERT(list);

            Q_ASSERT(offset > 0);
            Q_ASSERT(indexes.empty() == false);
            Q_ASSERT(indexes.front() >= 0);
            Q_ASSERT(indexes.back() + offset < list->size());

            this->emitListAboutToChange();

            for (auto it = indexes.rbegin(); it != indexes.rend(); it++) {
                const auto& index = *it;

                moveListItem(index, index + offset, *list);

                this->emitItemMoved(index, index + offset);
            }

            this->emitListChanged();
        }
    };

public:
    ListAndMultipleSelectionUndoHelper(AccessorT* accessor)
        : ListUndoHelper<AccessorT>(accessor)
    {
    }

    template <typename EditFunction>
    QList<QUndoCommand*> editSelectedCommands(EditFunction editFunction)
    {
        QList<QUndoCommand*> commands;

        const ArgsT listArgs = this->selectedListTuple();
        const ListT* list = this->getList(listArgs);
        if (list == nullptr) {
            return commands;
        }

        const vectorset<index_type>& indexes = this->_accessor->selectedIndexes();

        commands.reserve(indexes.size());

        for (const index_type& index : indexes) {
            if (index >= 0 && index < list->size()) {
                DataT newValue = list->at(index);

                editFunction(newValue, index);

                QUndoCommand* c = this->editCommand(listArgs, index, newValue);
                if (c) {
                    commands.append(c);
                }
            }
        }

        return commands;
    }

    template <typename EditFunction>
    bool editSelectedItems(const QString& text,
                           EditFunction editFunction)
    {
        auto commands = editSelectedCommands(editFunction);
        return pushCommands(commands, text);
    }

    template <typename FieldT, typename UnaryFunction>
    bool setSelectedFields(const FieldT& newValue,
                           const QString& text,
                           UnaryFunction getter)
    {
        const ArgsT listArgs = this->selectedListTuple();
        const vectorset<index_type>& indexes = this->_accessor->selectedIndexes();

        QList<QUndoCommand*> commands;
        commands.reserve(indexes.size());

        for (const index_type& index : indexes) {
            QUndoCommand* c = this->editFieldCommand(listArgs, index, newValue, QString(), getter);
            if (c) {
                commands.append(c);
            }
        }

        return pushCommands(commands, text);
    }

    bool addItemToSelectedList(index_type index)
    {
        const ArgsT listArgs = this->selectedListTuple();

        bool s = this->addItem(listArgs, index);
        if (s) {
            this->_accessor->setSelectedIndexes({ index });
        }
        return s;
    }

    bool addItemToSelectedList()
    {
        const ArgsT listArgs = this->selectedListTuple();

        ListT* list = this->getList(listArgs);
        if (list == nullptr) {
            return false;
        }
        index_type index = list->size();

        bool s = this->addItem(listArgs, index);
        if (s) {
            this->_accessor->setSelectedIndexes({ index });
        }
        return s;
    }

    // will return nullptr if list is not accessable or indexes are invalid
    QUndoCommand* cloneMultipleCommand(const ArgsT& listArgs, const vectorset<index_type>& indexes)
    {
        if (indexes.empty()) {
            return nullptr;
        }
        ListT* list = this->getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (indexes.front() < 0 || indexes.back() >= list->size()) {
            return nullptr;
        }
        if (list->size() + indexes.size() > max_size) {
            return nullptr;
        }

        std::vector<index_type> newIndexes;
        newIndexes.reserve(indexes.size());
        for (auto it = indexes.begin(); it != indexes.end(); it++) {
            newIndexes.push_back(*it + std::distance(indexes.begin(), it) + 1);
        }

        std::vector<DataT> newValues;
        newValues.reserve(indexes.size());
        for (const index_type& i : indexes) {
            newValues.emplace_back(list->at(i));
        }

        return new AddMultipleCommand(this->_accessor, listArgs, std::move(newIndexes), std::move(newValues),
                                      tr("Clone %1").arg(this->_accessor->typeNamePlural()));
    }

    bool cloneMultipleItems(const ArgsT& listArgs, const vectorset<index_type>& indexes)
    {
        QUndoCommand* c = cloneMultipleCommand(listArgs, indexes);
        if (c) {
            this->_accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    bool cloneSelectedItems()
    {
        const ArgsT listArgs = this->selectedListTuple();
        const vectorset<index_type>& indexes = this->_accessor->selectedIndexes();

        QUndoCommand* c = cloneMultipleCommand(listArgs, indexes);
        if (c) {
            this->_accessor->resourceItem()->undoStack()->push(c);

            auto* cc = dynamic_cast<AddRemoveMultipleCommand*>(c);
            Q_ASSERT(cc);
            this->_accessor->setSelectedIndexes(cc->indexes());
        }
        return c != nullptr;
    }

    // will return nullptr if list is not accessable or indexes are invalid
    QUndoCommand* removeMultipleCommand(const ArgsT& listArgs, const vectorset<index_type>& indexes)
    {
        if (indexes.empty()) {
            return nullptr;
        }
        ListT* list = this->getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (indexes.front() < 0 || indexes.back() >= list->size()) {
            return nullptr;
        }
        if (list->size() + indexes.size() > max_size) {
            return nullptr;
        }

        std::vector<DataT> values;
        values.reserve(indexes.size());
        for (const index_type& i : indexes) {
            values.emplace_back(list->at(i));
        }

        return new RemoveMultipleCommand(this->_accessor, listArgs, indexes, std::move(values),
                                         tr("Remove %1").arg(this->_accessor->typeNamePlural()));
    }

    bool removeMultipleItems(const ArgsT& listArgs, const vectorset<index_type>& indexes)
    {
        QUndoCommand* c = removeMultipleCommand(listArgs, indexes);
        if (c) {
            this->_accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    bool removeSelectedItems()
    {
        const ArgsT listArgs = this->selectedListTuple();
        const vectorset<index_type>& indexes = this->_accessor->selectedIndexes();

        return this->removeMultipleItems(listArgs, indexes);
    }

    // will return nullptr if list is not accessable or indexes are invalid
    QUndoCommand* moveMultipleCommand(const ArgsT& listArgs, const vectorset<index_type>& indexes, int offset,
                                      const QString& text)
    {
        if (indexes.empty()) {
            return nullptr;
        }
        if (offset == 0) {
            return nullptr;
        }

        ListT* list = this->getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (indexes.front() < 0 || indexes.back() >= list->size()) {
            return nullptr;
        }
        if (offset > 0 && indexes.back() + index_type(offset) >= list->size()) {
            return nullptr;
        }
        if (offset < 0 && indexes.front() < index_type(-offset)) {
            return nullptr;
        }

        return new MoveMultipleCommand(this->_accessor, listArgs, indexes, offset, text);
    }

    bool moveMultipleItems(const ArgsT& listArgs, const vectorset<index_type>& indexes, int offset,
                           const QString& text)
    {
        QUndoCommand* c = moveMultipleCommand(listArgs, indexes, offset, text);
        if (c) {
            this->_accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    bool moveMultipleItems(const ArgsT& listArgs, const vectorset<index_type>& indexes, int offset)
    {
        return moveMultipleItems(listArgs, indexes, offset,
                                 tr("Move %1").arg(this->_accessor->typeNamePlural()));
    }

    bool raiseSelectedItems()
    {
        const ArgsT listArgs = this->selectedListTuple();
        const vectorset<index_type>& indexes = this->_accessor->selectedIndexes();

        return this->moveMultipleItems(listArgs, indexes, -1,
                                       tr("Raise %1").arg(this->_accessor->typeNamePlural()));
    }

    bool lowerSelectedItems()
    {
        const ArgsT listArgs = this->selectedListTuple();
        const vectorset<index_type>& indexes = this->_accessor->selectedIndexes();

        return this->moveMultipleItems(listArgs, indexes, +1,
                                       tr("Lower %1").arg(this->_accessor->typeNamePlural()));
    }

private:
    bool pushCommands(const QList<QUndoCommand*>& commands, const QString& text)
    {
        if (commands.isEmpty() == false) {
            QUndoStack* undoStack = this->_accessor->resourceItem()->undoStack();

            undoStack->beginMacro(text);
            for (QUndoCommand* c : commands) {
                undoStack->push(c);
            }
            undoStack->endMacro();

            return true;
        }
        else {
            return false;
        }
    }
};
}
}
}
