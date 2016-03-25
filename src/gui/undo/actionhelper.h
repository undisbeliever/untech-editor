#ifndef _UNTECH_GUI_UNDO_ACTIONHELPER_H_
#define _UNTECH_GUI_UNDO_ACTIONHELPER_H_

#include "undostack.h"

// ::TODO get undo stack from the item.;:
// ::: I suggest adding it to a project class ::

// ::ANNOY cannot template a method name::

#define SIMPLE_UNDO_ACTION(name, cls, type, getter, setter,                  \
                           signal, actionText)                               \
                                                                             \
    inline void name(UnTech::Undo::UndoStack& undoStack,                     \
                     const std::shared_ptr<cls>& item, const type& value)    \
    {                                                                        \
        class Action : public ::UnTech::Undo::Action {                       \
        public:                                                              \
            Action() = delete;                                               \
            Action(const std::shared_ptr<cls>& item,                         \
                   const type& oldValue, const type& newValue)               \
                : _item(item)                                                \
                , _oldValue(oldValue)                                        \
                , _newValue(newValue)                                        \
            {                                                                \
            }                                                                \
                                                                             \
            virtual ~Action() override = default;                            \
                                                                             \
            virtual void undo() override                                     \
            {                                                                \
                _item->setter(_oldValue);                                    \
                signal.emit(_item);                                          \
            }                                                                \
                                                                             \
            virtual void redo() override                                     \
            {                                                                \
                _item->setter(_newValue);                                    \
                signal.emit(_item);                                          \
            }                                                                \
                                                                             \
            virtual const Glib::ustring& message() const override            \
            {                                                                \
                const static Glib::ustring message = _(actionText);          \
                return message;                                              \
            }                                                                \
                                                                             \
        private:                                                             \
            const std::shared_ptr<cls> _item;                                \
            const type _oldValue;                                            \
            const type _newValue;                                            \
        };                                                                   \
                                                                             \
        type oldValue = item->getter();                                      \
                                                                             \
        item->setter(value);                                                 \
        signal.emit(item);                                                   \
                                                                             \
        type newValue = item->getter();                                      \
                                                                             \
        if (oldValue != newValue) {                                          \
            std::unique_ptr<Action> a(new Action(item, oldValue, newValue)); \
            undoStack.add_undo(std::move(a));                                \
        }                                                                    \
    }

#define SIMPLE_UNDO_ACTION2(name, cls, type, getter, setter,                 \
                            signal1, signal2, actionText)                    \
                                                                             \
    inline void name(UnTech::Undo::UndoStack& undoStack,                     \
                     const std::shared_ptr<cls>& item, const type& value)    \
    {                                                                        \
        class Action : public ::UnTech::Undo::Action {                       \
        public:                                                              \
            Action() = delete;                                               \
            Action(const std::shared_ptr<cls>& item,                         \
                   const type& oldValue, const type& newValue)               \
                : _item(item)                                                \
                , _oldValue(oldValue)                                        \
                , _newValue(newValue)                                        \
            {                                                                \
            }                                                                \
                                                                             \
            virtual ~Action() override = default;                            \
                                                                             \
            virtual void undo() override                                     \
            {                                                                \
                _item->setter(_oldValue);                                    \
                signal1.emit(_item);                                         \
                signal2.emit(_item);                                         \
            }                                                                \
                                                                             \
            virtual void redo() override                                     \
            {                                                                \
                _item->setter(_newValue);                                    \
                signal1.emit(_item);                                         \
                signal2.emit(_item);                                         \
            }                                                                \
                                                                             \
            virtual const Glib::ustring& message() const override            \
            {                                                                \
                const static Glib::ustring message = _(actionText);          \
                return message;                                              \
            }                                                                \
                                                                             \
        private:                                                             \
            const std::shared_ptr<cls> _item;                                \
            const type _oldValue;                                            \
            const type _newValue;                                            \
        };                                                                   \
                                                                             \
        type oldValue = item->getter();                                      \
                                                                             \
        item->setter(value);                                                 \
        signal1.emit(item);                                                  \
        signal2.emit(item);                                                  \
                                                                             \
        type newValue = item->getter();                                      \
                                                                             \
        if (oldValue != newValue) {                                          \
            std::unique_ptr<Action> a(new Action(item, oldValue, newValue)); \
            undoStack.add_undo(std::move(a));                                \
        }                                                                    \
    }

#define PARAMETER_UNDO_ACTION2(name, cls, parameter, type, getter, setter,   \
                               signal1, signal2, actionText)                 \
                                                                             \
    inline void name(UnTech::Undo::UndoStack& undoStack,                     \
                     const std::shared_ptr<cls>& item, const type& value)    \
    {                                                                        \
        class Action : public ::UnTech::Undo::Action {                       \
        public:                                                              \
            Action() = delete;                                               \
            Action(const std::shared_ptr<cls>& item,                         \
                   const type& oldValue, const type& newValue)               \
                : _item(item)                                                \
                , _oldValue(oldValue)                                        \
                , _newValue(newValue)                                        \
            {                                                                \
            }                                                                \
                                                                             \
            virtual ~Action() override = default;                            \
                                                                             \
            virtual void undo() override                                     \
            {                                                                \
                _item->parameter().setter(_oldValue);                        \
                signal1.emit(_item);                                         \
                signal2.emit(_item);                                         \
            }                                                                \
                                                                             \
            virtual void redo() override                                     \
            {                                                                \
                _item->parameter().setter(_newValue);                        \
                signal1.emit(_item);                                         \
                signal2.emit(_item);                                         \
            }                                                                \
                                                                             \
            virtual const Glib::ustring& message() const override            \
            {                                                                \
                const static Glib::ustring message = _(actionText);          \
                return message;                                              \
            }                                                                \
                                                                             \
        private:                                                             \
            const std::shared_ptr<cls> _item;                                \
            const type _oldValue;                                            \
            const type _newValue;                                            \
        };                                                                   \
                                                                             \
        type oldValue = item->parameter().getter();                          \
                                                                             \
        item->parameter().setter(value);                                     \
        signal1.emit(item);                                                  \
        signal2.emit(item);                                                  \
                                                                             \
        type newValue = item->parameter().getter();                          \
                                                                             \
        if (oldValue != newValue) {                                          \
            std::unique_ptr<Action> a(new Action(item, oldValue, newValue)); \
            undoStack.add_undo(std::move(a));                                \
        }                                                                    \
    }

#endif
