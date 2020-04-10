/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/vector-helpers.h"
#include "models/common/vectorset.h"
#include <QCoreApplication>
#include <QUndoCommand>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

template <typename AccessorT>
struct BlankNestedSelectionModifier {
    using index_type = typename AccessorT::index_type;
    using selection_type = std::tuple<>;

    inline static selection_type getSelection(const AccessorT*) { return selection_type(); }
    inline static void setSelection(AccessorT*, const selection_type&) {}

    inline static void postAddCommand(AccessorT*, const index_type, const index_type) {}
    inline static void postAddCommand(AccessorT*, const selection_type&) {}

    inline static void itemAdded(selection_type&, const index_type, const index_type) {}
    inline static void itemRemoved(selection_type&, const index_type, const index_type) {}
    inline static void itemMoved(selection_type&, const index_type, const index_type, const index_type, const index_type) {}
};

// ::TODO std::tuple<index_type, vectorset<index_type>> indexes selected modified::

template <typename AccessorT>
struct MultipleNestedSelectionModifier {
    using index_type = typename AccessorT::index_type;
    using index_pair_t = std::pair<index_type, index_type>;
    using selection_type = std::vector<index_pair_t>;

    inline static selection_type getSelection(const AccessorT* a) { return a->selectedIndexes(); }
    inline static void setSelection(AccessorT* a, selection_type&& selection) { a->setSelectedIndexes(std::move(selection)); }

    inline static void postAddCommand(AccessorT* a, const index_type parentIndex, const index_type childIndex) { a->setSelectedIndex(parentIndex, childIndex); }
    inline static void postAddCommand(AccessorT* a, const selection_type& indexes) { a->setSelectedIndexes(indexes); }

    static void itemAdded(selection_type& selection, const index_type parentIndex, const index_type childIndex)
    {
        for (index_pair_t& sel : selection) {
            if (sel.first == parentIndex && sel.second >= childIndex) {
                sel.second++;
            }
        }
    }

    static void itemRemoved(selection_type& selection, const index_type parentIndex, const index_type childIndex)
    {
        selection.erase(std::remove(selection.begin(), selection.end(), index_pair_t{ parentIndex, childIndex }),
                        selection.end());

        for (index_pair_t& sel : selection) {
            if (sel.first == parentIndex && sel.second >= childIndex) {
                sel.second--;
            }
        }
    }

    static void itemMoved(selection_type& selection,
                          const index_type fromParentIndex, const index_type fromChildIndex,
                          const index_type toParentIndex, const index_type toChildIndex)
    {
        if (fromParentIndex == toParentIndex) {
            // Move item to new location within the same parentIndex
            for (index_pair_t& sel : selection) {
                if (sel.first == fromParentIndex) {
                    if (sel.second == fromChildIndex) {
                        sel.second = toChildIndex;
                    }
                    else if (sel.second > fromChildIndex && sel.second <= toChildIndex) {
                        sel.second--;
                    }
                    else if (sel.second >= toChildIndex && sel.second < fromChildIndex) {
                        sel.second++;
                    }
                }
            }
        }
        else {
            // Remove item from one parentIndex, insert into different parentIndex
            for (index_pair_t& sel : selection) {
                if (sel.first == fromParentIndex && sel.second == fromChildIndex) {
                    sel = { toParentIndex, toChildIndex };
                }
                else if (sel.first == fromParentIndex && sel.second > fromChildIndex) {
                    // Remove from item
                    sel.second--;
                }
                else if (sel.first == toParentIndex && sel.second >= toChildIndex) {
                    // Add to item
                    sel.second++;
                }
            }
        }
    }
};

template <class AccessorT, class SelectionModifier>
class NestedListUndoHelper {

public:
    using DataT = typename AccessorT::DataT;
    using ParentT = typename AccessorT::ParentT;
    using ParentListT = typename AccessorT::ParentListT;
    using ChildListT = typename AccessorT::ChildListT;
    using index_type = typename AccessorT::index_type;

private:
    static inline QString tr(const char* s)
    {
        return QCoreApplication::tr(s);
    }

    class BaseCommand : public QUndoCommand {
    protected:
        AccessorT* const _accessor;

    public:
        BaseCommand(AccessorT* accessor, const QString& text)
            : QUndoCommand(text)
            , _accessor(accessor)
        {
        }
        ~BaseCommand() = default;

    protected:
        inline ParentListT* getParentList()
        {
            return _accessor->getParentList();
        }

        inline ChildListT& getChildList(const index_type parentIndex)
        {
            ParentListT* parentList = _accessor->getParentList();
            Q_ASSERT(parentList);
            Q_ASSERT(parentIndex >= 0 && parentIndex < parentList->size());
            ParentT& parentItem = parentList->at(parentIndex);

            return _accessor->getChildList(parentItem);
        }

        inline DataT& getChildItem(const index_type parentIndex, const index_type childIndex)
        {
            ParentListT* parentList = _accessor->getParentList();
            Q_ASSERT(parentList);
            Q_ASSERT(parentIndex >= 0 && parentIndex < parentList->size());
            ParentT& parentItem = parentList->at(parentIndex);

            ChildListT& childList = _accessor->getChildList(parentItem);
            Q_ASSERT(childIndex >= 0 && childIndex < childList.size());
            return childList.at(childIndex);
        }

        inline void emitDataChanged(index_type parentIndex, index_type childIndex)
        {
            emit _accessor->dataChanged(parentIndex, childIndex);

            emit _accessor->resourceItem()->dataChanged();
        }

        template <typename ExtraSignalsFunction>
        inline void emitDataChanged(index_type parentIndex, index_type childIndex, ExtraSignalsFunction extraSignals)
        {
            emit _accessor->dataChanged(parentIndex, childIndex);
            extraSignals(_accessor, parentIndex, childIndex);

            emit _accessor->resourceItem()->dataChanged();
        }

        inline void emitChildListChanged(index_type parentIndex)
        {
            emit _accessor->listChanged(parentIndex);
            emit _accessor->resourceItem()->dataChanged();
        }

        // When this signal is emitted you MUST close all editors
        // accessing the list to prevent data corruption
        inline void emitListAboutToChange(index_type parentIndex)
        {
            emit _accessor->listAboutToChange(parentIndex);
        }

        inline void emitListChanged(index_type parentIndex)
        {
            emit _accessor->listChanged(parentIndex);
        }

        inline void emitItemAdded(index_type parentIndex, index_type childIndex)
        {
            emit _accessor->itemAdded(parentIndex, childIndex);
        }

        inline void emitItemAboutToBeRemoved(index_type parentIndex, index_type childIndex)
        {
            emit _accessor->itemAboutToBeRemoved(parentIndex, childIndex);
        }

        inline void emitItemMoved(index_type fromParentIndex, index_type fromChildIndex, index_type toParentIndex, index_type toChildIndex)
        {
            emit _accessor->itemMoved(fromParentIndex, fromChildIndex, toParentIndex, toChildIndex);
        }
    };

    class EditCommand : public BaseCommand {
    private:
        const index_type _parentIndex;
        const index_type _childIndex;
        const DataT _oldValue;
        const DataT _newValue;

        inline void setValue(const DataT& value)
        {
            DataT& childItem = this->getChildItem(_parentIndex, _childIndex);

            childItem = value;
            this->emitDataChanged(_parentIndex, _childIndex);
        }

    public:
        EditCommand(AccessorT* accessor, index_type parentIndex, index_type childIndex,
                    const DataT& oldValue, const DataT& newValue)
            : BaseCommand(accessor,
                          tr("Edit %1").arg(accessor->typeName()))
            , _parentIndex(parentIndex)
            , _childIndex(childIndex)
            , _oldValue(oldValue)
            , _newValue(newValue)
        {
        }
        EditCommand(AccessorT* accessor, index_type parentIndex, index_type childIndex,
                    const DataT& oldValue, const DataT& newValue,
                    const QString& text)
            : BaseCommand(accessor, text)
            , _parentIndex(parentIndex)
            , _childIndex(childIndex)
            , _oldValue(oldValue)
            , _newValue(newValue)
        {
        }
        ~EditCommand() = default;

        virtual void undo() final
        {
            setValue(_oldValue);
        }

        virtual void redo() final
        {
            setValue(_newValue);
        }
    };

    // This class allows command merging
    class EditMergeCommand : public BaseCommand {
    private:
        const index_type _parentIndex;
        const index_type _childIndex;
        const DataT _oldValue;
        DataT _newValue;
        const bool _first;

        inline void setValue(const DataT& value)
        {
            DataT& childItem = this->getChildItem(_parentIndex, _childIndex);

            childItem = value;
            emit this->_accessor->DataChanged(_parentIndex, _childIndex);
        }

    public:
        EditMergeCommand(AccessorT* accessor, index_type parentIndex, index_type childIndex,
                         const DataT& oldValue, const DataT& newValue, const bool first)
            : BaseCommand(accessor, tr("Edit %1").arg(accessor->typeName()))
            , _parentIndex(parentIndex)
            , _childIndex(childIndex)
            , _oldValue(oldValue)
            , _newValue(newValue)
            , _first(first)
        {
        }
        ~EditMergeCommand() = default;

        virtual int id() const final
        {
            return 0x1942;
        }

        virtual void undo() final
        {
            setValue(_oldValue);
        }

        virtual void redo() final
        {
            setValue(_newValue);
        }

        virtual bool mergeWith(const QUndoCommand* cmd) final
        {
            const EditMergeCommand* command = dynamic_cast<const EditMergeCommand*>(cmd);

            if (command
                && command->_first == false
                && command->_accessor == this->_accessor
                && command->_parentIndex == this->_parentIndex
                && command->_childIndex == this->_childIndex
                && command->_oldValue == this->_newValue
                && command->parentList() == this->parentList()) {

                _newValue = command->_newValue;

                return true;
            }
            else {
                return false;
            }
        }
    };

    struct EmptySignalFunction {
        inline void operator()(AccessorT*, index_type, index_type) const {}
    };

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    class EditFieldCommand : public BaseCommand {
    private:
        const index_type _parentIndex;
        const index_type _childIndex;
        const FieldT _oldValue;
        const FieldT _newValue;
        const UnaryFunction _getter;
        const ExtraSignalsFunction _signalEmitter;

        inline void setField(const FieldT& value)
        {
            DataT& childItem = this->getChildItem(_parentIndex, _childIndex);
            FieldT& field = _getter(childItem);

            field = value;
            this->emitDataChanged(_parentIndex, _childIndex, _signalEmitter);
        }

    public:
        EditFieldCommand(AccessorT* accessor, index_type parentIndex, index_type childIndex,
                         const FieldT& oldValue, const FieldT& newValue,
                         const QString& text,
                         UnaryFunction getter, ExtraSignalsFunction signalEmitter)
            : BaseCommand(accessor, text)
            , _parentIndex(parentIndex)
            , _childIndex(childIndex)
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
    };

    template <typename UnaryFunction, typename... FieldT>
    class EditMultipleFieldsCommand : public BaseCommand {
    private:
        const index_type _parentIndex;
        const index_type _childIndex;
        const std::tuple<FieldT...> _oldValues;
        const std::tuple<FieldT...> _newValues;
        const UnaryFunction _getter;

        inline void setValue(const std::tuple<FieldT...>& values)
        {
            DataT& childItem = this->getChildItem(_parentIndex, _childIndex);
            std::tuple<FieldT&...> fields = _getter(childItem);

            fields = values;
            this->emitDataChanged(_parentIndex, _childIndex);
        }

    public:
        EditMultipleFieldsCommand(AccessorT* accessor, index_type parentIndex, index_type childIndex,
                                  const std::tuple<FieldT&...> oldValues, const std::tuple<FieldT...>& newValues,
                                  const QString& text,
                                  UnaryFunction getter)
            : BaseCommand(accessor, text)
            , _parentIndex(parentIndex)
            , _childIndex(childIndex)
            , _oldValues(oldValues)
            , _newValues(newValues)
            , _getter(getter)
        {
        }
        ~EditMultipleFieldsCommand() = default;

        virtual void undo() final
        {
            setFields(_oldValues);
        }

        virtual void redo() final
        {
            setFields(_newValues);
        }
    };

    template <typename FieldT, typename UnaryFunction>
    class EditFieldIncompleteCommand : public BaseCommand {
    private:
        const index_type _parentIndex;
        const index_type _childIndex;
        const FieldT _oldValue;
        FieldT _newValue;
        const UnaryFunction _getter;

        inline void setField(const FieldT& value)
        {
            DataT& childItem = this->getChildItem(_parentIndex, _childIndex);
            FieldT& field = _getter(childItem);

            field = value;
            this->emitDataChanged(_parentIndex, _childIndex);
        }

    public:
        EditFieldIncompleteCommand(AccessorT* accessor, index_type parentIndex, index_type childIndex,
                                   const FieldT& oldValue,
                                   const QString& text,
                                   UnaryFunction getter)
            : BaseCommand(accessor, text)
            , _parentIndex(parentIndex)
            , _childIndex(childIndex)
            , _oldValue(oldValue)
            , _newValue(oldValue)
            , _getter(getter)
        {
        }
        ~EditFieldIncompleteCommand() = default;

        virtual void undo() final
        {
            setField(_oldValue);
        }

        virtual void redo() final
        {
            setField(_newValue);
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

    class EditMultipleItemsSameParentCommand : public BaseCommand {
    private:
        const index_type _parentIndex;
        const std::vector<index_type> _childIndexes;
        const std::vector<DataT> _oldValues;
        const std::vector<DataT> _newValues;

        inline void setData(const std::vector<DataT>& values)
        {
            Q_ASSERT(_childIndexes.size() == values.size());

            ChildListT& childList = this->getChildList(_parentIndex);

            auto it = values.begin();
            for (index_type childIndex : _childIndexes) {
                Q_ASSERT(childIndex >= 0 && childIndex < childList.size());
                DataT& childItem = childList->at(childIndex);

                childItem = *it++;
                this->emitDataChanged(_parentIndex, childIndex);
            }
            Q_ASSERT(it == values.end());
        }

    public:
        EditMultipleItemsSameParentCommand(AccessorT* accessor, const index_type parentIndex,
                                           std::vector<index_type>&& childIndexes,
                                           std::vector<DataT>&& oldValues,
                                           std::vector<DataT>&& newValues,
                                           const QString& text)
            : BaseCommand(accessor, text)
            , _parentIndex(parentIndex)
            , _childIndexes(std::move(childIndexes))
            , _oldValues(std::move(oldValues))
            , _newValues(std::move(newValues))
        {
        }
        ~EditMultipleItemsSameParentCommand() = default;

        virtual void undo() final
        {
            setData(_oldValues);
        }

        virtual void redo() final
        {
            setData(_newValues);
        }
    };

    class EditMultipleItemsCommand : public BaseCommand {
    private:
        const std::vector<std::pair<index_type, index_type>> _indexes;
        const std::vector<DataT> _oldValues;
        const std::vector<DataT> _newValues;

        inline void setData(const std::vector<DataT>& values)
        {
            Q_ASSERT(_indexes.size() == values.size());

            auto it = values.begin();
            for (const std::pair<index_type, index_type>& index : _indexes) {
                DataT& childItem = this->getChildItem(index.first, index.second);

                childItem = *it++;
                this->emitDataChanged(index.first, index.second);
            }
            Q_ASSERT(it == values.end());
        }

    public:
        EditMultipleItemsCommand(AccessorT* accessor,
                                 std::vector<std::pair<index_type, index_type>>&& indexes,
                                 std::vector<DataT>&& oldValues,
                                 std::vector<DataT>&& newValues,
                                 const QString& text)
            : BaseCommand(accessor, text)
            , _indexes(indexes)
            , _oldValues(std::move(oldValues))
            , _newValues(std::move(newValues))
        {
        }
        ~EditMultipleItemsCommand() = default;

        virtual void undo() final
        {
            setData(_oldValues);
        }

        virtual void redo() final
        {
            setData(_newValues);
        }
    };

    template <typename FieldT, typename UnaryFunction>
    class EditMultipleItemsFieldSameParentCommand : public BaseCommand {
    private:
        const index_type _parentIndex;
        const std::vector<index_type> _childIndexes;
        const std::vector<FieldT> _oldValues;
        const FieldT _newValue;
        const UnaryFunction _getter;

    public:
        EditMultipleItemsFieldSameParentCommand(AccessorT* accessor,
                                                index_type parentIndex,
                                                std::vector<index_type>&& childIndexes,
                                                std::vector<FieldT>&& oldValues,
                                                const FieldT& newValue,
                                                const QString& text,
                                                UnaryFunction getter)
            : BaseCommand(accessor, text)
            , _parentIndex(parentIndex)
            , _childIndexes(std::move(childIndexes))
            , _oldValues(std::move(oldValues))
            , _newValue(newValue)
            , _getter(getter)
        {
        }
        ~EditMultipleItemsFieldSameParentCommand() = default;

        virtual void undo() final
        {
            Q_ASSERT(_childIndexes.size() == _oldValues.size());

            ChildListT& childList = this->getChildList(_parentIndex);

            auto it = _oldValues.begin();
            for (index_type childIndex : _childIndexes) {
                Q_ASSERT(childIndex >= 0 && childIndex < childList.size());
                DataT& childItem = childList->at(childIndex);
                FieldT& field = _getter(childItem);

                field = *it++;
                this->emitDataChanged(_parentIndex, childIndex);
            }
            Q_ASSERT(it == _oldValues.end());
        }

        virtual void redo() final
        {
            ChildListT& childList = this->getChildList(_parentIndex);

            for (index_type childIndex : _childIndexes) {
                Q_ASSERT(childIndex >= 0 && childIndex < childList.size());
                DataT& childItem = childList->at(childIndex);
                FieldT& field = _getter(childItem);

                field = _newValue;

                this->emitDataChanged(_parentIndex, childIndex);
            }
        }
    };

    template <typename FieldT, typename UnaryFunction>
    class EditMultipleItemsFieldCommand : public BaseCommand {
    private:
        const std::vector<std::pair<index_type, index_type>> _indexes;
        const std::vector<FieldT> _oldValues;
        const FieldT _newValue;
        const UnaryFunction _getter;

    public:
        EditMultipleItemsFieldCommand(AccessorT* accessor,
                                      std::vector<std::pair<index_type, index_type>>&& indexes,
                                      std::vector<FieldT>&& oldValues,
                                      const FieldT& newValue,
                                      const QString& text,
                                      UnaryFunction getter)
            : BaseCommand(accessor, text)
            , _indexes(std::move(indexes))
            , _oldValues(std::move(oldValues))
            , _newValue(newValue)
            , _getter(getter)
        {
        }
        ~EditMultipleItemsFieldCommand() = default;

        virtual void undo() final
        {
            Q_ASSERT(_indexes.size() == _oldValues.size());

            auto it = _oldValues.begin();
            for (std::pair<index_type, index_type>& index : _indexes) {
                DataT& childItem = this->getChildItem(index.first, index.second);

                childItem = *it++;
                this->emitDataChanged(index.first, index.second);
            }
            Q_ASSERT(it == _oldValues.end());
        }

        virtual void redo() final
        {
            for (std::pair<index_type, index_type>& index : _indexes) {
                DataT& childItem = this->getChildItem(index.first, index.second);

                childItem = _newValue;
                this->emitDataChanged(index.first, index.second);
            }
        }
    };

    class AddRemoveCommand : public BaseCommand {
    private:
        const index_type _parentIndex;
        const index_type _childIndex;
        const DataT _value;

    public:
        // used by the SelctionModifier and subclasses
        index_type parentIndex() const { return _parentIndex; }
        index_type childIndex() const { return _childIndex; }

    protected:
        AddRemoveCommand(AccessorT* accessor, index_type parentIndex, index_type childIndex,
                         const DataT& value,
                         const QString& text)
            : BaseCommand(accessor, text)
            , _parentIndex(parentIndex)
            , _childIndex(childIndex)
            , _value(value)
        {
        }
        ~AddRemoveCommand() = default;

        void addItem_NoChangeSelection()
        {
            ChildListT& childList = this->getChildList(_parentIndex);
            Q_ASSERT(_childIndex >= 0 && _childIndex <= childList.size());

            this->emitListAboutToChange(_parentIndex);

            childList.insert(childList.begin() + _childIndex, _value);

            this->emitItemAdded(_parentIndex, _childIndex);
            this->emitListChanged(_parentIndex);
        }

        void removeItem_NoChangeSelection()
        {
            ChildListT& childList = this->getChildList(_parentIndex);
            Q_ASSERT(_childIndex >= 0 && _childIndex < childList.size());

            this->emitListAboutToChange(_parentIndex);
            this->emitItemAboutToBeRemoved(_parentIndex, _childIndex);

            childList.erase(childList.begin() + _childIndex);

            this->emitListChanged(_parentIndex);
        }

    protected:
        void addItem()
        {
            auto selection = SelectionModifier::getSelection(this->_accessor);
            addItem_NoChangeSelection();
            SelectionModifier::itemAdded(selection, _parentIndex, _childIndex);
            SelectionModifier::setSelection(this->_accessor, std::move(selection));
        }

        void removeItem()
        {
            auto selection = SelectionModifier::getSelection(this->_accessor);
            removeItem_NoChangeSelection();
            SelectionModifier::itemRemoved(selection, _parentIndex, _childIndex);
            SelectionModifier::setSelection(this->_accessor, std::move(selection));
        }
    };

    class AddCommand : public AddRemoveCommand {
    public:
        AddCommand(AccessorT* accessor, index_type parentIndex, index_type childIndex)
            : AddRemoveCommand(accessor, parentIndex, childIndex, DataT(),
                               tr("Add %1").arg(accessor->typeName()))
        {
        }

        AddCommand(AccessorT* accessor, index_type parentIndex, index_type childIndex, const DataT& value, const QString text)
            : AddRemoveCommand(accessor, parentIndex, childIndex, value, text)
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
        RemoveCommand(AccessorT* accessor, index_type parentIndex, index_type childIndex, const DataT& value, const QString text)
            : AddRemoveCommand(accessor, parentIndex, childIndex, value, text)
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
        const index_type _fromParentIndex;
        const index_type _fromChildIndex;
        const index_type _toParentIndex;
        const index_type _toChildIndex;

    private:
        inline void _moveSameList(const index_type parentIndex, const index_type from, const index_type to)
        {
            ChildListT& childList = this->getChildList(parentIndex);
            Q_ASSERT(from != to);
            Q_ASSERT(from >= 0 && from < childList.size());
            Q_ASSERT(to >= 0 && to < childList.size());

            this->emitListAboutToChange(parentIndex);

            moveListItem(from, to, childList);

            this->emitItemMoved(parentIndex, from, parentIndex, to);
            this->emitListChanged(parentIndex);
        }

        void _moveDifferentList(const index_type fromParentIndex, const index_type fromChildIndex,
                                const index_type toParentIndex, const index_type toChildIndex)
        {
            ChildListT& fromChildList = this->getChildList(fromParentIndex);
            Q_ASSERT(fromChildIndex >= 0 && fromChildIndex < fromChildList.size());

            ChildListT& toChildList = this->getChildList(toParentIndex);
            Q_ASSERT(toChildIndex >= 0 && toChildIndex <= toChildList.size());

            this->emitListAboutToChange(toParentIndex);
            this->emitListAboutToChange(fromParentIndex);

            toChildList.insert(toChildList.begin() + toChildIndex, std::move(fromChildList.at(fromChildIndex)));

            this->emitItemAdded(toParentIndex, toChildIndex);
            this->emitItemAboutToBeRemoved(fromParentIndex, fromChildIndex);

            fromChildList.erase(fromChildList.begin() + fromChildIndex);

            this->emitListChanged(fromParentIndex);
            this->emitListChanged(toParentIndex);
        }

        void moveItem(const index_type fromParentIndex, const index_type fromChildIndex,
                      const index_type toParentIndex, const index_type toChildIndex)
        {
            auto selection = SelectionModifier::getSelection(this->_accessor);

            if (fromParentIndex == toParentIndex) {
                _moveSameList(fromParentIndex, toParentIndex, toChildIndex);
            }
            else {
                _moveDifferentList(fromParentIndex, fromChildIndex, toParentIndex, toChildIndex);
            }

            SelectionModifier::itemMoved(selection, fromParentIndex, fromChildIndex, toParentIndex, toChildIndex);

            SelectionModifier::setSelection(this->_accessor, std::move(selection));
        }

    public:
        MoveCommand(AccessorT* accessor,
                    index_type fromParentIndex, index_type fromChildIndex,
                    index_type toParentIndex, index_type toChildIndex,
                    const QString& text)
            : BaseCommand(accessor, text)
            , _fromParentIndex(fromParentIndex)
            , _fromChildIndex(fromChildIndex)
            , _toParentIndex(toParentIndex)
            , _toChildIndex(toChildIndex)
        {
        }
        ~MoveCommand() = default;

        void undo()
        {
            moveItem(_toParentIndex, _toChildIndex, _fromParentIndex, _fromChildIndex);
        }

        void redo()
        {
            moveItem(_fromParentIndex, _fromChildIndex, _toParentIndex, _toChildIndex);
        }
    };

    class AddRemoveMultipleCommand : public BaseCommand {
    private:
        const std::vector<std::pair<index_type, index_type>> _indexes;
        const std::vector<DataT> _values;

    public:
        const std::vector<std::pair<index_type, index_type>>& indexes() const { return _indexes; }
        const std::vector<DataT>& values() const { return _values; }

    protected:
        AddRemoveMultipleCommand(AccessorT* accessor,
                                 const std::vector<std::pair<index_type, index_type>>& indexes,
                                 std::vector<DataT>&& values,
                                 const QString& text)
            : BaseCommand(accessor, text)
            , _indexes(indexes)
            , _values(std::move(values))
        {
        }
        AddRemoveMultipleCommand(AccessorT* accessor,
                                 std::vector<std::pair<index_type, index_type>>&& indexes,
                                 std::vector<DataT>&& values,
                                 const QString& text)
            : BaseCommand(accessor, text)
            , _indexes(std::move(indexes))
            , _values(std::move(values))
        {
        }
        ~AddRemoveMultipleCommand() = default;

        void addItems_NoUpdateSelection()
        {
            Q_ASSERT(_indexes.size() == _values.size());
            Q_ASSERT(_indexes.size() > 0);

            auto iIt = _indexes.cbegin();
            auto vIt = _values.cbegin();

            while (iIt != _indexes.cend()) {
                index_type parentIndex = iIt->first;
                ChildListT& childList = this->getChildList(parentIndex);

                this->emitListAboutToChange(parentIndex);

                while (iIt != _indexes.cend() && iIt->first == parentIndex) {
                    const index_type childIndex = iIt->second;
                    const DataT& value = *vIt;

                    Q_ASSERT(childIndex <= childList.size());

                    childList.insert(childList.begin() + childIndex, value);
                    this->emitItemAdded(parentIndex, childIndex);

                    iIt++;
                    vIt++;
                }

                this->emitChildListChanged(parentIndex);
            }
            Q_ASSERT(vIt == _values.cend());
        }

        void removeItems_NoUpdateSelection()
        {
            Q_ASSERT(_indexes.size() > 0);

            Q_ASSERT(_indexes.size() > 0);

            auto iIt = _indexes.crbegin();

            while (iIt != _indexes.crend()) {
                index_type parentIndex = iIt->first;
                ChildListT& childList = this->getChildList(parentIndex);

                this->emitListAboutToChange(parentIndex);

                while (iIt != _indexes.crend() && iIt->first == parentIndex) {
                    const index_type childIndex = iIt->second;
                    Q_ASSERT(childIndex < childList.size());

                    this->emitItemAboutToBeRemoved(parentIndex, childIndex);

                    childList.erase(childList.begin() + childIndex);

                    iIt++;
                }

                this->emitChildListChanged(parentIndex);
            }
        }

        void addItems()
        {
            auto selection = SelectionModifier::getSelection(this->_accessor);
            addItems_NoUpdateSelection();
            for (const auto index : _indexes) {
                SelectionModifier::itemAdded(selection, index.first, index.second);
            }
            SelectionModifier::setSelection(this->_accessor, std::move(selection));
        }

        void removeItems()
        {
            auto selection = SelectionModifier::getSelection(this->_accessor);
            removeItems_NoUpdateSelection();
            for (auto it = _indexes.crbegin(); it != _indexes.crend(); it++) {
                SelectionModifier::itemRemoved(selection, it->first, it->second);
            }
            SelectionModifier::setSelection(this->_accessor, std::move(selection));
        }
    };

    class AddMultipleCommand : public AddRemoveMultipleCommand {
    public:
        AddMultipleCommand(AccessorT* accessor,
                           std::vector<std::pair<index_type, index_type>>&& indexes,
                           std::vector<DataT>&& values,
                           const QString& text)
            : AddRemoveMultipleCommand(accessor, std::move(indexes), std::move(values), text)
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
        RemoveMultipleCommand(AccessorT* accessor,
                              const std::vector<std::pair<index_type, index_type>>& indexes,
                              std::vector<DataT>&& values,
                              const QString& text)
            : AddRemoveMultipleCommand(accessor, indexes, std::move(values), text)
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
        struct MoveChildItems {
            const index_type parentIndex;
            const std::vector<index_type> fromIndexes;
            const std::vector<index_type> toIndexes;
        };
        const std::vector<MoveChildItems> _childListsMoved;
        const bool _moveUp;

    private:
        std::vector<index_type> buildToIndexes(const index_type parentIndex, const vectorset<index_type>& fromIndexes, const int offset)
        {
            ChildListT& list = this->getChildList(parentIndex);
            Q_ASSERT(fromIndexes.size() < list.size());

            std::vector<index_type> toIndexes;
            toIndexes.reserve(fromIndexes.size());

            Q_ASSERT(offset != 0);
            if (offset < 0) {
                // move up
                const index_type absOffset = -offset;
                index_type limit = 0;
                for (const index_type& from : fromIndexes) {
                    toIndexes.push_back(from > absOffset ? std::max(from - absOffset, limit) : limit);
                    limit++;
                }
            }
            else {
                // move down
                const index_type absOffset = offset;
                index_type limit = list.size() - fromIndexes.size();
                for (const index_type& from : fromIndexes) {
                    toIndexes.push_back(std::min(from + absOffset, limit));
                    limit++;
                }
            }

            return toIndexes;
        }

        std::vector<MoveChildItems> buildChildListsMoved(const std::vector<std::pair<index_type, vectorset<index_type>>>& indexes, int offset)
        {
            std::vector<MoveChildItems> out;
            out.reserve(indexes.size());

            for (const auto& input : indexes) {
                const auto& parentIndex = input.first;
                const auto& childIndexes = input.second;

                out.emplace_back(MoveChildItems{
                    .parentIndex = parentIndex,
                    .fromIndexes = childIndexes,
                    .toIndexes = buildToIndexes(parentIndex, childIndexes, offset) });
            }

            return out;
        }

        void moveItems(const index_type parentIndex, const std::vector<index_type>& from, const std::vector<index_type>& to, bool moveUp)
        {
            auto selection = SelectionModifier::getSelection(this->_accessor);

            ChildListT& childList = this->getChildList(parentIndex);

            Q_ASSERT(from.empty() == false);
            Q_ASSERT(from.size() == to.size());
            Q_ASSERT(from.front() >= 0 && from.back() < childList.size());
            Q_ASSERT(to.front() >= 0 && to.back() < childList.size());

            auto doMove = [&](index_type from, index_type to) {
                if (from != to) {
                    moveListItem(from, to, childList);
                    this->emitItemMoved(parentIndex, from, parentIndex, to);
                    SelectionModifier::itemMoved(selection, parentIndex, from, parentIndex, to);
                }
            };

            this->emitListAboutToChange(parentIndex);

            if (moveUp) {
                // Move items up
                auto fromIt = from.cbegin();
                auto toIt = to.cbegin();
                while (fromIt != from.cend()) {
                    doMove(*fromIt++, *toIt++);
                }
                assert(toIt == to.cend());
            }
            else {
                // Move items down
                auto fromIt = from.crbegin();
                auto toIt = to.crbegin();
                while (fromIt != from.crend()) {
                    doMove(*fromIt++, *toIt++);
                }
                assert(toIt == to.crend());
            }

            this->emitListChanged(parentIndex);

            SelectionModifier::setSelection(this->_accessor, std::move(selection));
        }

    public:
        MoveMultipleCommand(AccessorT* accessor,
                            const std::vector<std::pair<index_type, vectorset<index_type>>>&& indexes, int offset,
                            const QString& text)
            : BaseCommand(accessor, text)
            , _childListsMoved(buildChildListsMoved(indexes, offset))
            , _moveUp(offset < 0)
        {
        }
        MoveMultipleCommand(AccessorT* accessor,
                            const index_type parentIndex, std::vector<index_type>& indexes, int offset,
                            const QString& text)
            : BaseCommand(accessor, text)
            , _childListsMoved({ .parentIndex = parentIndex,
                                 .fromIndexes = indexes,
                                 .toIndexes = buildToIndexes(parentIndex, indexes, offset) })
            , _moveUp(offset < 0)
        {
        }
        ~MoveMultipleCommand() = default;

        void undo()
        {
            for (auto& clm : _childListsMoved) {
                moveItems(clm.parentIndex, clm.toIndexes, clm.fromIndexes, !_moveUp);
            }
        }

        void redo()
        {
            for (auto& clm : _childListsMoved) {
                moveItems(clm.parentIndex, clm.fromIndexes, clm.toIndexes, _moveUp);
            }
        }
    };

    class MoveMultipleToChildListCommand : public AddRemoveMultipleCommand {
    private:
        const index_type _targetParentIndex;

    private:
        index_type _addToTargetList()
        {
            const auto& values = this->values();

            Q_ASSERT(!values.empty());

            this->emitListAboutToChange(_targetParentIndex);

            ChildListT& childList = this->getChildList(_targetParentIndex);

            const index_type firstChildIndex = childList.size();

            index_type childIndex = childList.size();
            for (const DataT& data : values) {
                childList.emplace_back(data);

                this->emitItemAdded(_targetParentIndex, childIndex);
                childIndex++;
            }

            this->emitChildListChanged(_targetParentIndex);

            return firstChildIndex;
        }

        index_type _removeFromTargetList()
        {
            const auto& indexes = this->indexes();

            Q_ASSERT(!indexes.empty());

            this->emitListAboutToChange(_targetParentIndex);

            ChildListT& childList = this->getChildList(_targetParentIndex);

            for (index_type i = 0; i < indexes.size(); i++) {
                Q_ASSERT(childList.empty() == false);
                const index_type childIndex = childList.size() - 1;

                this->emitItemAboutToBeRemoved(_targetParentIndex, childIndex);

                childList.erase(childList.begin() + childIndex);
            }

            this->emitChildListChanged(_targetParentIndex);

            return childList.size();
        }

    public:
        MoveMultipleToChildListCommand(AccessorT* accessor,
                                       std::vector<std::pair<index_type, index_type>>&& indexes, std::vector<DataT>&& values,
                                       index_type targetParentIndex,
                                       const QString& text)
            : AddRemoveMultipleCommand(accessor, std::move(indexes), std::move(values), text)
            , _targetParentIndex(targetParentIndex)
        {
        }
        ~MoveMultipleToChildListCommand() = default;

        virtual void undo() final
        {
            auto selection = SelectionModifier::getSelection(this->_accessor);

            index_type targetChildIndex = _removeFromTargetList();
            this->addItems_NoUpdateSelection();

            for (const auto& index : this->indexes()) {
                SelectionModifier::itemMoved(selection, _targetParentIndex, targetChildIndex, index.first, index.second);
            }

            SelectionModifier::setSelection(this->_accessor, std::move(selection));
        }

        virtual void redo() final
        {
            auto selection = SelectionModifier::getSelection(this->_accessor);

            this->removeItems_NoUpdateSelection();
            const index_type firstChildAdded = _addToTargetList();

            const auto& indexes = this->indexes();
            for (auto it = indexes.rbegin(); it != indexes.rend(); it++) {
                SelectionModifier::itemMoved(selection, it->first, it->second, _targetParentIndex, firstChildAdded);
            }

            SelectionModifier::setSelection(this->_accessor, std::move(selection));
        }
    };

protected:
    AccessorT* const _accessor;

public:
    NestedListUndoHelper(AccessorT* accessor)
        : _accessor(accessor)
    {
    }

private:
    inline const ParentListT* getParentList() const
    {
        return _accessor->getParentList();
    }

    inline const ChildListT* getChildList(const index_type parentIndex) const
    {
        const ParentListT* parentList = _accessor->getParentList();

        if (parentList == nullptr) {
            return nullptr;
        }
        if (parentIndex < 0 || parentIndex >= parentList->size()) {
            return nullptr;
        }

        const ParentT& parentItem = parentList->at(parentIndex);
        return &_accessor->childList(parentItem);
    }

    inline const DataT* getChildItem(const index_type parentIndex, const index_type childIndex) const
    {
        const ParentListT* parentList = _accessor->parentList();

        if (parentList == nullptr) {
            return nullptr;
        }
        if (parentIndex < 0 || parentIndex >= parentList->size()) {
            return nullptr;
        }
        const ParentT& parentItem = parentList->at(parentIndex);
        const ChildListT& childList = _accessor->childList(parentItem);

        if (childIndex < 0 || childIndex > childList.size()) {
            return nullptr;
        }

        return &childList.at(childIndex);
    }

    inline DataT* getChildItem_NO_CONST(const index_type parentIndex, const index_type childIndex)
    {
        ParentListT* parentList = _accessor->getParentList();

        if (parentList == nullptr) {
            return nullptr;
        }
        if (parentIndex < 0 || parentIndex >= parentList->size()) {
            return nullptr;
        }

        ParentT& parentItem = parentList->at(parentIndex);
        ChildListT& childList = _accessor->getChildList(parentItem);

        if (childIndex < 0 || childIndex > childList.size()) {
            return nullptr;
        }
        return &childList.at(childIndex);
    }

public:
    // will return nullptr if data cannot be accessed or is equal to newValue
    QUndoCommand* editItemCommand(const index_type parentIndex, const index_type childIndex, const DataT& newValue)
    {
        const DataT* oldValue = getChildItem(parentIndex, childIndex);
        if (oldValue == nullptr) {
            return nullptr;
        }

        if (*oldValue == newValue) {
            return nullptr;
        }

        return new EditCommand(_accessor, parentIndex, childIndex, *oldValue, newValue);
    }

    bool editItem(const index_type parentIndex, const index_type childIndex, const DataT& newValue)
    {
        QUndoCommand* e = editItemCommand(parentIndex, childIndex, newValue);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    // will return nullptr if data cannot be accessed or is unchanged
    template <typename EditFunction>
    QUndoCommand* editItemCommand(const index_type parentIndex, const index_type childIndex, const QString& text,
                                  EditFunction editFunction)
    {
        const DataT* oldValue = getChildItem(parentIndex, childIndex);
        if (oldValue == nullptr) {
            return nullptr;
        }

        DataT newValue = *oldValue;
        editFunction(newValue);

        if (*oldValue == newValue) {
            return nullptr;
        }

        return new EditCommand(_accessor, parentIndex, childIndex, *oldValue, newValue, text);
    }

    template <typename EditFunction>
    bool editItem(const index_type parentIndex, const index_type childIndex, const QString& text,
                  EditFunction editFunction)
    {
        QUndoCommand* e = editItemCommand(parentIndex, childIndex, text, editFunction);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    // will return nullptr if data cannot be accessed or is equal to newValue
    QUndoCommand* editMergeCommand(const index_type parentIndex, const index_type childIndex, const DataT& newValue, bool first = false)
    {
        const DataT* oldValue = getChildItem(parentIndex, childIndex);
        if (oldValue == nullptr) {
            return nullptr;
        }

        if (*oldValue == newValue) {
            return nullptr;
        }

        return new EditMergeCommand(_accessor, parentIndex, childIndex, *oldValue, newValue, first);
    }

    bool editMerge(const index_type parentIndex, const index_type childIndex, const DataT& newValue, bool first = false)
    {
        QUndoCommand* c = editMergeCommand(parentIndex, childIndex, newValue, first);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if data cannot be accessed or is equal to newValue
    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    QUndoCommand* editFieldCommand(const index_type parentIndex, const index_type childIndex, const FieldT& newValue, const QString& text,
                                   UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        DataT* item = getChildItem_NO_CONST(parentIndex, childIndex);
        if (item == nullptr) {
            return nullptr;
        }

        const FieldT& oldValue = getter(*item);
        if (oldValue == newValue) {
            return nullptr;
        }

        return new EditFieldCommand<FieldT, UnaryFunction, ExtraSignalsFunction>(
            _accessor, parentIndex, childIndex, oldValue, newValue, text, getter, extraSignals);
    }

    // will return nullptr if data cannot be accessed or is equal to newValue
    template <typename FieldT, typename UnaryFunction>
    QUndoCommand* editFieldCommand(const index_type parentIndex, const index_type childIndex, const FieldT& newValue, const QString& text,
                                   UnaryFunction getter)
    {
        return editFieldCommand(parentIndex, childIndex, newValue, text, getter, EmptySignalFunction());
    }

    template <typename FieldT, typename UnaryFunction>
    bool editField(const index_type parentIndex, const index_type childIndex, const FieldT& newValue, const QString& text,
                   UnaryFunction getter)
    {
        QUndoCommand* e = editFieldCommand(parentIndex, childIndex, newValue, text, getter);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    bool editField(const index_type parentIndex, const index_type childIndex, const FieldT& newValue, const QString& text,
                   UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        QUndoCommand* e = editFieldCommand(parentIndex, childIndex, newValue, text, getter, extraSignals);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    // will return nullptr if data cannot be accessed or is equal to newValues
    template <typename... FieldT, typename UnaryFunction>
    QUndoCommand* editMulitpleFieldsCommand(const index_type parentIndex, const index_type childIndex, const std::tuple<FieldT...>& newValues,
                                            const QString& text,
                                            UnaryFunction getter)
    {
        const DataT* item = getChildItem(parentIndex, childIndex);
        if (item == nullptr) {
            return nullptr;
        }

        const std::tuple<FieldT&...> oldValues = getter(*item);

        if (oldValues == newValues) {
            return nullptr;
        }

        return new EditMultipleFieldsCommand<UnaryFunction, FieldT...>(
            _accessor, parentIndex, childIndex, oldValues, newValues, text, getter);
    }

    template <typename... FieldT, typename UnaryFunction>
    bool editMultipleFields(const index_type parentIndex, const index_type childIndex, const std::tuple<FieldT...>& newValues,
                            const QString& text,
                            UnaryFunction getter)
    {
        QUndoCommand* c = editMulitpleFieldsCommand(parentIndex, childIndex, newValues, text, getter);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // The caller is responsible for setting the new value of the command and
    // releasing it into the undo stack.
    // Will return nullptr if field cannot be accessed
    template <typename FieldT, typename UnaryFunction>
    std::unique_ptr<EditFieldIncompleteCommand<FieldT, UnaryFunction>>
    editFieldIncompleteCommand(const index_type parentIndex, const index_type childIndex, const QString& text,
                               UnaryFunction getter)
    {
        const DataT* item = getChildItem(parentIndex, childIndex);
        if (item == nullptr) {
            return nullptr;
        }
        const FieldT& oldValue = getter(*item);

        return std::make_unique<EditFieldIncompleteCommand<FieldT, UnaryFunction>>(
            _accessor, parentIndex, childIndex, oldValue, text, getter);
    }

    // will return nullptr if data cannot be accessed or unchanged
    template <typename EditFunction>
    QUndoCommand* editMultipleItemsCommand(const index_type parentIndex, const vectorset<index_type>& childIndexes,
                                           const QString& text, EditFunction editFunction)
    {
        const ChildListT* childList = getChildList(parentIndex);
        if (childList == nullptr) {
            return nullptr;
        }

        std::vector<index_type> indexesEdited;
        std::vector<DataT> oldValues;
        std::vector<DataT> newValues;

        indexesEdited.reserve(childIndexes.size());
        oldValues.reserve(childIndexes.size());
        newValues.reserve(childIndexes.size());

        for (const index_type& index : childIndexes) {
            if (index >= 0 && index < childList->size()) {
                const DataT& oldValue = childList->at(index);
                DataT newValue = childList->at(index);

                editFunction(newValue, parentIndex, index);

                if (newValue != oldValue) {
                    indexesEdited.push_back(index);
                    oldValues.push_back(oldValue);
                    newValues.push_back(std::move(newValue));
                }
            }
        }

        if (indexesEdited.empty()) {
            return nullptr;
        }

        return new EditMultipleItemsSameParentCommand(
            _accessor, parentIndex, std::move(indexesEdited), std::move(oldValues), std::move(newValues), text);
    }

    template <typename EditFunction>
    bool editMultipleItems(const index_type parentIndex, const vectorset<index_type>& childIndexes,
                           const QString& text, EditFunction editFunction)
    {
        QUndoCommand* c = editMultipleItemsCommand(parentIndex, childIndexes, text, editFunction);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if data cannot be accessed or unchanged
    template <typename EditFunction>
    QUndoCommand* editMultipleItemsCommand(const vectorset<std::pair<index_type, index_type>>& indexes,
                                           const QString& text, EditFunction editFunction)
    {
        std::vector<std::pair<index_type, index_type>> indexesEdited;
        std::vector<DataT> oldValues;
        std::vector<DataT> newValues;

        indexesEdited.reserve(indexes.size());
        oldValues.reserve(indexes.size());
        newValues.reserve(indexes.size());

        for (const auto index : indexes) {
            if (const DataT* oldValue = getChildItem(index.first, index.second)) {
                DataT newValue = *oldValue;

                editFunction(newValue, index.first, index.second);

                if (newValue != *oldValue) {
                    indexesEdited.emplace_back(index);
                    oldValues.emplace_back(*oldValue);
                    newValues.emplace_back(std::move(newValue));
                }
            }
        }

        if (indexesEdited.empty()) {
            return nullptr;
        }

        return new EditMultipleItemsCommand(_accessor, std::move(indexesEdited), std::move(oldValues), std::move(newValues), text);
    }

    template <typename EditFunction>
    bool editMultipleItems(const vectorset<std::pair<index_type, index_type>>& indexes,
                           const QString& text, EditFunction editFunction)
    {
        QUndoCommand* c = editMultipleItemsCommand(indexes, text, editFunction);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    template <typename FieldT, typename UnaryFunction>
    QUndoCommand* setFieldInMultipleItemsCommand(const index_type parentIndex, const vectorset<index_type>& childIndexes, const FieldT& newValue,
                                                 const QString& text, UnaryFunction getter)
    {
        const ChildListT* childList = getChildList(parentIndex);
        if (childList == nullptr) {
            return nullptr;
        }

        std::vector<index_type> indexesEdited;
        std::vector<FieldT> oldValues;

        indexesEdited.reserve(childIndexes.size());
        oldValues.reserve(childIndexes.size());

        for (const index_type index : childIndexes) {
            if (index >= 0 && index < childList->size()) {
                const DataT& data = childList->at(index);
                const FieldT& oldValue = getter(data);

                if (oldValue != newValue) {
                    indexesEdited.push_back(index);
                    oldValues.push_back(oldValue);
                }
            }
        }

        if (indexesEdited.empty()) {
            return nullptr;
        }

        return new EditMultipleItemsSameParentCommand(_accessor, parentIndex, std::move(indexesEdited), std::move(oldValues), newValue, text);
    }

    template <typename FieldT, typename UnaryFunction>
    bool setFieldInMultipleItems(const index_type parentIndex, const vectorset<index_type>& childIndexes, const FieldT& newValue,
                                 const QString& text, UnaryFunction getter)
    {
        QUndoCommand* c = setFieldInMultipleItemsCommand(parentIndex, childIndexes, newValue, text, getter);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    template <typename FieldT, typename UnaryFunction>
    QUndoCommand* setFieldInMultipleItemsCommand(const vectorset<std::pair<index_type, index_type>>& indexes, const FieldT& newValue,
                                                 const QString& text, UnaryFunction getter)
    {
        std::vector<index_type> indexesEdited;
        std::vector<FieldT> oldValues;

        indexesEdited.reserve(indexes.size());
        oldValues.reserve(indexes.size());

        for (const auto index : indexes) {
            if (const DataT* data = getChildItem(index.first, index.second)) {
                const FieldT& oldValue = getter(data);

                if (oldValue != newValue) {
                    indexesEdited.push_back(index);
                    oldValues.push_back(oldValue);
                }
            }
        }

        if (indexesEdited.empty()) {
            return nullptr;
        }

        return new EditMultipleFieldsCommand(_accessor, std::move(indexesEdited), std::move(oldValues), newValue, text);
    }

    template <typename FieldT, typename UnaryFunction>
    bool setFieldInMultipleItems(const vectorset<std::pair<index_type, index_type>>& indexes, const FieldT& newValue,
                                 const QString& text, UnaryFunction getter)
    {
        QUndoCommand* c = setFieldInMultipleItemsCommand(indexes, newValue, text, getter);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if data cannot be accessed or unchanged
    template <typename EditFunction>
    QUndoCommand* editAllItemsInChildListCommand(const index_type parentIndex, const QString& text, EditFunction editFunction)
    {
        const ChildListT* childList = getChildList(parentIndex);
        if (childList == nullptr) {
            return nullptr;
        }

        std::vector<index_type> childIndexesEdited;
        std::vector<DataT> oldValues;
        std::vector<DataT> newValues;

        childIndexesEdited.reserve(childList->size());
        oldValues.reserve(childList->size());
        newValues.reserve(childList->size());

        for (index_type childIndex = 0; childIndex < childList->size(); childIndex++) {
            const DataT& oldValue = childList->at(childIndex);
            DataT newValue = oldValue;

            editFunction(newValue, childIndex);

            if (newValue != oldValue) {
                childIndexesEdited.push_back(childIndex);
                oldValues.push_back(oldValue);
                newValues.push_back(std::move(newValue));
            }
        }

        if (childIndexesEdited.empty()) {
            return nullptr;
        }

        return new EditMultipleItemsSameParentCommand(
            _accessor, parentIndex,
            std::move(childIndexesEdited), std::move(oldValues), std::move(newValues),
            text);
    }

    template <typename EditFunction>
    bool editAllItemsInChildList(const index_type parentIndex, const QString& text, EditFunction editFunction)
    {
        QUndoCommand* c = editAllItemsCommand(parentIndex, text, editFunction);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if data cannot be accessed or unchanged
    template <typename EditFunction>
    QUndoCommand* editAllItemsRecursiveCommand(const QString& text, EditFunction editFunction)
    {
        const ParentListT* parentList = getParentList();
        if (parentList == nullptr) {
            return nullptr;
        }

        std::vector<std::pair<index_type, index_type>> indexesEdited;
        std::vector<DataT> oldValues;
        std::vector<DataT> newValues;

        for (index_type parentIndex = 0; parentList < parentList->size(); parentIndex++) {
            const ParentT& parentItem = parentList->at(parentIndex);
            const ChildListT& childList = _accessor->getChildList(*parentItem);

            for (index_type childIndex = 0; childIndex < childList->size(); childIndex++) {
                const DataT& oldValue = childList->at(childIndex);
                DataT newValue = oldValue;

                editFunction(newValue, parentIndex, childIndex);

                if (newValue != oldValue) {
                    indexesEdited.emplace_back(parentIndex, childIndex);
                    oldValues.push_back(oldValue);
                    newValues.push_back(std::move(newValue));
                }
            }
        }

        if (indexesEdited.empty()) {
            return nullptr;
        }

        return new EditMultipleItemsCommand(_accessor, std::move(indexesEdited), std::move(oldValues), std::move(newValues), text);
    }

    template <typename EditFunction>
    bool editAllItemsRecursive(const QString& text, EditFunction editFunction)
    {
        QUndoCommand* c = editAllItemsRecursiveCommand(text, editFunction);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* addCommand(const index_type parentIndex, index_type childIndex, DataT item, const QString& text)
    {
        const ChildListT* childList = getChildList(parentIndex);
        if (childList == nullptr) {
            return nullptr;
        }
        if (childIndex < 0 || childIndex > childList->size()) {
            return nullptr;
        }
        if (childIndex >= _accessor->maxChildListSize()) {
            return nullptr;
        }

        return new AddCommand(_accessor, parentIndex, childIndex, item, text);
    }

    QUndoCommand* addCommand(const index_type parentIndex, const index_type childIndex, DataT item = DataT())
    {
        return addCommand(parentIndex, childIndex, std::move(item),
                          tr("Add %1").arg(_accessor->typeName()));
    }

    bool addItem(const index_type parentIndex, const index_type childIndex, DataT item, const QString& text)
    {
        QUndoCommand* c = addCommand(parentIndex, childIndex, std::move(item), text);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
            SelectionModifier::postAddCommand(_accessor, parentIndex, static_cast<AddRemoveCommand*>(c)->childIndex());
        }
        return c != nullptr;
    }

    bool addItem(const index_type parentIndex, const index_type childIndex, DataT item = DataT())
    {
        return addItem(parentIndex, childIndex, std::move(item), tr("Add %1").arg(_accessor->typeName()));
    }

    bool addItem()
    {
        return addItem(INT_MAX);
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* cloneCommand(const index_type parentIndex, const index_type childIndex)
    {
        const ChildListT* childList = getChildList(parentIndex);
        if (childList == nullptr) {
            return nullptr;
        }
        if (childIndex < 0 || childIndex >= childList->size()) {
            return nullptr;
        }
        const index_type newChildIndex = childIndex + 1;

        if (newChildIndex >= _accessor->maxChildListSize()) {
            return nullptr;
        }
        return addCommand(parentIndex, newChildIndex, childList->at(childIndex), tr("Clone %1").arg(_accessor->typeName()));
    }

    bool cloneItem(const index_type parentIndex, const index_type childIndex)
    {
        QUndoCommand* c = cloneCommand(parentIndex, childIndex);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
            SelectionModifier::postAddCommand(_accessor, parentIndex, static_cast<AddRemoveCommand*>(c)->childIndex());
        }
        return c != nullptr;
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* removeCommand(const index_type parentIndex, const index_type childIndex)
    {
        const DataT* data = getChildItem(parentIndex, childIndex);
        if (data == nullptr) {
            return nullptr;
        }
        return new RemoveCommand(_accessor, parentIndex, childIndex, *data,
                                 tr("Remove %1").arg(_accessor->typeName()));
    }

    bool removeItem(const index_type parentIndex, const index_type childIndex)
    {
        QUndoCommand* c = removeCommand(parentIndex, childIndex);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    QUndoCommand* cloneMultipleCommand(const index_type parentIndex, const vectorset<index_type>& childIndexes)
    {
        if (childIndexes.empty()) {
            return nullptr;
        }

        const ChildListT* childList = getChildList(parentIndex);
        if (childList == nullptr) {
            return nullptr;
        }
        if (childIndexes.front() < 0 || childIndexes.back() >= childList->size()) {
            return nullptr;
        }
        if (childList->size() + childIndexes.size() > _accessor->maxSize()) {
            return nullptr;
        }

        std::vector<std::pair<index_type, index_type>> newIndexes;
        newIndexes.reseve(childIndexes.size());
        for (auto it = childIndexes.cbegin(); it != childIndexes.cend(); it++) {
            newIndexes.emplace_back(parentIndex, *it + std::distance(childIndexes.cbegin(), it) + 1);
        }

        std::vector<DataT> newValues;
        newValues.reserve(childIndexes.size());
        for (const index_type& i : childIndexes) {
            newValues.emplace_back(childList->at(i));
        }

        const QString& typeName = newIndexes.size() == 1 ? _accessor->typeName() : _accessor->typeNamePlural();

        return new AddMultipleCommand(_accessor, std::move(newIndexes), std::move(newValues), tr("Clone %1").arg(typeName));
    }

    bool cloneMultipleItems(const index_type parentIndex, const vectorset<index_type>& indexes)
    {
        QUndoCommand* c = cloneMultipleCommand(parentIndex, indexes);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
            SelectionModifier::postAddCommand(_accessor, static_cast<AddRemoveMultipleCommand*>(c)->indexes());
        }
        return c != nullptr;
    }

    QUndoCommand* cloneMultipleCommand(const vectorset<std::pair<index_type, index_type>>& indexes)
    {
        if (indexes.empty()) {
            return nullptr;
        }

        std::vector<std::pair<index_type, index_type>> newIndexes;
        newIndexes.reserve(indexes.size());
        std::vector<DataT> newValues;
        newValues.reserve(indexes.size());

        auto it = indexes.begin();
        while (it != indexes.end()) {
            const index_type parentIndex = it->first;

            const ChildListT* childList = this->getChildList(parentIndex);
            if (childList == nullptr) {
                return nullptr;
            }

            index_type offset = 1;

            while (it != indexes.end() && it->first == parentIndex) {
                const index_type childIndex = it->second;

                if (childIndex < 0 || childIndex >= childList->size()) {
                    return nullptr;
                }

                newValues.emplace_back(childList->at(childIndex));

                newIndexes.emplace_back(parentIndex, childIndex + offset);
                offset++;

                it++;
            }
        }

        const QString& typeName = newIndexes.size() == 1 ? _accessor->typeName() : _accessor->typeNamePlural();

        return new AddMultipleCommand(_accessor, std::move(newIndexes), std::move(newValues), tr("Clone %1").arg(typeName));
    }

    bool cloneMultipleItems(const vectorset<std::pair<index_type, index_type>>& indexes)
    {
        QUndoCommand* c = cloneMultipleCommand(indexes);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
            SelectionModifier::postAddCommand(_accessor, static_cast<AddRemoveMultipleCommand*>(c)->indexes());
        }
        return c != nullptr;
    }

    // will return nullptr if list is not accessable or indexes are invalid
    QUndoCommand* removeMultipleCommand(const index_type parentIndex, const vectorset<index_type>& childIndexes)
    {
        if (childIndexes.empty()) {
            return nullptr;
        }

        const ChildListT* childList = getChildList(parentIndex);
        if (childList == nullptr) {
            return nullptr;
        }
        if (childIndexes.front() < 0 || childIndexes.back() >= childList->size()) {
            return nullptr;
        }

        std::vector<DataT> values;
        values.reserve(childIndexes.size());
        for (const index_type i : childIndexes) {
            values.emplace_back(childList->at(i));
        }

        std::vector<std::pair<index_type, index_type>> indexes;
        indexes.reserve(childIndexes.size());
        for (const index_type i : childIndexes) {
            indexes.emplace_back(parentIndex, i);
        }

        const QString& typeName = indexes.size() == 1 ? _accessor->typeName() : _accessor->typeNamePlural();

        return new RemoveMultipleCommand(_accessor, std::move(indexes), std::move(values), tr("Remove %1").arg(typeName));
    }

    // will return nullptr if list is not accessable or indexes are invalid
    QUndoCommand* removeMultipleCommand(const vectorset<std::pair<index_type, index_type>>& indexes)
    {
        if (indexes.empty()) {
            return nullptr;
        }

        std::vector<DataT> values;
        values.reserve(indexes.size());

        auto it = indexes.begin();
        while (it != indexes.end()) {
            const index_type parentIndex = it->first;

            const ChildListT* childList = this->getChildList(parentIndex);
            if (childList == nullptr) {
                return nullptr;
            }

            while (it != indexes.end() && it->first == parentIndex) {
                const index_type childIndex = it->second;

                if (childIndex < 0 || childIndex >= childList->size()) {
                    return nullptr;
                }

                values.emplace_back(childList->at(childIndex));

                it++;
            }
        }

        const QString& typeName = indexes.size() == 1 ? _accessor->typeName() : _accessor->typeNamePlural();

        return new RemoveMultipleCommand(_accessor, indexes, std::move(values), tr("Remove %1").arg(typeName));
    }

    bool removeMultipleItems(const vectorset<std::pair<index_type, index_type>>& indexes)
    {
        QUndoCommand* c = removeMultipleCommand(indexes);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if list cannot be accessed or indexes are invalid
    QUndoCommand* moveCommand(const index_type fromParentIndex, const index_type fromChildIndex,
                              const index_type toParentIndex, index_type toChildIndex)
    {
        const ChildListT* fromChildList = getChildList(fromParentIndex);
        const ChildListT* toChildList = getChildList(toParentIndex);
        if (fromChildList == nullptr
            || toChildList == nullptr) {

            return nullptr;
        }

        if (fromChildList->empty()
            || fromChildIndex < 0
            || toChildIndex < 0
            || fromChildIndex >= fromChildList->size()) {

            return nullptr;
        }

        if (toChildIndex > toChildList->size()) {
            // Play nice with moveMultipleItemsToChildListCommand
            toChildIndex = toChildList->size();
        }

        const char* text = "Move %1";
        if (fromParentIndex == toParentIndex) {
            if (fromChildIndex == toChildIndex) {
                return nullptr;
            }
            if (toChildIndex == fromChildIndex - 1) {
                text = "Raise %1";
            }
            else if (toChildIndex == 0) {
                text = "Raise %1 To Top";
            }
            else if (toChildIndex == fromChildIndex + 1) {
                text = "Lower %1";
            }
            else if (toChildIndex == toChildList->size() - 1) {
                text = "Lower %1 To Bottom";
            }
        }

        return new MoveCommand(_accessor, fromParentIndex, fromChildIndex, toParentIndex, toChildIndex,
                               tr(text).arg(_accessor->typeName()));
    }

    bool moveItem(const index_type fromParentIndex, const index_type fromChildIndex,
                  const index_type toParentIndex, const index_type toChildIndex)
    {
        QUndoCommand* c = moveCommand(fromParentIndex, fromChildIndex, toParentIndex, toChildIndex);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if list is not accessable or indexes are invalid
    QUndoCommand* moveMultipleCommand(const index_type parentIndex, const vectorset<index_type>& childIndexes, int offset)
    {
        if (childIndexes.empty()) {
            return nullptr;
        }
        if (offset == 0) {
            return nullptr;
        }

        const ChildListT* list = getChildList(parentIndex);
        if (list == nullptr) {
            return nullptr;
        }
        if (childIndexes.front() < 0 || childIndexes.back() >= list->size()) {
            return nullptr;
        }
        if (offset == 0) {
            return nullptr;
        }
        if (offset < 0 && childIndexes.back() < childIndexes.size()) {
            return nullptr;
        }
        if (offset > 0 && childIndexes.front() >= list->size() - childIndexes.size()) {
            return nullptr;
        }

        const char* text = "Move %1";
        if (offset < 0) {
            if (offset == -1) {
                text = "Raise %1";
            }
            else if (unsigned(-offset) > list->size()) {
                text = "Raise %1 To Top";
            }
        }
        else if (offset > 0) {
            if (offset == 1) {
                text = "Lower %1";
            }
            else if (unsigned(offset) > list->size()) {
                text = "Lower %1 To Bottom";
            }
        }
        const QString& typeName = childIndexes.size() == 1 ? _accessor->typeName() : _accessor->typeNamePlural();

        return new MoveMultipleCommand(_accessor, parentIndex, childIndexes, offset, tr(text).arg(typeName));
    }

    bool moveMultipleItems(const index_type parentIndex, const vectorset<index_type>& childIndexes, int offset)
    {
        QUndoCommand* c = moveMultipleCommand(parentIndex, childIndexes, offset);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if list is not accessable or indexes are invalid
    QUndoCommand* moveMultipleCommand(const vectorset<std::pair<index_type, index_type>>& indexes, int offset)
    {
        if (indexes.empty()) {
            return nullptr;
        }

        if (offset == 0) {
            return nullptr;
        }

        const char* text = "Move %1";
        if (offset < 0) {
            if (offset == -1) {
                text = "Raise %1";
            }
        }
        else if (offset > 0) {
            if (offset == 1) {
                text = "Lower %1";
            }
        }

        std::vector<std::pair<index_type, vectorset<index_type>>> indexesToMove;

        auto it = indexes.begin();
        while (it != indexes.end()) {
            const index_type parentIndex = it->first;

            const ChildListT* childList = this->getChildList(parentIndex);
            if (childList == nullptr) {
                return nullptr;
            }

            vectorset<index_type> childIndexes;

            while (it != indexes.end() && it->first == parentIndex) {
                childIndexes.insert(it->second);
                it++;
            }

            // same tests as UndoListHelper::moveMultipleCommand(indexes, offset)
            Q_ASSERT(childIndexes.empty() == false);
            if (offset < 0 && childIndexes.back() < childIndexes.size()) {
                return nullptr;
            }
            if (offset > 0 && childIndexes.front() >= childList->size() - childIndexes.size()) {
                return nullptr;
            }

            indexesToMove.emplace_back(parentIndex, std::move(childIndexes));
        }

        Q_ASSERT(indexesToMove.empty() == false);

        const QString& typeName = indexesToMove.size() == 1 ? _accessor->typeName() : _accessor->typeNamePlural();

        return new MoveMultipleCommand(_accessor, std::move(indexesToMove), offset, tr(text).arg(typeName));
    }

    bool moveMultipleItems(const vectorset<std::pair<index_type, index_type>>& indexes, int offset)
    {
        QUndoCommand* c = moveMultipleCommand(indexes, offset);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if list is not accessable or indexes are invalid
    QUndoCommand* moveMultipleItemsToChildListCommand(const vectorset<std::pair<index_type, index_type>>& indexes, const index_type targetParentIndex)
    {
        const ChildListT* targetList = getChildList(targetParentIndex);
        if (targetList == nullptr) {
            return nullptr;
        }

        std::vector<std::pair<index_type, index_type>> toMove;
        std::vector<DataT> values;

        for (const auto& index : indexes) {
            if (index.first != targetParentIndex) {
                const DataT* data = getChildItem(index.first, index.second);
                if (data) {
                    toMove.push_back(index);
                    values.push_back(*data);
                }
            }
        }

        if (toMove.empty()) {
            return nullptr;
        }

        if (toMove.size() == 1) {
            return moveCommand(toMove.front().first, toMove.front().second, targetParentIndex, INT_MAX);
        }

        return new MoveMultipleToChildListCommand(_accessor, std::move(toMove), std::move(values), targetParentIndex,
                                                  tr("Move %1").arg(_accessor->typeNamePlural()));
    }

    bool moveMultipleItemsToChildList(const vectorset<std::pair<index_type, index_type>>& indexes, const index_type targetParentIndex)
    {
        QUndoCommand* c = moveMultipleItemsToChildListCommand(indexes, targetParentIndex);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }
};

template <class AccessorT>
class NestedListWithNoSelectionUndoHelper : public NestedListUndoHelper<AccessorT, BlankNestedSelectionModifier<AccessorT>> {
public:
    NestedListWithNoSelectionUndoHelper(AccessorT* accessor)
        : NestedListUndoHelper<AccessorT, BlankNestedSelectionModifier<AccessorT>>(accessor)
    {
    }
};

template <class AccessorT>
class NestedListAndMultipleSelectionUndoHelper : public NestedListUndoHelper<AccessorT, MultipleNestedSelectionModifier<AccessorT>> {

public:
    using index_type = typename AccessorT::index_type;

public:
    NestedListAndMultipleSelectionUndoHelper(AccessorT* accessor)
        : NestedListUndoHelper<AccessorT, MultipleNestedSelectionModifier<AccessorT>>(accessor)
    {
    }

    template <typename EditFunction>
    QUndoCommand* editSelectedItemsCommand(const QString& text, EditFunction editFunction)
    {
        return this->editMultipleItemsCommand(this->_accessor->selectedIndexes(), text, editFunction);
    }

    template <typename EditFunction>
    bool editSelectedItems(const QString& text, EditFunction editFunction)
    {
        return this->editMultipleItems(this->_accessor->selectedIndexes(), text, editFunction);
    }

    template <typename FieldT, typename UnaryFunction>
    bool setFieldInSelectedItems(const FieldT& newValue, const QString& text,
                                 UnaryFunction getter)
    {
        return this->setFieldInMultipleItems(this->_accessor->selectedIndexes(), newValue, text, getter);
    }
};

}
}
}
