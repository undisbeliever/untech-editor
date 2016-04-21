#ifndef _UNTECH_GUI_UNDO_ORDEREDLISTACTIONS_H_
#define _UNTECH_GUI_UNDO_ORDEREDLISTACTIONS_H_

/*
 * This file is tightly coupled with the `UnTech::orderedlist` template and
 * the `UnTech::Widgets::OrderedListView` widgets.
 */

#include "undostack.h"
#include "undodocument.h"
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
                         T* item)
        : _list(list)
        , _item(nullptr)
    {
        int index = _list->indexOf(item);
        assert(index >= 0);
        _index = (size_t)index;
    }

    void add()
    {
        _list->insertAtIndex(std::move(_item), _index);
    }

    void remove()
    {
        _item = _list->removeFromList(_index);
    }

    typename T::list_t* list() const { return _list; }

private:
    typename T::list_t* _list;
    std::unique_ptr<T> _item;
    size_t _index;
};
}

template <class T>
inline T* orderedList_create(typename T::list_t* list,
                             const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
                             const Glib::ustring& message)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(
            typename T::list_t* list, T* item,
            const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
            const Glib::ustring& message)
            : _handler(list, item)
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

    T* newItem = &(list->create());

    listChangedSignal.emit(list);

    auto a = std::make_unique<Action>(list, newItem, listChangedSignal, message);

    auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(&(newItem->document()));
    undoDoc->undoStack().add_undo(std::move(a));

    return newItem;
}

template <class T>
inline T* orderedList_clone(typename T::list_t* list, T* item,
                            const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
                            const Glib::ustring& message)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(
            typename T::list_t* list, T* item,
            const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
            const Glib::ustring& message)
            : _handler(list, item)
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

    if (item) {
        T* newItem = &(list->clone(*item));

        listChangedSignal.emit(list);

        auto a = std::make_unique<Action>(list, newItem, listChangedSignal, message);

        auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(&(newItem->document()));
        undoDoc->undoStack().add_undo(std::move(a));

        return newItem;
    }
    else {
        return nullptr;
    }
}

template <class T>
inline void orderedList_remove(typename T::list_t* list, T* item,
                               const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
                               const Glib::ustring& message)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(
            typename T::list_t* list, T* item,
            const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
            const Glib::ustring& message)
            : _handler(list, item)
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

    if (list->contains(item)) {
        auto a = std::make_unique<Action>(list, item, listChangedSignal, message);

        a->redo();

        auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(&(item->document()));
        undoDoc->undoStack().add_undo(std::move(a));
    }
}

template <class T>
inline void orderedList_moveUp(typename T::list_t* list,
                               T* item,
                               const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
                               const Glib::ustring& message)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(
            typename T::list_t* list,
            T* item,
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
        T* _item;
        const typename sigc::signal<void, const typename T::list_t*>& _listChangedSignal;
        const Glib::ustring _message;
    };

    if (list && item) {
        bool r = list->moveUp(item);

        if (r) {
            listChangedSignal.emit(list);

            auto a = std::make_unique<Action>(list, item, listChangedSignal, message);

            auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(&(item->document()));
            undoDoc->undoStack().add_undo(std::move(a));
        }
    }
}

template <class T>
inline void orderedList_moveDown(typename T::list_t* list,
                                 T* item,
                                 const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
                                 const Glib::ustring& message)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(
            typename T::list_t* list,
            T* item,
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
        T* _item;
        const typename sigc::signal<void, const typename T::list_t*>& _listChangedSignal;
        const Glib::ustring _message;
    };

    if (list && item) {
        bool r = list->moveDown(item);

        if (r) {
            listChangedSignal.emit(list);

            auto a = std::make_unique<Action>(list, item, listChangedSignal, message);

            auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(&(item->document()));
            undoDoc->undoStack().add_undo(std::move(a));
        }
    }
}
}
}

#endif
