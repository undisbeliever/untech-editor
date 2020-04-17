/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractaccessors.h"
#include "movemultipleundohelper.h"
#include "models/common/vector-helpers.h"
#include "models/common/vectorset.h"
#include <QCoreApplication>
#include <QUndoCommand>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Accessor {

/*
 * ListUndoHelper is also responsible for managing the selection when:
 *
 *  * An item is added
 *  * An item is removed, or
 *  * An item is moved
 *
 * This used to be the responsibility of the accessor class but the
 * interactions of the various signals and slots would occasionally cause other
 * subsystems (usually QAbstractItemView) to mangle the selection.
 *
 * By moving the selection change into the Undo Command (after the list signals
 * have been emitted) we ensure the same item remains selected after the
 * QUndoCommand is invoked.
 */

struct BlankSelectionModifier {
    using index_type = size_t;
    using selection_type = std::tuple<>;

    inline static selection_type getSelection(const void*) { return selection_type(); }
    inline static void setSelection(const void*, const selection_type&) {}

    inline static void postAddCommand(const void*, const index_type) {}
    inline static void postAddCommand(const void*, const std::vector<index_type>&) {}

    inline static void itemAdded(selection_type&, const index_type) {}
    inline static void itemRemoved(selection_type&, const index_type) {}
    inline static void itemMoved(selection_type&, const index_type, const index_type) {}
};

struct SingleSelectionModifier {
    using AccessorT = AbstractListSingleSelectionAccessor;
    using index_type = AccessorT::index_type;
    using selection_type = index_type;

    inline static selection_type getSelection(const AccessorT* a) { return a->selectedIndex(); }
    inline static void setSelection(AccessorT* a, const selection_type& selectedIndex) { a->setSelectedIndex(selectedIndex); }

    inline static void postAddCommand(AccessorT* a, const index_type index) { a->setSelectedIndex(index); }
    inline static void postAddCommand(AccessorT* a, const std::vector<index_type>&) { a->setSelectedIndex(INT_MAX); }

    static void itemAdded(selection_type& selectedIndex, const index_type index)
    {
        if (selectedIndex >= index) {
            selectedIndex++;
        }
    }

    static void itemRemoved(selection_type& selectedIndex, const index_type index)
    {
        if (selectedIndex == index) {
            selectedIndex = INT_MAX;
        }
        else if (selectedIndex > index) {
            selectedIndex--;
        }
    }

    static void itemMoved(selection_type& selectedIndex, const index_type from, const index_type to)
    {
        if (selectedIndex == from) {
            selectedIndex = to;
        }
        else if (selectedIndex > from && selectedIndex <= to) {
            selectedIndex--;
        }
        else if (selectedIndex >= to && selectedIndex < from) {
            selectedIndex++;
        }
    }
};

struct MultipleSelectionModifier {
    using AccessorT = AbstractListMultipleSelectionAccessor;
    using index_type = AccessorT::index_type;
    using selection_type = std::vector<index_type>;

    inline static selection_type getSelection(const AccessorT* a) { return a->selectedIndexes(); }
    inline static void setSelection(AccessorT* a, selection_type&& selection) { a->setSelectedIndexes(std::move(selection)); }

    inline static void postAddCommand(AccessorT* a, const index_type index) { a->setSelectedIndexes({ index }); }
    inline static void postAddCommand(AccessorT* a, const std::vector<index_type>& indexes) { a->setSelectedIndexes(indexes); }

    static void itemAdded(selection_type& selection, const index_type index)
    {
        for (index_type& sel : selection) {
            if (sel >= index) {
                sel++;
            }
        }
    }

    static void itemRemoved(selection_type& selection, const index_type index)
    {
        selection.erase(std::remove(selection.begin(), selection.end(), index),
                        selection.end());

        for (index_type& sel : selection) {
            if (sel > index) {
                sel--;
            }
        }
    }

    static void itemMoved(selection_type& selection, const index_type from, const index_type to)
    {
        for (index_type& sel : selection) {
            if (sel == from) {
                sel = to;
            }
            else if (sel > from && sel <= to) {
                sel--;
            }
            else if (sel >= to && sel < from) {
                sel++;
            }
        }
    }
};

template <class AccessorT, class SelectionModifier>
class ListUndoHelper {

public:
    using DataT = typename AccessorT::DataT;
    using ListT = typename AccessorT::ListT;
    using index_type = typename AccessorT::index_type;
    using ArgsT = typename AccessorT::ArgsT;

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

    private:
        constexpr inline auto function_args() const
        {
            return std::tuple_cat(std::make_tuple(_accessor), _args);
        }

        constexpr inline auto function_args(index_type index) const
        {
            return std::tuple_cat(std::make_tuple(_accessor), _args, std::make_tuple(index));
        }

        constexpr inline auto function_args(index_type index1, index_type index2) const
        {
            return std::tuple_cat(std::make_tuple(_accessor), _args, std::make_tuple(index1, index2));
        }

    protected:
        inline const ListT* getListPtr() const
        {
            const ListT* l = std::apply(&AccessorT::getList, function_args());
            Q_ASSERT(l);
            return l;
        }

        inline ListT& getList()
        {
            ListT* l = std::apply(&AccessorT::getList, function_args());
            Q_ASSERT(l);
            return *l;
        }

        inline DataT& getItem(const index_type index)
        {
            ListT* l = std::apply(&AccessorT::getList, function_args());
            Q_ASSERT(l);
            Q_ASSERT(index >= 0 && index < l->size());
            return l->at(index);
        }

        inline void emitDataChanged(index_type index)
        {
            std::apply(&AccessorT::dataChanged, function_args(index));

            _accessor->resourceItem()->dataChanged();
        }

        template <typename ExtraSignalsFunction>
        inline void emitDataChanged(index_type index, ExtraSignalsFunction extraSignals)
        {
            auto args = function_args(index);
            std::apply(&AccessorT::dataChanged, args);
            std::apply(extraSignals, args);

            _accessor->resourceItem()->dataChanged();
        }

        inline void emitListChanged()
        {
            std::apply(&AccessorT::listChanged, function_args());

            _accessor->resourceItem()->dataChanged();
        }

        // When this signal is emitted you MUST close all editors
        // accessing the list to prevent data corruption
        inline void emitListAboutToChange()
        {
            std::apply(&AccessorT::listAboutToChange, function_args());
        }

        inline void emitItemAdded(index_type index)
        {
            std::apply(&AccessorT::itemAdded, function_args(index));
        }

        inline void emitItemAboutToBeRemoved(index_type index)
        {
            std::apply(&AccessorT::itemAboutToBeRemoved, function_args(index));
        }

        inline void emitItemMoved(index_type from, index_type to)
        {
            std::apply(&AccessorT::itemMoved, function_args(from, to));
        }
    };

    class EditCommand : public BaseCommand {
    private:
        const index_type _index;
        const DataT _oldValue;
        const DataT _newValue;

        inline void setValue(const DataT& value)
        {
            DataT& item = this->getItem(_index);

            item = value;
            this->emitDataChanged(_index);
        }

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
        const index_type _index;
        const DataT _oldValue;
        DataT _newValue;
        const bool _first;

        inline void setValue(const DataT& value)
        {
            DataT& item = this->getItem(_index);

            item = value;
            this->emitDataChanged(_index);
        }

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
                && command->_args == this->_args
                && command->_index == this->_index
                && command->_oldValue == this->_newValue
                && command->getListPtr() == this->getListPtr()) {

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

        inline void setField(const FieldT& value)
        {
            DataT& item = this->getItem(_index);
            FieldT& field = _getter(item);

            field = value;
            this->emitDataChanged(_index, _signalEmitter);
        }

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
        const index_type _index;
        const std::tuple<FieldT...> _oldValues;
        const std::tuple<FieldT...> _newValues;
        const UnaryFunction _getter;

        inline void setValue(const std::tuple<FieldT...>& values)
        {
            DataT& item = this->getItem(_index);
            std::tuple<FieldT&...> fields = _getter(item);

            fields = values;
            this->emitDataChanged(_index);
        }

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
            setValue(_oldValues);
        }

        virtual void redo() final
        {
            setValue(_newValues);
        }
    };

    template <typename FieldT, typename UnaryFunction>
    class EditFieldIncompleteCommand : public BaseCommand {
    private:
        const index_type _index;
        const FieldT _oldValue;
        FieldT _newValue;
        const UnaryFunction _getter;

        inline void setField(const FieldT& value)
        {
            DataT& item = this->getItem(_index);
            FieldT& field = _getter(item);

            field = value;
            this->emitDataChanged(_index);
        }

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

    class EditMultipleItemsCommand : public BaseCommand {
    private:
        const std::vector<index_type> _indexes;
        const std::vector<DataT> _oldValues;
        const std::vector<DataT> _newValues;

        void setData(const std::vector<DataT>& values)
        {
            Q_ASSERT(_indexes.size() == values.size());

            ListT& list = this->getList();

            auto it = values.begin();
            for (index_type index : _indexes) {
                Q_ASSERT(index >= 0 && index < list.size());
                DataT& item = list.at(index);

                item = *it++;
                this->emitDataChanged(index);
            }
            Q_ASSERT(it == values.end());
        }

    public:
        EditMultipleItemsCommand(AccessorT* accessor, const ArgsT& args,
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
    class EditMultipleItemsFieldCommand : public BaseCommand {
    private:
        const std::vector<index_type> _indexes;
        const std::vector<FieldT> _oldValues;
        const FieldT _newValue;
        const UnaryFunction _getter;

    public:
        EditMultipleItemsFieldCommand(AccessorT* accessor, const ArgsT& args,
                                      std::vector<index_type>&& indexes,
                                      std::vector<FieldT>&& oldValues,
                                      const FieldT& newValue,
                                      const QString& text,
                                      UnaryFunction getter)
            : BaseCommand(accessor, args, text)
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
            ListT& list = this->getList();

            auto it = _oldValues.begin();
            for (const index_type index : _indexes) {
                Q_ASSERT(index >= 0 && index < list.size());
                DataT& item = list.at(index);
                FieldT& field = _getter(item);

                field = *it++;
                this->emitDataChanged(index);
            }
            Q_ASSERT(it == _oldValues.end());
        }

        virtual void redo() final
        {
            ListT& list = this->getList();

            for (const index_type index : _indexes) {
                Q_ASSERT(index >= 0 && index < list.size());
                DataT& item = list.at(index);
                FieldT& field = _getter(item);

                field = _newValue;
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

    public:
        index_type index() const { return _index; }

    private:
        void addItem_NoUpdateSelection()
        {
            ListT& list = this->getList();
            Q_ASSERT(_index >= 0 && _index <= list.size());

            this->emitListAboutToChange();

            list.insert(list.begin() + _index, _value);

            this->emitItemAdded(_index);
            this->emitListChanged();
        }

        void removeItem_NoUpdateSelection()
        {
            ListT& list = this->getList();
            Q_ASSERT(_index >= 0 && _index < list.size());

            this->emitListAboutToChange();
            this->emitItemAboutToBeRemoved(_index);

            list.erase(list.begin() + _index);

            this->emitListChanged();
        }

    protected:
        void addItem()
        {
            if (this->_args == this->_accessor->selectedListTuple()) {
                auto selection = SelectionModifier::getSelection(this->_accessor);
                addItem_NoUpdateSelection();
                SelectionModifier::itemAdded(selection, _index);
                SelectionModifier::setSelection(this->_accessor, std::move(selection));
            }
            else {
                addItem_NoUpdateSelection();
            }
        }

        void removeItem()
        {
            if (this->_args == this->_accessor->selectedListTuple()) {
                auto selection = SelectionModifier::getSelection(this->_accessor);
                removeItem_NoUpdateSelection();
                SelectionModifier::itemRemoved(selection, _index);
                SelectionModifier::setSelection(this->_accessor, std::move(selection));
            }
            else {
                removeItem_NoUpdateSelection();
            }
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

        void moveItem_NoUpdateSelection(const index_type from, const index_type to)
        {
            ListT& list = this->getList();
            Q_ASSERT(from != to);
            Q_ASSERT(from >= 0 && from < list.size());
            Q_ASSERT(to >= 0 && to < list.size());

            this->emitListAboutToChange();

            moveListItem(from, to, list);

            this->emitItemMoved(from, to);
            this->emitListChanged();
        }

        void moveItem(const index_type from, const index_type to)
        {
            if (this->_args == this->_accessor->selectedListTuple()) {
                auto selection = SelectionModifier::getSelection(this->_accessor);
                moveItem_NoUpdateSelection(from, to);
                SelectionModifier::itemMoved(selection, from, to);
                SelectionModifier::setSelection(this->_accessor, std::move(selection));
            }
            else {
                moveItem_NoUpdateSelection(from, to);
            }
        }

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
    };

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

        const vectorset<index_type>& indexes() const { return _indexes; }

    private:
        void addItems_NoUpdateSelection()
        {
            ListT& list = this->getList();

            this->emitListAboutToChange();

            auto iit = _indexes.begin();
            auto vit = _values.begin();

            for (; iit != _indexes.end(); iit++, vit++) {
                const auto& index = *iit;
                const auto& value = *vit;
                Q_ASSERT(index >= 0 && index <= list.size());

                list.insert(list.begin() + index, value);

                this->emitItemAdded(index);
            }
            Q_ASSERT(vit == _values.end());

            this->emitListChanged();
        }

        void removeItems_NoUpdateSelection()
        {
            ListT& list = this->getList();

            this->emitListAboutToChange();

            for (auto it = _indexes.rbegin(); it != _indexes.rend(); it++) {
                const auto& index = *it;
                Q_ASSERT(index >= 0 && index < list.size());

                this->emitItemAboutToBeRemoved(index);

                list.erase(list.begin() + index);
            }

            this->emitListChanged();
        }

    protected:
        void addItems()
        {
            if (this->_args == this->_accessor->selectedListTuple()) {
                auto selection = SelectionModifier::getSelection(this->_accessor);
                addItems_NoUpdateSelection();
                for (const auto& index : _indexes) {
                    SelectionModifier::itemAdded(selection, index);
                }
                SelectionModifier::setSelection(this->_accessor, std::move(selection));
            }
            else {
                addItems_NoUpdateSelection();
            }
        }

        void removeItems()
        {
            if (this->_args == this->_accessor->selectedListTuple()) {
                auto selection = SelectionModifier::getSelection(this->_accessor);
                removeItems_NoUpdateSelection();
                for (auto it = _indexes.rbegin(); it != _indexes.rend(); it++) {
                    SelectionModifier::itemRemoved(selection, *it);
                }
                SelectionModifier::setSelection(this->_accessor, std::move(selection));
            }
            else {
                removeItems_NoUpdateSelection();
            }
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
        const vectorset<index_type> _indexes;
        const MoveMultipleDirection _direction;

    public:
        MoveMultipleCommand(AccessorT* accessor, const ArgsT& args,
                            const vectorset<index_type>& indexes,
                            const MoveMultipleDirection direction,
                            const QString& text)
            : BaseCommand(accessor, args, text)
            , _indexes(indexes)
            , _direction(direction)
        {
        }
        ~MoveMultipleCommand() = default;

        void undo()
        {
            ListT& list = this->getList();

            const bool listIsSelected = this->_args == this->_accessor->selectedListTuple();

            auto selection = listIsSelected ? SelectionModifier::getSelection(this->_accessor)
                                            : typename SelectionModifier::selection_type();

            this->emitListAboutToChange();

            MoveMultipleUndoHelper::undo(_direction, _indexes, list.size(),
                                         [&](index_type from, index_type to) {
                                             moveListItem(from, to, list);
                                             this->emitItemMoved(from, to);
                                             SelectionModifier::itemMoved(selection, from, to);
                                         });

            this->emitListChanged();

            if (listIsSelected) {
                SelectionModifier::setSelection(this->_accessor, std::move(selection));
            }
        }

        void redo()
        {
            ListT& list = this->getList();

            const bool listIsSelected = this->_args == this->_accessor->selectedListTuple();

            auto selection = listIsSelected ? SelectionModifier::getSelection(this->_accessor)
                                            : typename SelectionModifier::selection_type();

            this->emitListAboutToChange();

            MoveMultipleUndoHelper::redo(_direction, _indexes, list.size(),
                                         [&](const index_type from, const index_type to) {
                                             moveListItem(from, to, list);
                                             this->emitItemMoved(from, to);
                                             SelectionModifier::itemMoved(selection, from, to);
                                         });

            this->emitListChanged();

            if (listIsSelected) {
                SelectionModifier::setSelection(this->_accessor, std::move(selection));
            }
        }
    };

protected:
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
        return std::apply(&AccessorT::getList,
                          std::tuple_cat(std::make_tuple(_accessor), listArgs));
    }

    inline const DataT* getItem(const ArgsT& listArgs, const index_type index)
    {
        const ListT* l = getList(listArgs);
        if (l == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= l->size()) {
            return nullptr;
        }
        return &l->at(index);
    }

    // MUST ONLY this function when it's absolutely necessary
    inline ListT* getList_NO_CONST(const ArgsT& listArgs)
    {
        return std::apply(&AccessorT::getList,
                          std::tuple_cat(std::make_tuple(_accessor), listArgs));
    }

    // MUST ONLY this function when it's absolutely necessary
    inline DataT* getItem_NO_CONST(const ArgsT& listArgs, const index_type index)
    {
        ListT* l = getList_NO_CONST(listArgs);
        if (l == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= l->size()) {
            return nullptr;
        }
        return &l->at(index);
    }

public:
    // will return nullptr if data cannot be accessed or is equal to newValue
    QUndoCommand* editItemCommand(index_type index, const DataT& newValue)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        const DataT* oldValue = getItem(listArgs, index);
        if (oldValue == nullptr) {
            return nullptr;
        }

        if (*oldValue == newValue) {
            return nullptr;
        }
        return new EditCommand(_accessor, listArgs, index, *oldValue, newValue);
    }

    bool editItem(index_type index, const DataT& newValue)
    {
        QUndoCommand* e = editItemCommand(index, newValue);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    // will return nullptr if data cannot be accessed or is unchanged
    template <typename EditFunction>
    QUndoCommand* editItemCommand(index_type index, const QString& text,
                                  EditFunction editFunction)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        const DataT* oldValue = getItem(listArgs, index);
        if (oldValue == nullptr) {
            return nullptr;
        }

        DataT newValue = *oldValue;

        editFunction(newValue);

        if (newValue == *oldValue) {
            return nullptr;
        }
        return new EditCommand(_accessor, listArgs, index, *oldValue, newValue, text);
    }

    template <typename EditFunction>
    bool editItem(index_type index, const QString& text,
                  EditFunction editFunction)
    {
        QUndoCommand* e = editItemCommand(index, text, editFunction);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    // will return nullptr if data cannot be accessed or is equal to newValue
    QUndoCommand* editMergeCommand(index_type index, const DataT& newValue, bool first = false)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        const DataT* oldValue = getItem(listArgs, index);
        if (oldValue == nullptr) {
            return nullptr;
        }

        if (*oldValue == newValue) {
            return nullptr;
        }

        return new EditMergeCommand(_accessor, listArgs, index, *oldValue, newValue, first);
    }

    bool editMerge(index_type index, const DataT& newValue, bool first = false)
    {
        QUndoCommand* c = editMergeCommand(index, newValue, first);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if data cannot be accessed or is equal to newValue
    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    QUndoCommand* editFieldCommand(index_type index, const FieldT& newValue, const QString& text,
                                   UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        DataT* item = getItem_NO_CONST(listArgs, index);
        if (item == nullptr) {
            return nullptr;
        }

        const FieldT& oldValue = getter(*item);
        if (oldValue == newValue) {
            return nullptr;
        }

        return new EditFieldCommand<FieldT, UnaryFunction, ExtraSignalsFunction>(
            _accessor, listArgs, index, oldValue, newValue, text, getter, extraSignals);
    }

    // will return nullptr if data cannot be accessed or is equal to newValue
    template <typename FieldT, typename UnaryFunction>
    QUndoCommand* editFieldCommand(index_type index, const FieldT& newValue, const QString& text,
                                   UnaryFunction getter)
    {
        return editFieldCommand(index, newValue, text, getter, EmptySignalFunction<ArgsT>());
    }

    template <typename FieldT, typename UnaryFunction>
    bool editField(index_type index, const FieldT& newValue, const QString& text,
                   UnaryFunction getter)
    {
        QUndoCommand* e = editFieldCommand(index, newValue, text, getter);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    bool editField(index_type index, const FieldT& newValue, const QString& text,
                   UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        QUndoCommand* e = editFieldCommand(index, newValue, text, getter, extraSignals);
        if (e) {
            _accessor->resourceItem()->undoStack()->push(e);
        }
        return e != nullptr;
    }

    // will return nullptr if data cannot be accessed or is equal to newValues
    template <typename... FieldT, typename UnaryFunction>
    QUndoCommand* editMulitpleFieldsCommand(index_type index, const std::tuple<FieldT...>& newValues,
                                            const QString& text,
                                            UnaryFunction getter)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        DataT* item = getItem_NO_CONST(listArgs, index);
        if (item == nullptr) {
            return nullptr;
        }

        const std::tuple<FieldT&...> oldValues = getter(*item);

        if (oldValues == newValues) {
            return nullptr;
        }

        return new EditMultipleFieldsCommand<UnaryFunction, FieldT...>(
            _accessor, listArgs, index, oldValues, newValues, text, getter);
    }

    template <typename... FieldT, typename UnaryFunction>
    bool editMultipleFields(index_type index, const std::tuple<FieldT...>& newValues,
                            const QString& text,
                            UnaryFunction getter)
    {
        QUndoCommand* c = editMulitpleFieldsCommand(index, newValues, text, getter);
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
    editFieldIncompleteCommand(index_type index, const QString& text,
                               UnaryFunction getter)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        DataT* item = getItem_NO_CONST(listArgs, index);
        if (item == nullptr) {
            return nullptr;
        }

        const FieldT& oldValue = getter(*item);

        return std::make_unique<EditFieldIncompleteCommand<FieldT, UnaryFunction>>(
            _accessor, listArgs, index, oldValue, text, getter);
    }

    // will return nullptr if data cannot be accessed or unchanged
    template <typename EditFunction>
    QUndoCommand* editMultipleItemsCommand(const vectorset<index_type>& indexes, const QString& text,
                                           EditFunction editFunction)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        const ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }

        std::vector<index_type> indexesEdited;
        std::vector<DataT> oldValues;
        std::vector<DataT> newValues;

        indexesEdited.reserve(indexes.size());
        oldValues.reserve(indexes.size());
        newValues.reserve(indexes.size());

        for (const index_type& index : indexes) {
            if (index >= 0 && index < list->size()) {
                const DataT& oldValue = list->at(index);
                DataT newValue = list->at(index);

                editFunction(newValue, index);

                if (newValue != oldValue) {
                    indexesEdited.push_back(index);
                    oldValues.push_back(oldValue);
                    newValues.push_back(std::move(newValue));
                }
            }
        }

        if (!indexesEdited.empty()) {
            return new EditMultipleItemsCommand(
                _accessor, listArgs,
                std::move(indexesEdited), std::move(oldValues), std::move(newValues),
                text);
        }
        else {
            return nullptr;
        }
    }

    template <typename EditFunction>
    bool editMultipleItems(const vectorset<index_type>& indexes, const QString& text,
                           EditFunction editFunction)
    {
        QUndoCommand* c = editMultipleItemsCommand(indexes, text, editFunction);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    template <typename FieldT, typename UnaryFunction>
    QUndoCommand* setFieldInMultipleItemsCommand(const vectorset<index_type>& indexes, const FieldT& newValue,
                                                 const QString& text, UnaryFunction getter)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        ListT* list = getList_NO_CONST(listArgs);
        if (list == nullptr) {
            return nullptr;
        }

        std::vector<index_type> indexesEdited;
        std::vector<FieldT> oldValues;

        indexesEdited.reserve(indexes.size());
        oldValues.reserve(indexes.size());

        for (const index_type& index : indexes) {
            if (index >= 0 && index < list->size()) {
                DataT& data = list->at(index);
                const FieldT& oldValue = getter(data);

                if (newValue != oldValue) {
                    indexesEdited.push_back(index);
                    oldValues.emplace_back(oldValue);
                }
            }
        }

        if (!indexesEdited.empty()) {
            return new EditMultipleItemsFieldCommand<FieldT, UnaryFunction>(
                _accessor, listArgs,
                std::move(indexesEdited), std::move(oldValues), newValue,
                text, getter);
        }
        else {
            return nullptr;
        }
    }

    template <typename FieldT, typename UnaryFunction>
    bool setFieldInMultipleItems(const vectorset<index_type>& indexes, const FieldT& newValue,
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
    QUndoCommand* editAllItemsCommand(const QString& text, EditFunction editFunction)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
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
            return new EditMultipleItemsCommand(
                _accessor, listArgs,
                std::move(indexesEdited), std::move(oldValues), std::move(newValues),
                text);
        }
        else {
            return nullptr;
        }
    }

    template <typename EditFunction>
    bool editAllItems(const QString& text, EditFunction editFunction)
    {
        QUndoCommand* c = editAllItemsCommand(text, editFunction);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* addCommand(index_type index, DataT item, const QString& text)
    {
        const ArgsT listArgs = selectedListTuple();
        const ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0
            || list->size() >= _accessor->maxSize()) {

            return nullptr;
        }
        if (index > list->size()) {
            index = list->size();
        }

        return new AddCommand(_accessor, listArgs, index, item, text);
    }

    QUndoCommand* addCommand(index_type index, DataT item = DataT())
    {
        return addCommand(index, std::move(item),
                          tr("Add %1").arg(_accessor->typeName()));
    }

    bool addItem(index_type index, DataT item, const QString& text)
    {
        QUndoCommand* c = addCommand(index, std::move(item), text);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
            SelectionModifier::postAddCommand(_accessor, static_cast<AddRemoveCommand*>(c)->index());
        }
        return c != nullptr;
    }

    bool addItem(index_type index, DataT item = DataT())
    {
        return addItem(index, std::move(item),
                       tr("Add %1").arg(_accessor->typeName()));
    }

    bool addItem()
    {
        return addItem(INT_MAX);
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* cloneCommand(index_type index)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
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

    bool cloneItem(index_type index)
    {
        QUndoCommand* c = cloneCommand(index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
            SelectionModifier::postAddCommand(_accessor, static_cast<AddRemoveCommand*>(c)->index());
        }
        return c != nullptr;
    }

    // will return nullptr if list cannot be accessed,
    // index is invalid or too many items in list
    QUndoCommand* removeCommand(index_type index)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        const ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (index < 0 || index >= list->size()) {
            return nullptr;
        }

        return new RemoveCommand(_accessor, listArgs, index, list->at(index));
    }

    bool removeItem(index_type index)
    {
        QUndoCommand* c = removeCommand(index);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    QUndoCommand* cloneMultipleCommand(const vectorset<index_type>& indexes)
    {
        if (indexes.empty()) {
            return nullptr;
        }

        const ArgsT listArgs = _accessor->selectedListTuple();
        const ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (indexes.front() < 0 || indexes.back() >= list->size()) {
            return nullptr;
        }
        if (list->size() + indexes.size() > _accessor->maxSize()) {
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

        const QString& typeName = newIndexes.size() == 1 ? _accessor->typeName() : _accessor->typeNamePlural();

        return new AddMultipleCommand(_accessor, listArgs, std::move(newIndexes), std::move(newValues),
                                      tr("Clone %1").arg(typeName));
    }

    bool cloneMultipleItems(const vectorset<index_type>& indexes)
    {
        QUndoCommand* c = cloneMultipleCommand(indexes);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
            SelectionModifier::postAddCommand(_accessor, static_cast<AddRemoveMultipleCommand*>(c)->indexes());
        }
        return c != nullptr;
    }

    // will return nullptr if list is not accessable or indexes are invalid
    QUndoCommand* removeMultipleCommand(const vectorset<index_type>& indexes)
    {
        if (indexes.empty()) {
            return nullptr;
        }

        const ArgsT listArgs = _accessor->selectedListTuple();
        const ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (indexes.front() < 0 || indexes.back() >= list->size()) {
            return nullptr;
        }

        std::vector<DataT> values;
        values.reserve(indexes.size());
        for (const index_type& i : indexes) {
            values.emplace_back(list->at(i));
        }

        const QString& typeName = indexes.size() == 1 ? _accessor->typeName() : _accessor->typeNamePlural();

        return new RemoveMultipleCommand(_accessor, listArgs, indexes, std::move(values),
                                         tr("Remove %1").arg(typeName));
    }

    bool removeMultipleItems(const vectorset<index_type>& indexes)
    {
        QUndoCommand* c = removeMultipleCommand(indexes);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if list cannot be accessed or indexes are invalid
    QUndoCommand* moveCommand(index_type from, index_type to)
    {
        const ArgsT listArgs = _accessor->selectedListTuple();
        const ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }
        if (list->empty()
            || from < 0
            || to < 0) {
            return nullptr;
        }
        if (from >= list->size()) {
            from = list->size() - 1;
        }
        if (to >= list->size()) {
            to = list->size() - 1;
        }
        if (from == to) {
            return nullptr;
        }

        const char* text = "Move %1";
        if (to == from - 1) {
            text = "Raise %1";
        }
        else if (to == 0) {
            text = "Raise %1 To Top";
        }
        else if (to == from + 1) {
            text = "Lower %1";
        }
        else if (to == list->size() - 1) {
            text = "Lower %1 To Bottom";
        }

        return new MoveCommand(_accessor, listArgs, from, to,
                               tr(text).arg(_accessor->typeName()));
    }

    bool moveItem(index_type from, index_type to)
    {
        QUndoCommand* c = moveCommand(from, to);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }

    // will return nullptr if list is not accessable or indexes are invalid
    QUndoCommand* moveMultipleCommand(const vectorset<index_type>& indexes, const MoveMultipleDirection direction)
    {
        if (indexes.empty()) {
            return nullptr;
        }

        const ArgsT listArgs = _accessor->selectedListTuple();
        const ListT* list = getList(listArgs);
        if (list == nullptr) {
            return nullptr;
        }

        if (MoveMultipleUndoHelper::canMove(direction, indexes, list->size()) == false) {
            return nullptr;
        }

        const char* text = MoveMultipleUndoHelper::getText(direction);
        const QString& typeName = indexes.size() == 1 ? _accessor->typeName() : _accessor->typeNamePlural();

        return new MoveMultipleCommand(_accessor, listArgs, indexes, direction,
                                       tr(text).arg(typeName));
    }

    bool moveMultipleItems(const vectorset<index_type>& indexes, const MoveMultipleDirection direction)
    {
        QUndoCommand* c = moveMultipleCommand(indexes, direction);
        if (c) {
            _accessor->resourceItem()->undoStack()->push(c);
        }
        return c != nullptr;
    }
};

template <class AccessorT>
class ListWithNoSelectionUndoHelper : public ListUndoHelper<AccessorT, BlankSelectionModifier> {
public:
    ListWithNoSelectionUndoHelper(AccessorT* accessor)
        : ListUndoHelper<AccessorT, BlankSelectionModifier>(accessor)
    {
    }
};

template <class AccessorT>
class ListAndSelectionUndoHelper : public ListUndoHelper<AccessorT, SingleSelectionModifier> {

public:
    using DataT = typename AccessorT::DataT;
    using index_type = typename AccessorT::index_type;

public:
    ListAndSelectionUndoHelper(AccessorT* accessor)
        : ListUndoHelper<AccessorT, SingleSelectionModifier>(accessor)
    {
    }

    bool editSelectedItem(const DataT& newValue)
    {
        return this->editItem(this->_accessor->selectedIndex(), newValue);
    }

    template <typename FieldT, typename UnaryFunction>
    QUndoCommand* editSelectedItemFieldCommand(const FieldT& newValue, const QString& text,
                                               UnaryFunction getter)
    {
        return this->editFieldCommand(this->_accessor->selectedIndex(), newValue, text, getter);
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    QUndoCommand* editSelectedItemFieldCommand(const FieldT& newValue, const QString& text,
                                               UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        return this->editFieldCommand(this->_accessor->selectedIndex(), newValue, text, getter, extraSignals);
    }

    template <typename FieldT, typename UnaryFunction>
    bool editSelectedItemField(const FieldT& newValue, const QString& text,
                               UnaryFunction getter)
    {
        return this->editField(this->_accessor->selectedIndex(), newValue, text, getter);
    }

    template <typename FieldT, typename UnaryFunction, typename ExtraSignalsFunction>
    bool editSelectedItemField(const FieldT& newValue, const QString& text,
                               UnaryFunction getter, ExtraSignalsFunction extraSignals)
    {
        return this->editField(this->_accessor->selectedIndex(), newValue, text, getter, extraSignals);
    }

    template <typename... FieldT, typename UnaryFunction>
    bool editSelectedItemMultipleFields(const std::tuple<FieldT...>& newValues, const QString& text,
                                        UnaryFunction getter)
    {
        return this->editMultipleFields(this->_accessor->selectedIndex(), newValues, text, getter);
    }

    // will return nullptr if data cannot be accessed
    template <typename FieldT, typename UnaryFunction>
    auto editSelectedItemFieldIncompleteCommand(const QString& text, UnaryFunction getter)
    {
        return this->template editFieldIncompleteCommand<FieldT>(
            this->_accessor->selectedIndex(), text, getter);
    }

    bool cloneSelectedItem()
    {
        return this->cloneItem(this->_accessor->selectedIndex());
    }

    bool removeSelectedItem()
    {
        return this->removeItem(this->_accessor->selectedIndex());
    }

    bool raiseSelectedItemToTop()
    {
        const index_type index = this->_accessor->selectedIndex();
        return this->moveItem(index, 0);
    }

    bool raiseSelectedItem()
    {
        const index_type index = this->_accessor->selectedIndex();
        if (index == 0) {
            return false;
        }
        return this->moveItem(index, index - 1);
    }

    bool lowerSelectedItem()
    {
        const index_type index = this->_accessor->selectedIndex();
        return this->moveItem(index, index + 1);
    }

    bool lowerSelectedItemToBottom()
    {
        const index_type index = this->_accessor->selectedIndex();
        return this->moveItem(index, INT_MAX);
    }
};

template <class AccessorT>
class ListAndMultipleSelectionUndoHelper : public ListUndoHelper<AccessorT, MultipleSelectionModifier> {

public:
    using index_type = typename AccessorT::index_type;

public:
    ListAndMultipleSelectionUndoHelper(AccessorT* accessor)
        : ListUndoHelper<AccessorT, MultipleSelectionModifier>(accessor)
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

    bool cloneSelectedItems()
    {
        return this->cloneMultipleItems(this->_accessor->selectedIndexes());
    }

    bool removeSelectedItems()
    {
        return this->removeMultipleItems(this->_accessor->selectedIndexes());
    }

    bool moveSelectedItems(const MoveMultipleDirection direction)
    {
        return this->moveMultipleItems(this->_accessor->selectedIndexes(), direction);
    }
};
}
}
}
