#pragma once

#include "orderedlistcontroller.h"

#include "gui/controllers/undo/undostack.h"
#include "models/common/orderedlist.h"

#include <cassert>
#include <sigc++/signal.h>

namespace UnTech {
namespace Controller {

namespace Private {
// This class is tightly coupled with the `UnTech::orderedlist` template.
template <class T>
class OrderedListAddRemove {
public:
    OrderedListAddRemove(typename T::list_t* list, T* item,
                         OrderedListController<T>& controller)
        : _list(list)
        , _item(nullptr)
        , _controller(controller)
    {
        int index = _list->indexOf(item);
        assert(index >= 0);
        _index = (size_t)index;
    }

    OrderedListAddRemove(const OrderedListAddRemove&) = delete;
    ~OrderedListAddRemove() = default;

    void add()
    {
        _list->insertAtIndex(std::move(_item), _index);
        _controller.signal_listDataChanged().emit(_list);
    }

    void remove()
    {
        _item = _list->removeFromList(_index);

        // prevent access after item is removed
        if (_item.get() == _controller.selected()) {
            _controller.setSelected(nullptr);
        }
        _controller.signal_listDataChanged().emit(_list);
    }

private:
    typename T::list_t* _list;
    std::unique_ptr<T> _item;
    size_t _index;
    OrderedListController<T>& _controller;
};
}

template <class T>
OrderedListController<T>::OrderedListController(BaseController& baseController)
    : _baseController(baseController)
    , _list(nullptr)
    , _selected(nullptr)
{
    _signal_dataChanged.connect([this](const T* item) {
        if (item == _selected) {
            _signal_selectedDataChanged.emit();
        }
    });
}

template <class T>
void OrderedListController<T>::setList(typename T::list_t* list)
{
    if (_list != list) {
        if (_selected != nullptr) {
            _selected = nullptr;
            signal_selectedChanged().emit();
        }

        _list = list;
        signal_listChanged().emit();
    }
}

template <class T>
void OrderedListController<T>::setSelected(const T* item)
{
    if (_selected != item) {
        if (item != nullptr && _list != nullptr) {
            // check if item is in the list
            // needed to convert const T* to T*

            int index = _list->indexOf(item);
            if (index >= 0) {
                _selected = &_list->at(index);
                signal_selectedChanged().emit();
                return;
            }
        }

        // item is null or not found
        if (_selected != nullptr) {
            _selected = nullptr;
            signal_selectedChanged().emit();
        }
    }
}

template <class T>
void OrderedListController<T>::setSelectedFromPtr(const void* item)
{
    if (_selected != item) {
        if (item != nullptr && _list != nullptr) {
            for (unsigned i = 0; i < _list->size(); i++) {
                if (&_list->at(i) == item) {
                    setSelected(int(i));
                    return;
                }
            }
        }

        // none found
        setSelected(nullptr);
    }
}

template <class T>
void OrderedListController<T>::setSelected(int index)
{
    if (_list != nullptr) {
        T* item = nullptr;

        if (index >= 0 && (unsigned)index < _list->size()) {
            item = &_list->at(index);
        }

        if (_selected != item) {
            _selected = item;
            signal_selectedChanged().emit();
        }
    }
}

template <class T>
void OrderedListController<T>::create()
{
    class Action : public Undo::Action {
    public:
        Action() = delete;
        Action(OrderedListController<T>& controller,
               typename T::list_t* _list, T* item)
            : _handler(_list, item, controller)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _handler.remove();
        }

        virtual void redo() override
        {
            _handler.add();
        }

        virtual const std::string& message() const override
        {
            static std::string message = std::string("Create ") + T::TYPE_NAME;
            return message;
        }

    private:
        Private::OrderedListAddRemove<T> _handler;
    };

    if (canCreate()) {
        T* newItem = &(_list->create());

        signal_listDataChanged().emit(_list);
        setSelected(newItem);

        _baseController.undoStack().add_undo(
            std::make_unique<Action>(*this, _list, newItem));
    }
}

template <class T>
void OrderedListController<T>::selected_clone()
{
    class Action : public Undo::Action {
    public:
        Action() = delete;
        Action(OrderedListController<T>& controller,
               typename T::list_t* _list, T* item)
            : _handler(_list, item, controller)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _handler.remove();
        }

        virtual void redo() override
        {
            _handler.add();
        }

        virtual const std::string& message() const override
        {
            static std::string message = std::string("Clone ") + T::TYPE_NAME;
            return message;
        }

    private:
        Private::OrderedListAddRemove<T> _handler;
    };

    if (canCloneSelected()) {
        T* newItem = &(_list->clone(*_selected));

        signal_listDataChanged().emit(_list);
        setSelected(newItem);

        _baseController.undoStack().add_undo(
            std::make_unique<Action>(*this, _list, newItem));
    }
}

template <class T>
void OrderedListController<T>::selected_remove()
{
    class Action : public Undo::Action {
    public:
        Action() = delete;
        Action(OrderedListController<T>& controller,
               typename T::list_t* _list, T* item)
            : _handler(_list, item, controller)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _handler.add();
        }

        virtual void redo() override
        {
            _handler.remove();
        }

        virtual const std::string& message() const override
        {
            static std::string message = std::string("Remove ") + T::TYPE_NAME;
            return message;
        }

    private:
        Private::OrderedListAddRemove<T> _handler;
    };

    if (canRemoveSelected()) {
        if (_list->contains(_selected)) {
            auto a = std::make_unique<Action>(*this, _list, _selected);

            setSelected(nullptr);
            a->redo();

            _baseController.undoStack().add_undo(std::move(a));
        }
    }
}

template <class T>
void OrderedListController<T>::selected_moveUp()
{
    class Action : public Undo::Action {
    public:
        Action() = delete;
        Action(OrderedListController<T>& controller,
               typename T::list_t* list, T* item)
            : _list(list)
            , _item(item)
            , _controller(controller)
        {
        }
        virtual ~Action() override = default;

        virtual void undo() override
        {
            _list->moveDown(_item);
            _controller.signal_listDataChanged().emit(_list);
        }

        virtual void redo() override
        {
            _list->moveUp(_item);
            _controller.signal_listDataChanged().emit(_list);
        }

        virtual const std::string& message() const override
        {
            static std::string message = std::string("Move ") + T::TYPE_NAME + " Up";
            return message;
        }

    private:
        typename T::list_t* _list;
        T* _item;
        OrderedListController<T>& _controller;
    };

    if (canMoveSelectedUp()) {
        bool r = _list->moveUp(_selected);

        if (r) {
            signal_listDataChanged().emit(_list);

            _baseController.undoStack().add_undo(
                std::make_unique<Action>(*this, _list, _selected));
        }
    }
}

template <class T>
void OrderedListController<T>::selected_moveDown()
{
    class Action : public Undo::Action {
    public:
        Action() = delete;
        Action(OrderedListController<T>& controller,
               typename T::list_t* list, T* item)
            : _list(list)
            , _item(item)
            , _controller(controller)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _list->moveUp(_item);
            _controller.signal_listDataChanged().emit(_list);
        }

        virtual void redo() override
        {
            _list->moveDown(_item);
            _controller.signal_listDataChanged().emit(_list);
        }

        virtual const std::string& message() const override
        {
            static std::string message = std::string("Move ") + T::TYPE_NAME + " Down";
            return message;
        }

    private:
        typename T::list_t* _list;
        T* _item;
        OrderedListController<T>& _controller;
    };

    if (canMoveSelectedDown()) {
        bool r = _list->moveDown(_selected);

        if (r) {
            signal_listDataChanged().emit(_list);

            _baseController.undoStack().add_undo(
                std::make_unique<Action>(*this, _list, _selected));
        }
    }
}
}
}
