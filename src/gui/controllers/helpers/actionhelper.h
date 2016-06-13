#pragma once

#include "gui/controllers/basecontroller.h"
#include "gui/controllers/undo/undostack.h"
#include <memory>
#include <stdexcept>

// ::ANNOY cannot template a method name::

// ::SHOULDO add a Boolean action::
// ::: would need undo and redo messages in UnTech::Controller::Undo::Action ::

// ::TODO change return type of message to std::string::

#define CREATE_SIMPLE_ACTION(ControllerCls, name,                   \
                             ItemCls, type, getter, setter,         \
                             signal, actionText)                    \
                                                                    \
    void ControllerCls::name(const type& value)                     \
    {                                                               \
        class Action : public UnTech::Controller::Undo::Action {    \
        public:                                                     \
            Action() = delete;                                      \
            Action(ControllerCls& controller,                       \
                   ItemCls* item,                                   \
                   const type& oldValue, const type& newValue)      \
                : _controller(controller)                           \
                , _item(item)                                       \
                , _oldValue(oldValue)                               \
                , _newValue(newValue)                               \
            {                                                       \
            }                                                       \
                                                                    \
            virtual ~Action() override = default;                   \
                                                                    \
            virtual void undo() override                            \
            {                                                       \
                _item->setter(_oldValue);                           \
                _controller.signal().emit(_item);                   \
            }                                                       \
                                                                    \
            virtual void redo() override                            \
            {                                                       \
                _item->setter(_newValue);                           \
                _controller.signal().emit(_item);                   \
            }                                                       \
                                                                    \
            virtual const Glib::ustring& message() const override   \
            {                                                       \
                const static Glib::ustring message = _(actionText); \
                return message;                                     \
            }                                                       \
                                                                    \
        private:                                                    \
            ControllerCls& _controller;                             \
            ItemCls* _item;                                         \
            const type _oldValue;                                   \
            const type _newValue;                                   \
        };                                                          \
                                                                    \
        ItemCls* item = this->selected_editable();                  \
        if (item) {                                                 \
            type oldValue = item->getter();                         \
                                                                    \
            item->setter(value);                                    \
            signal().emit(item);                                    \
                                                                    \
            type newValue = item->getter();                         \
                                                                    \
            if (oldValue != newValue) {                             \
                this->baseController().undoStack().add_undo(        \
                    std::make_unique<Action>(                       \
                        *this, item, oldValue, newValue));          \
            }                                                       \
        }                                                           \
    }

#define CREATE_SIMPLE_ACTION2(ControllerCls, name,                  \
                              ItemCls, type, getter, setter,        \
                              signal1, signal2, actionText)         \
                                                                    \
    void ControllerCls::name(const type& value)                     \
    {                                                               \
        class Action : public UnTech::Controller::Undo::Action {    \
        public:                                                     \
            Action() = delete;                                      \
            Action(ControllerCls& controller,                       \
                   ItemCls* item,                                   \
                   const type& oldValue, const type& newValue)      \
                : _controller(controller)                           \
                , _item(item)                                       \
                , _oldValue(oldValue)                               \
                , _newValue(newValue)                               \
            {                                                       \
            }                                                       \
                                                                    \
            virtual ~Action() override = default;                   \
                                                                    \
            virtual void undo() override                            \
            {                                                       \
                _item->setter(_oldValue);                           \
                _controller.signal1().emit(_item);                  \
                _controller.signal2().emit(_item);                  \
            }                                                       \
                                                                    \
            virtual void redo() override                            \
            {                                                       \
                _item->setter(_newValue);                           \
                _controller.signal1().emit(_item);                  \
                _controller.signal2().emit(_item);                  \
            }                                                       \
                                                                    \
            virtual const Glib::ustring& message() const override   \
            {                                                       \
                const static Glib::ustring message = _(actionText); \
                return message;                                     \
            }                                                       \
                                                                    \
        private:                                                    \
            ControllerCls& _controller;                             \
            ItemCls* _item;                                         \
            const type _oldValue;                                   \
            const type _newValue;                                   \
        };                                                          \
                                                                    \
        ItemCls* item = this->selected_editable();                  \
        if (item) {                                                 \
            type oldValue = item->getter();                         \
                                                                    \
            item->setter(value);                                    \
            signal1().emit(item);                                   \
            signal2().emit(item);                                   \
                                                                    \
            type newValue = item->getter();                         \
                                                                    \
            if (oldValue != newValue) {                             \
                this->baseController().undoStack().add_undo(        \
                    std::make_unique<Action>(                       \
                        *this, item, oldValue, newValue));          \
            }                                                       \
        }                                                           \
    }

#define CREATE_DUAL_ACTION(ControllerCls, name, ItemCls,            \
                           type1, getter1, setter1,                 \
                           type2, getter2, setter2,                 \
                           signal, actionText)                      \
                                                                    \
    void ControllerCls::name(const type1& value1,                   \
                             const type2& value2)                   \
    {                                                               \
        class Action : public UnTech::Controller::Undo::Action {    \
        public:                                                     \
            Action() = delete;                                      \
            Action(ControllerCls& controller,                       \
                   ItemCls* item,                                   \
                   const type1& oldValue1, const type1& newValue1,  \
                   const type2& oldValue2, const type2& newValue2)  \
                : _controller(controller)                           \
                , _item(item)                                       \
                , _oldValue1(oldValue1)                             \
                , _newValue1(newValue1)                             \
                , _oldValue2(oldValue2)                             \
                , _newValue2(newValue2)                             \
            {                                                       \
            }                                                       \
                                                                    \
            virtual ~Action() override = default;                   \
                                                                    \
            virtual void undo() override                            \
            {                                                       \
                _item->setter1(_oldValue1);                         \
                _item->setter2(_oldValue2);                         \
                _controller.signal().emit(_item);                   \
            }                                                       \
                                                                    \
            virtual void redo() override                            \
            {                                                       \
                _item->setter1(_newValue1);                         \
                _item->setter2(_newValue2);                         \
                _controller.signal().emit(_item);                   \
            }                                                       \
                                                                    \
            virtual const Glib::ustring& message() const override   \
            {                                                       \
                const static Glib::ustring message = _(actionText); \
                return message;                                     \
            }                                                       \
                                                                    \
        private:                                                    \
            ControllerCls& _controller;                             \
            ItemCls* _item;                                         \
            const type1 _oldValue1;                                 \
            const type1 _newValue1;                                 \
            const type2 _oldValue2;                                 \
            const type2 _newValue2;                                 \
        };                                                          \
                                                                    \
        ItemCls* item = this->selected_editable();                  \
        if (item) {                                                 \
            type1 oldValue1 = item->getter1();                      \
            type2 oldValue2 = item->getter2();                      \
                                                                    \
            item->setter1(value1);                                  \
            item->setter2(value2);                                  \
            signal().emit(item);                                    \
                                                                    \
            type1 newValue1 = item->getter1();                      \
            type2 newValue2 = item->getter2();                      \
                                                                    \
            if (oldValue1 != newValue1 || oldValue2 != newValue2) { \
                this->baseController().undoStack().add_undo(        \
                    std::make_unique<Action>(                       \
                        *this, item, oldValue1, newValue1,          \
                        oldValue2, newValue2));                     \
            }                                                       \
        }                                                           \
    }

#define CREATE_DUAL_ACTION2(ControllerCls, name, ItemCls,           \
                            type1, getter1, setter1,                \
                            type2, getter2, setter2,                \
                            signal1, signal2, actionText)           \
                                                                    \
    void ControllerCls::name(const type1& value1,                   \
                             const type2& value2)                   \
    {                                                               \
        class Action : public UnTech::Controller::Undo::Action {    \
        public:                                                     \
            Action() = delete;                                      \
            Action(ControllerCls& controller,                       \
                   ItemCls* item,                                   \
                   const type1& oldValue1, const type1& newValue1,  \
                   const type2& oldValue2, const type2& newValue2)  \
                : _controller(controller)                           \
                , _item(item)                                       \
                , _oldValue1(oldValue1)                             \
                , _newValue1(newValue1)                             \
                , _oldValue2(oldValue2)                             \
                , _newValue2(newValue2)                             \
            {                                                       \
            }                                                       \
                                                                    \
            virtual ~Action() override = default;                   \
                                                                    \
            virtual void undo() override                            \
            {                                                       \
                _item->setter1(_oldValue1);                         \
                _item->setter2(_oldValue2);                         \
                _controller.signal1().emit(_item);                  \
                _controller.signal2().emit(_item);                  \
            }                                                       \
                                                                    \
            virtual void redo() override                            \
            {                                                       \
                _item->setter1(_newValue1);                         \
                _item->setter2(_newValue2);                         \
                _controller.signal1().emit(_item);                  \
                _controller.signal2().emit(_item);                  \
            }                                                       \
                                                                    \
            virtual const Glib::ustring& message() const override   \
            {                                                       \
                const static Glib::ustring message = _(actionText); \
                return message;                                     \
            }                                                       \
                                                                    \
        private:                                                    \
            ControllerCls& _controller;                             \
            ItemCls* _item;                                         \
            const type1 _oldValue1;                                 \
            const type1 _newValue1;                                 \
            const type2 _oldValue2;                                 \
            const type2 _newValue2;                                 \
        };                                                          \
                                                                    \
        ItemCls* item = this->selected_editable();                  \
        if (item) {                                                 \
            type1 oldValue1 = item->getter1();                      \
            type2 oldValue2 = item->getter2();                      \
                                                                    \
            item->setter1(value1);                                  \
            item->setter2(value2);                                  \
                                                                    \
            this->signal1().emit(item);                             \
            this->signal2().emit(item);                             \
                                                                    \
            type1 newValue1 = item->getter1();                      \
            type2 newValue2 = item->getter2();                      \
                                                                    \
            if (oldValue1 != newValue1 || oldValue2 != newValue2) { \
                this->baseController().undoStack().add_undo(        \
                    std::make_unique<Action>(                       \
                        *this, item, oldValue1, newValue1,          \
                        oldValue2, newValue2));                     \
            }                                                       \
        }                                                           \
    }

#define CREATE_HANDLED_ACTION2(ControllerCls, name,                 \
                               ItemCls, type, getter, setter,       \
                               signal1, signal2,                    \
                               actionText, errorText)               \
                                                                    \
    void ControllerCls::name(const type& value)                     \
    {                                                               \
        class Action : public UnTech::Controller::Undo::Action {    \
        public:                                                     \
            Action() = delete;                                      \
            Action(ControllerCls& controller,                       \
                   ItemCls* item,                                   \
                   const type& oldValue, const type& newValue)      \
                : _controller(controller)                           \
                , _item(item)                                       \
                , _oldValue(oldValue)                               \
                , _newValue(newValue)                               \
            {                                                       \
            }                                                       \
                                                                    \
            virtual ~Action() override = default;                   \
                                                                    \
            virtual void undo() override                            \
            {                                                       \
                try {                                               \
                    _item->setter(_oldValue);                       \
                }                                                   \
                catch (const std::exception& ex) {                  \
                    _controller.baseController()                    \
                        .showError(errorText, ex);                  \
                }                                                   \
                _controller.signal1().emit(_item);                  \
                _controller.signal2().emit(_item);                  \
            }                                                       \
                                                                    \
            virtual void redo() override                            \
            {                                                       \
                try {                                               \
                    _item->setter(_oldValue);                       \
                }                                                   \
                catch (const std::exception& ex) {                  \
                    _controller.baseController()                    \
                        .showError(errorText, ex);                  \
                }                                                   \
                _controller.signal1().emit(_item);                  \
                _controller.signal2().emit(_item);                  \
            }                                                       \
                                                                    \
            virtual const Glib::ustring& message() const override   \
            {                                                       \
                const static Glib::ustring message = _(actionText); \
                return message;                                     \
            }                                                       \
                                                                    \
        private:                                                    \
            ControllerCls& _controller;                             \
            ItemCls* _item;                                         \
            const type _oldValue;                                   \
            const type _newValue;                                   \
        };                                                          \
                                                                    \
        ItemCls* item = this->selected_editable();                  \
        if (item) {                                                 \
            type oldValue = item->getter();                         \
                                                                    \
            try {                                                   \
                item->setter(value);                                \
            }                                                       \
            catch (const std::exception& ex) {                      \
                this->baseController().showError(errorText, ex);    \
            }                                                       \
            signal1().emit(item);                                   \
            signal2().emit(item);                                   \
                                                                    \
            type newValue = item->getter();                         \
                                                                    \
            if (oldValue != newValue) {                             \
                this->baseController().undoStack().add_undo(        \
                    std::make_unique<Action>(                       \
                        *this, item, oldValue, newValue));          \
            }                                                       \
        }                                                           \
    }

#define CREATE_INDEXED_ACTION(ControllerCls, name,                  \
                              ItemCls, type, accessor,              \
                              signal, actionText)                   \
                                                                    \
    void ControllerCls::name(size_t index, const type& newValue)    \
    {                                                               \
        class Action : public UnTech::Controller::Undo::Action {    \
        public:                                                     \
            Action() = delete;                                      \
            Action(ControllerCls& controller,                       \
                   ItemCls* item, size_t index,                     \
                   const type& oldValue, const type& newValue)      \
                : _controller(controller)                           \
                , _item(item)                                       \
                , _index(index)                                     \
                , _oldValue(oldValue)                               \
                , _newValue(newValue)                               \
            {                                                       \
            }                                                       \
                                                                    \
            virtual ~Action() override = default;                   \
                                                                    \
            virtual void undo() override                            \
            {                                                       \
                _item->accessor(_index) = _oldValue;                \
                _controller.signal().emit(_item);                   \
            }                                                       \
                                                                    \
            virtual void redo() override                            \
            {                                                       \
                _item->accessor(_index) = _newValue;                \
                _controller.signal().emit(_item);                   \
            }                                                       \
                                                                    \
            virtual const Glib::ustring& message() const override   \
            {                                                       \
                const static Glib::ustring message = _(actionText); \
                return message;                                     \
            }                                                       \
                                                                    \
        private:                                                    \
            ControllerCls& _controller;                             \
            ItemCls* _item;                                         \
            const size_t _index;                                    \
            const type _oldValue;                                   \
            const type _newValue;                                   \
        };                                                          \
                                                                    \
        ItemCls* item = this->selected_editable();                  \
        if (item) {                                                 \
            type oldValue = item->accessor(index);                  \
                                                                    \
            if (oldValue != newValue) {                             \
                item->accessor(index) = newValue;                   \
                signal().emit(item);                                \
                                                                    \
                this->baseController().undoStack().add_undo(        \
                    std::make_unique<Action>(                       \
                        *this, item, index, oldValue, newValue));   \
            }                                                       \
        }                                                           \
    }

#define CREATE_PARAMETER_ACTION2(ControllerCls, name,               \
                                 ItemCls, parameter,                \
                                 type, getter, setter,              \
                                 signal1, signal2, actionText)      \
                                                                    \
    void ControllerCls::name(const type& value)                     \
    {                                                               \
        class Action : public UnTech::Controller::Undo::Action {    \
        public:                                                     \
            Action() = delete;                                      \
            Action(ControllerCls& controller,                       \
                   ItemCls* item,                                   \
                   const type& oldValue, const type& newValue)      \
                : _controller(controller)                           \
                , _item(item)                                       \
                , _oldValue(oldValue)                               \
                , _newValue(newValue)                               \
            {                                                       \
            }                                                       \
                                                                    \
            virtual ~Action() override = default;                   \
                                                                    \
            virtual void undo() override                            \
            {                                                       \
                _item->parameter().setter(_oldValue);               \
                _controller.signal1().emit(_item);                  \
                _controller.signal2().emit(_item);                  \
            }                                                       \
                                                                    \
            virtual void redo() override                            \
            {                                                       \
                _item->parameter().setter(_newValue);               \
                _controller.signal1().emit(_item);                  \
                _controller.signal2().emit(_item);                  \
            }                                                       \
                                                                    \
            virtual const Glib::ustring& message() const override   \
            {                                                       \
                const static Glib::ustring message = _(actionText); \
                return message;                                     \
            }                                                       \
                                                                    \
        private:                                                    \
            ControllerCls& _controller;                             \
            ItemCls* _item;                                         \
            const type _oldValue;                                   \
            const type _newValue;                                   \
        };                                                          \
                                                                    \
        ItemCls* item = this->selected_editable();                  \
        if (item) {                                                 \
            type oldValue = item->parameter().getter();             \
                                                                    \
            item->parameter().setter(value);                        \
            signal1().emit(item);                                   \
            signal2().emit(item);                                   \
                                                                    \
            type newValue = item->parameter().getter();             \
                                                                    \
            if (oldValue != newValue) {                             \
                this->baseController().undoStack().add_undo(        \
                    std::make_unique<Action>(                       \
                        *this, item, oldValue, newValue));          \
            }                                                       \
        }                                                           \
    }
