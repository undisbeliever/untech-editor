#ifndef _UNTECH_GUI_UNDO_MERGE_ACTIONHELPER_H_
#define _UNTECH_GUI_UNDO_MERGE_ACTIONHELPER_H_

#include "undostack.h"
#include "undodocument.h"

// ::ANNOY cannot template a method name::

#define SIMPLE_UNDO_MERGE_ACTION(name, cls, type, getter, setter,           \
                                 signal, actionText)                        \
                                                                            \
    inline void name(cls* item, const type& value)                          \
    {                                                                       \
        class Action : public ::UnTech::Undo::MergeAction {                 \
        public:                                                             \
            Action() = delete;                                              \
            Action(cls* item,                                               \
                   const type& oldValue, const type& newValue)              \
                : _item(item)                                               \
                , _oldValue(oldValue)                                       \
                , _newValue(newValue)                                       \
            {                                                               \
            }                                                               \
                                                                            \
            virtual ~Action() override = default;                           \
                                                                            \
            virtual void undo() override                                    \
            {                                                               \
                _item->setter(_oldValue);                                   \
                signal.emit(_item);                                         \
            }                                                               \
                                                                            \
            virtual void redo() override                                    \
            {                                                               \
                _item->setter(_newValue);                                   \
                signal.emit(_item);                                         \
            }                                                               \
                                                                            \
            virtual bool mergeWith(::UnTech::Undo::MergeAction* o) override \
            {                                                               \
                Action* other = dynamic_cast<Action*>(o);                   \
                                                                            \
                if (other != nullptr) {                                     \
                    if (this->_item == other->_item                         \
                        && this->_newValue == other->_oldValue) {           \
                                                                            \
                        this->_newValue = other->_newValue;                 \
                        return true;                                        \
                    }                                                       \
                }                                                           \
                                                                            \
                return false;                                               \
            }                                                               \
                                                                            \
            virtual const Glib::ustring& message() const override           \
            {                                                               \
                const static Glib::ustring message = _(actionText);         \
                return message;                                             \
            }                                                               \
                                                                            \
        private:                                                            \
            cls* _item;                                                     \
            const type _oldValue;                                           \
            type _newValue;                                                 \
        };                                                                  \
                                                                            \
        if (item) {                                                         \
            type oldValue = item->getter();                                 \
                                                                            \
            item->setter(value);                                            \
            signal.emit(item);                                              \
                                                                            \
            type newValue = item->getter();                                 \
                                                                            \
            if (oldValue != newValue) {                                     \
                auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(   \
                    &(item->document()));                                   \
                undoDoc->undoStack().add_undoMerge(                         \
                    std::make_unique<Action>(item, oldValue, newValue));    \
            }                                                               \
        }                                                                   \
    }

#define SIMPLE_UNDO_MERGE_ACTION2(name, cls, type, getter, setter,          \
                                  signal1, signal2, actionText)             \
                                                                            \
    inline void name(cls* item, const type& value)                          \
    {                                                                       \
        class Action : public ::UnTech::Undo::MergeAction {                 \
        public:                                                             \
            Action() = delete;                                              \
            Action(cls* item,                                               \
                   const type& oldValue, const type& newValue)              \
                : _item(item)                                               \
                , _oldValue(oldValue)                                       \
                , _newValue(newValue)                                       \
            {                                                               \
            }                                                               \
                                                                            \
            virtual ~Action() override = default;                           \
                                                                            \
            virtual void undo() override                                    \
            {                                                               \
                _item->setter(_oldValue);                                   \
                signal1.emit(_item);                                        \
                signal2.emit(_item);                                        \
            }                                                               \
                                                                            \
            virtual void redo() override                                    \
            {                                                               \
                _item->setter(_newValue);                                   \
                signal1.emit(_item);                                        \
                signal2.emit(_item);                                        \
            }                                                               \
                                                                            \
            virtual bool mergeWith(::UnTech::Undo::MergeAction* o) override \
            {                                                               \
                Action* other = dynamic_cast<Action*>(o);                   \
                                                                            \
                if (other != nullptr) {                                     \
                    if (this->_item == other->_item                         \
                        && this->_newValue == other->_oldValue) {           \
                                                                            \
                        this->_newValue = other->_newValue;                 \
                        return true;                                        \
                    }                                                       \
                }                                                           \
                                                                            \
                return false;                                               \
            }                                                               \
                                                                            \
            virtual const Glib::ustring& message() const override           \
            {                                                               \
                const static Glib::ustring message = _(actionText);         \
                return message;                                             \
            }                                                               \
                                                                            \
        private:                                                            \
            cls* _item;                                                     \
            const type _oldValue;                                           \
            type _newValue;                                                 \
        };                                                                  \
                                                                            \
        if (item) {                                                         \
            type oldValue = item->getter();                                 \
                                                                            \
            item->setter(value);                                            \
            signal1.emit(item);                                             \
            signal2.emit(item);                                             \
                                                                            \
            type newValue = item->getter();                                 \
                                                                            \
            if (oldValue != newValue) {                                     \
                auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(   \
                    &(item->document()));                                   \
                undoDoc->undoStack().add_undoMerge(                         \
                    std::make_unique<Action>(item, oldValue, newValue));    \
            }                                                               \
        }                                                                   \
    }

#define PARAMETER_UNDO_MERGE_ACTION2(name, cls, parameter, type, getter, setter, \
                                     signal1, signal2, actionText)               \
                                                                                 \
    inline void name(cls* item, const type& value)                               \
    {                                                                            \
        class Action : public ::UnTech::Undo::MergeAction {                      \
        public:                                                                  \
            Action() = delete;                                                   \
            Action(cls* item,                                                    \
                   const type& oldValue, const type& newValue)                   \
                : _item(item)                                                    \
                , _oldValue(oldValue)                                            \
                , _newValue(newValue)                                            \
            {                                                                    \
            }                                                                    \
                                                                                 \
            virtual ~Action() override = default;                                \
                                                                                 \
            virtual void undo() override                                         \
            {                                                                    \
                _item->parameter().setter(_oldValue);                            \
                signal1.emit(_item);                                             \
                signal2.emit(_item);                                             \
            }                                                                    \
                                                                                 \
            virtual bool mergeWith(::UnTech::Undo::MergeAction* o) override      \
            {                                                                    \
                Action* other = dynamic_cast<Action*>(o);                        \
                                                                                 \
                if (other != nullptr) {                                          \
                    if (this->_item == other->_item                              \
                        && this->_newValue == other->_oldValue) {                \
                                                                                 \
                        this->_newValue = other->_newValue;                      \
                        return true;                                             \
                    }                                                            \
                }                                                                \
                                                                                 \
                return false;                                                    \
            }                                                                    \
                                                                                 \
            virtual void redo() override                                         \
            {                                                                    \
                _item->parameter().setter(_newValue);                            \
                signal1.emit(_item);                                             \
                signal2.emit(_item);                                             \
            }                                                                    \
                                                                                 \
            virtual const Glib::ustring& message() const override                \
            {                                                                    \
                const static Glib::ustring message = _(actionText);              \
                return message;                                                  \
            }                                                                    \
                                                                                 \
        private:                                                                 \
            cls* _item;                                                          \
            const type _oldValue;                                                \
            type _newValue;                                                      \
        };                                                                       \
                                                                                 \
        if (item) {                                                              \
            type oldValue = item->parameter().getter();                          \
                                                                                 \
            item->parameter().setter(value);                                     \
            signal1.emit(item);                                                  \
            signal2.emit(item);                                                  \
                                                                                 \
            type newValue = item->parameter().getter();                          \
                                                                                 \
            if (oldValue != newValue) {                                          \
                auto undoDoc = dynamic_cast<UnTech::Undo::UndoDocument*>(        \
                    &(item->document()));                                        \
                undoDoc->undoStack().add_undoMerge(                              \
                    std::make_unique<Action>(item, oldValue, newValue));         \
            }                                                                    \
        }                                                                        \
    }

#endif
