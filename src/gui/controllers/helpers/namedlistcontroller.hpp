#pragma once

#include "namedlistcontroller.h"

#include "gui/controllers/undo/undostack.h"
#include "models/common/orderedlist.h"

#include <cassert>
#include <sigc++/signal.h>

namespace UnTech {
namespace Controller {

namespace Private {
// This class is tightly coupled with the `UnTech::orderedlist` template.
template <class T>
class NamedListAddRemove {
public:
    NamedListAddRemove(typename T::list_t* list, const std::string& name,
                       NamedListController<T>& controller)
        : _list(list)
        , _name(name)
        , _controller(controller)
    {
    }

    NamedListAddRemove(const NamedListAddRemove&) = delete;
    ~NamedListAddRemove() = default;

    void add()
    {
        _list->insertInto(std::move(_item), _name);
        _controller.signal_listDataChanged().emit(_list);
    }

    void remove()
    {
        _item = _list->removeFrom(_name);

        // prevent access after item is removed
        if (_item.get() == _controller.selected()) {
            _controller.setSelected(nullptr);
        }
        _controller.signal_listDataChanged().emit(_list);
    }

private:
    typename T::list_t* _list;
    const std::string _name;
    std::unique_ptr<T> _item;
    NamedListController<T>& _controller;
};
}

template <class T>
NamedListController<T>::NamedListController(BaseController& baseController)
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
void NamedListController<T>::setList(typename T::list_t* list)
{
    if (list == nullptr) {
        setSelected(nullptr);
    }
    if (_list != list) {
        _list = list;
        signal_listChanged().emit();
    }
}

template <class T>
void NamedListController<T>::setSelected(const T* item)
{
    if (_selected != item) {
        if (item != nullptr && _list != nullptr) {
            // check if item is in the list
            // needed to convert const T* to T*

            auto n = _list->getName(item);

            if (n) {
                _selected = &_list->at(n.value());
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
std::string NamedListController<T>::selectedName() const
{
    if (_list == nullptr || _selected == nullptr) {
        return "";
    }
    return _list->getName(_selected).value();
}

template <class T>
void NamedListController<T>::create(const std::string& name)
{
    class Action : public Undo::Action {
    public:
        Action() = delete;
        Action(NamedListController<T>& controller,
               typename T::list_t* _list, const std::string& name)
            : _handler(_list, name, controller)
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
        Private::NamedListAddRemove<T> _handler;
    };

    if (_list) {
        T* newItem = _list->create(name);

        if (newItem != nullptr) {
            signal_listDataChanged().emit(_list);
            setSelected(newItem);

            _baseController.undoStack().add_undo(
                std::make_unique<Action>(*this, _list, name));
        }
    }
}

template <class T>
void NamedListController<T>::selected_clone(const std::string& newName)
{
    class Action : public Undo::Action {
    public:
        Action() = delete;
        Action(NamedListController<T>& controller,
               typename T::list_t* _list, const std::string& newName)
            : _handler(_list, newName, controller)
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
        Private::NamedListAddRemove<T> _handler;
    };

    if (_selected && _list) {
        T* newItem = _list->clone(*_selected, newName);

        if (newItem != nullptr) {
            signal_listDataChanged().emit(_list);
            setSelected(newItem);

            _baseController.undoStack().add_undo(
                std::make_unique<Action>(*this, _list, newName));
        }
    }
}

template <class T>
void NamedListController<T>::selected_remove()
{
    class Action : public Undo::Action {
    public:
        Action() = delete;
        Action(NamedListController<T>& controller,
               typename T::list_t* _list, const std::string& name)
            : _handler(_list, name, controller)
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
        Private::NamedListAddRemove<T> _handler;
    };

    if (_selected && _list) {
        const auto name = _list->getName(_selected);

        if (name) {
            auto a = std::make_unique<Action>(*this, _list, name.value());

            setSelected(nullptr);
            a->redo();

            _baseController.undoStack().add_undo(std::move(a));
        }
    }
}

template <class T>
void NamedListController<T>::selected_rename(const std::string& newName)
{
    class Action : public Undo::Action {
    public:
        Action() = delete;
        Action(NamedListController<T>& controller,
               typename T::list_t* list, T* item,
               const std::string& oldName, const std::string& newName)
            : _list(list)
            , _item(item)
            , _oldName(oldName)
            , _newName(newName)
            , _controller(controller)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _list->changeName(_item, _oldName);
            _controller.signal_itemRenamed().emit(_item);
        }

        virtual void redo() override
        {
            _list->changeName(_item, _newName);
            _controller.signal_itemRenamed().emit(_item);
        }

        virtual const std::string& message() const override
        {
            static std::string message = std::string("Rename ") + T::TYPE_NAME;
            return message;
        }

    private:
        typename T::list_t* _list;
        T* _item;
        const std::string _oldName;
        const std::string _newName;
        NamedListController<T>& _controller;
    };

    if (_selected && _list) {
        const auto oldName = _list->getName(_selected);

        if (oldName) {
            bool r = _list->changeName(_selected, newName);

            if (r) {
                signal_listDataChanged().emit(_list);

                _baseController.undoStack().add_undo(
                    std::make_unique<Action>(*this, _list, _selected,
                                             oldName.value(), newName));
            }
        }
    }
}
}
}
