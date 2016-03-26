#ifndef _UNTECH_GUI_UNDO_NAMEDLISTACTIONS_H_
#define _UNTECH_GUI_UNDO_NAMEDLISTACTIONS_H_

/*
 * This file is tightly coupled with the `UnTech::orderedlist` template and
 * the `UnTech::Widgets::NamedListView` widgets.
 *
 * Using a raw pointer here is bad and wrong.
 * The list pointer will still exist in memory because of the way
 * the create/remove actions are handled.
 */

#include "undostack.h"
#include "undodocument.h"
#include "models/common/namedlist.h"

#include <cassert>
#include <sigc++/signal.h>

namespace UnTech {
namespace Undo {

namespace Private {

template <class T>
class NamedListAddRemove {
public:
    NamedListAddRemove(typename T::list_t* list,
                       const std::string& name,
                       const std::shared_ptr<T>& item)
        : _list(list)
        , _name(name)
        , _item(item)
    {
    }

    void add()
    {
        _list->insertInto(_item, _name);
    }

    void remove()
    {
        _list->remove(_item);
    }

    auto list() const { return _list; }

private:
    typename T::list_t* _list;
    const std::string _name;
    const std::shared_ptr<T> _item;
};
}

template <class T>
inline std::shared_ptr<T> namedList_create(typename T::list_t* list,
                                           const std::string name,
                                           const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
                                           const Glib::ustring& message)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(
            Private::NamedListAddRemove<T>& handler,
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
        Private::NamedListAddRemove<T> _handler;
        const typename sigc::signal<void, const typename T::list_t*>& _listChangedSignal;
        const Glib::ustring _message;
    };

    if (list) {
        auto newItem = list->create(name);

        if (newItem != nullptr) {
            listChangedSignal.emit(list);

            Private::NamedListAddRemove<T> handler(list, name, newItem);

            auto a = std::make_unique<Action>(handler, listChangedSignal, message);

            auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(&(newItem->document()));
            undoDoc->undoStack().add_undo(std::move(a));

            return newItem;
        }
    }

    return nullptr;
}

template <class T>
inline std::shared_ptr<T> namedList_clone(typename T::list_t* list, std::shared_ptr<T> item,
                                          const std::string& name,
                                          const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
                                          const Glib::ustring& message)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(
            Private::NamedListAddRemove<T>& handler,
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
        Private::NamedListAddRemove<T> _handler;
        const typename sigc::signal<void, const typename T::list_t*>& _listChangedSignal;
        const Glib::ustring _message;
    };

    if (list) {
        auto newItem = list->clone(item, name);

        if (newItem != nullptr) {
            listChangedSignal.emit(list);

            Private::NamedListAddRemove<T> handler(list, name, newItem);

            auto a = std::make_unique<Action>(handler, listChangedSignal, message);

            auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(&(newItem->document()));
            undoDoc->undoStack().add_undo(std::move(a));

            return newItem;
        }
    }

    return nullptr;
}

template <class T>
inline void namedList_remove(typename T::list_t* list, std::shared_ptr<T> item,
                             const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
                             const Glib::ustring& message)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(
            Private::NamedListAddRemove<T>& handler,
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
        Private::NamedListAddRemove<T> _handler;
        const typename sigc::signal<void, const typename T::list_t*>& _listChangedSignal;
        const Glib::ustring _message;
    };

    if (list) {
        auto name = list->getName(item);

        if (name.second) {
            // item exists
            Private::NamedListAddRemove<T> handler(list, name.first, item);
            handler.remove();
            listChangedSignal.emit(list);

            auto a = std::make_unique<Action>(handler, listChangedSignal, message);

            auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(&(item->document()));
            undoDoc->undoStack().add_undo(std::move(a));
        }
    }
}

template <class T>
inline void namedList_rename(typename T::list_t* list,
                             const std::shared_ptr<T>& item, const std::string& newName,
                             const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
                             const Glib::ustring& message)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(typename T::list_t* list, const std::shared_ptr<T>& item,
               const std::string& oldName, const std::string& newName,
               const typename sigc::signal<void, const typename T::list_t*>& listChangedSignal,
               const Glib::ustring& message)
            : _list(list)
            , _item(item)
            , _oldName(oldName)
            , _newName(newName)
            , _listChangedSignal(listChangedSignal)
            , _message(message)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            _list->changeName(_item, _oldName);
            _listChangedSignal.emit(_list);
        }

        virtual void redo() override
        {
            _list->changeName(_item, _newName);
            _listChangedSignal.emit(_list);
        }

        virtual const Glib::ustring& message() const override { return _message; }

    private:
        typename T::list_t* _list;
        const std::shared_ptr<T>& _item;
        const std::string _oldName;
        const std::string _newName;
        const typename sigc::signal<void, const typename T::list_t*>& _listChangedSignal;
        const Glib::ustring _message;
    };

    if (list) {
        auto oldName = list->getName(item);

        if (oldName.second) {
            bool r = list->changeName(item, newName);

            if (r) {
                listChangedSignal.emit(list);

                auto a = std::make_unique<Action>(list, item, oldName.first, newName,
                                                  listChangedSignal, message);

                auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(&(item->document()));
                undoDoc->undoStack().add_undo(std::move(a));
            }
        }
    }
}
}
}

#endif
