#ifndef _UNTECH_GUI_UNDO_ORDEREDLISTACTIONS_H_
#define _UNTECH_GUI_UNDO_ORDEREDLISTACTIONS_H_

/*
 * This file is tightly coupled with the `UnTech::orderedlist` template and
 * the `UnTech::Widgets::OrderedListView` widgets.
 *
 * Using a raw pointer here is bad and wrong.
 * The list pointer will still exist in memory because of the way
 * the create/remove actions are handled.
 */

#include "undostack.h"
#include "models/common/orderedlist.h"

#include <cassert>
#include <sigc++/signal.h>

namespace UnTech {
namespace Undo {

namespace Private {

template <class T>
class OrderedListAddRemove {
public:
    OrderedListAddRemove(typename T::list_t* list,
                         const std::shared_ptr<T>& item)
        : _list(list)
        , _item(item)
    {
        _index = _list->indexOf(item);
        assert(_index >= 0);
    }

    void add()
    {
        _list->insertAtIndex(_item, _index);
    }

    void remove()
    {
        _list->remove(_item);
    }

    auto list() const { return _list; }

private:
    typename T::list_t* _list;
    const std::shared_ptr<T> _item;
    int _index;
};
}

template <class T>
inline std::shared_ptr<T> orderedList_create(UnTech::Undo::UndoStack& undoStack,
                                             typename T::list_t* list,
                                             const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
                                             const Glib::ustring& message)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(
            Private::OrderedListAddRemove<T>& handler,
            const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
            const Glib::ustring& message)
            : _handler(handler)
            , _listChangedSignal(listChangedSignal)
            , _message(message)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _handler.remove();
            _listChangedSignal.emit(_handler.list());
        }

        virtual void redo() override
        {
            _handler.add();
            _listChangedSignal.emit(_handler.list());
        }

        virtual const Glib::ustring& message() const override { return _message; }

    private:
        Private::OrderedListAddRemove<T> _handler;
        const typename sigc::signal<void, const typename T::list_t*>& _listChangedSignal;
        const Glib::ustring _message;
    };

    if (list) {
        auto newItem = list->create();

        if (newItem != nullptr) {
            listChangedSignal.emit(list);

            Private::OrderedListAddRemove<T> handler(list, newItem);

            std::unique_ptr<Action> a(new Action(handler, listChangedSignal, message));
            undoStack.add_undo(std::move(a));

            return newItem;
        }
    }

    return nullptr;
}

template <class T>
inline std::shared_ptr<T> orderedList_clone(UnTech::Undo::UndoStack& undoStack,
                                            typename T::list_t* list, std::shared_ptr<T> item,
                                            const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
                                            const Glib::ustring& message)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(
            Private::OrderedListAddRemove<T>& handler,
            const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
            const Glib::ustring& message)
            : _handler(handler)
            , _listChangedSignal(listChangedSignal)
            , _message(message)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _handler.remove();
            _listChangedSignal.emit(_handler.list());
        }

        virtual void redo() override
        {
            _handler.add();
            _listChangedSignal.emit(_handler.list());
        }

        virtual const Glib::ustring& message() const override { return _message; }

    private:
        Private::OrderedListAddRemove<T> _handler;
        const typename sigc::signal<void, const typename T::list_t*>& _listChangedSignal;
        const Glib::ustring _message;
    };

    if (list) {
        auto newItem = list->clone(item);

        if (newItem != nullptr) {
            listChangedSignal.emit(list);

            Private::OrderedListAddRemove<T> handler(list, newItem);

            std::unique_ptr<Action> a(new Action(handler, listChangedSignal, message));
            undoStack.add_undo(std::move(a));

            return newItem;
        }
    }

    return nullptr;
}

template <class T>
inline void orderedList_remove(UnTech::Undo::UndoStack& undoStack,
                               typename T::list_t* list, std::shared_ptr<T> item,
                               const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
                               const Glib::ustring& message)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(
            Private::OrderedListAddRemove<T>& handler,
            const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
            const Glib::ustring& message)
            : _handler(handler)
            , _listChangedSignal(listChangedSignal)
            , _message(message)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _handler.add();
            _listChangedSignal.emit(_handler.list());
        }

        virtual void redo() override
        {
            _handler.remove();
            _listChangedSignal.emit(_handler.list());
        }

        virtual const Glib::ustring& message() const override { return _message; }

    private:
        Private::OrderedListAddRemove<T> _handler;
        const typename sigc::signal<void, const typename T::list_t*>& _listChangedSignal;
        const Glib::ustring _message;
    };

    if (list) {
        if (list->contains(item)) {
            Private::OrderedListAddRemove<T> handler(list, item);
            handler.remove();
            listChangedSignal.emit(list);

            std::unique_ptr<Action> a(new Action(handler, listChangedSignal, message));
            undoStack.add_undo(std::move(a));
        }
    }
}

template <class T>
inline void orderedList_moveUp(UnTech::Undo::UndoStack& undoStack,
                               typename T::list_t* list,
                               const std::shared_ptr<T>& item,
                               const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
                               const Glib::ustring& message)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(
            typename T::list_t* list,
            const std::shared_ptr<T>& item,
            const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
            const Glib::ustring& message)
            : _list(list)
            , _item(item)
            , _listChangedSignal(listChangedSignal)
            , _message(message)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _list->moveDown(_item);
            _listChangedSignal.emit(_list);
        }

        virtual void redo() override
        {
            _list->moveUp(_item);
            _listChangedSignal.emit(_list);
        }

        virtual const Glib::ustring& message() const override { return _message; }

    private:
        typename T::list_t* _list;
        const std::shared_ptr<T>& _item;
        const typename sigc::signal<void, const typename T::list_t*>& _listChangedSignal;
        const Glib::ustring _message;
    };

    if (list) {
        bool r = list->moveUp(item);

        if (r) {
            listChangedSignal.emit(list);

            std::unique_ptr<Action> a(new Action(list, item, listChangedSignal, message));
            undoStack.add_undo(std::move(a));
        }
    }
}

template <class T>
inline void orderedList_moveDown(UnTech::Undo::UndoStack& undoStack,
                                 typename T::list_t* list,
                                 const std::shared_ptr<T>& item,
                                 const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
                                 const Glib::ustring& message)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(
            typename T::list_t* list,
            const std::shared_ptr<T>& item,
            const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
            const Glib::ustring& message)
            : _list(list)
            , _item(item)
            , _listChangedSignal(listChangedSignal)
            , _message(message)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _list->moveUp(_item);
            _listChangedSignal.emit(_list);
        }

        virtual void redo() override
        {
            _list->moveDown(_item);
            _listChangedSignal.emit(_list);
        }

        virtual const Glib::ustring& message() const override { return _message; }

    private:
        typename T::list_t* _list;
        const std::shared_ptr<T>& _item;
        const typename sigc::signal<void, const typename T::list_t*>& _listChangedSignal;
        const Glib::ustring _message;
    };

    if (list) {
        bool r = list->moveDown(item);

        if (r) {
            listChangedSignal.emit(list);

            std::unique_ptr<Action> a(new Action(list, item, listChangedSignal, message));
            undoStack.add_undo(std::move(a));
        }
    }
}
}
}

#endif
