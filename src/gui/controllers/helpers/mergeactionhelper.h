#pragma once

#include "gui/controllers/basecontroller.h"
#include "gui/controllers/undo/undostack.h"
#include <memory>
#include <stdexcept>

// ::ANNOY cannot template a method name::

#define CREATE_MERGE_ACTION(ControllerCls, name,                   \
                            ItemCls, type, getter, setter,         \
                            signal, actionText)                    \
                                                                   \
    void ControllerCls::name(const type& value)                    \
    {                                                              \
        typedef UnTech::Controller::Undo::MergeAction MergeAction; \
        class Action : public MergeAction {                        \
        public:                                                    \
            Action() = delete;                                     \
            Action(ControllerCls& controller,                      \
                   ItemCls* item,                                  \
                   const type& oldValue, const type& newValue)     \
                : _controller(controller)                          \
                , _item(item)                                      \
                , _oldValue(oldValue)                              \
                , _newValue(newValue)                              \
            {                                                      \
            }                                                      \
                                                                   \
            virtual ~Action() override = default;                  \
                                                                   \
            virtual void undo() override                           \
            {                                                      \
                _item->setter(_oldValue);                          \
                _controller.signal().emit(_item);                  \
            }                                                      \
                                                                   \
            virtual void redo() override                           \
            {                                                      \
                _item->setter(_newValue);                          \
                _controller.signal().emit(_item);                  \
            }                                                      \
                                                                   \
            virtual bool mergeWith(MergeAction* o) override        \
            {                                                      \
                Action* other = dynamic_cast<Action*>(o);          \
                                                                   \
                if (other != nullptr) {                            \
                    if (this->_item == other->_item                \
                        && this->_newValue == other->_oldValue) {  \
                                                                   \
                        this->_newValue = other->_newValue;        \
                        return true;                               \
                    }                                              \
                }                                                  \
                                                                   \
                return false;                                      \
            }                                                      \
                                                                   \
            virtual const std::string& message() const override    \
            {                                                      \
                const static std::string message = actionText;     \
                return message;                                    \
            }                                                      \
                                                                   \
        private:                                                   \
            ControllerCls& _controller;                            \
            ItemCls* _item;                                        \
            const type _oldValue;                                  \
            type _newValue;                                        \
        };                                                         \
                                                                   \
        ItemCls* item = this->selected_editable();                 \
        if (item) {                                                \
            type oldValue = item->getter();                        \
                                                                   \
            item->setter(value);                                   \
            signal().emit(item);                                   \
                                                                   \
            type newValue = item->getter();                        \
                                                                   \
            if (oldValue != newValue) {                            \
                this->baseController().undoStack().add_undoMerge(  \
                    std::make_unique<Action>(                      \
                        *this, item, oldValue, newValue));         \
            }                                                      \
        }                                                          \
    }

#define CREATE_MERGE_ACTION2(ControllerCls, name,                  \
                             ItemCls, type, getter, setter,        \
                             signal1, signal2, actionText)         \
                                                                   \
    void ControllerCls::name(const type& value)                    \
    {                                                              \
        typedef UnTech::Controller::Undo::MergeAction MergeAction; \
        class Action : public MergeAction {                        \
        public:                                                    \
            Action() = delete;                                     \
            Action(ControllerCls& controller,                      \
                   ItemCls* item,                                  \
                   const type& oldValue, const type& newValue)     \
                : _controller(controller)                          \
                , _item(item)                                      \
                , _oldValue(oldValue)                              \
                , _newValue(newValue)                              \
            {                                                      \
            }                                                      \
                                                                   \
            virtual ~Action() override = default;                  \
                                                                   \
            virtual void undo() override                           \
            {                                                      \
                _item->setter(_oldValue);                          \
                _controller.signal1().emit(_item);                 \
                _controller.signal2().emit(_item);                 \
            }                                                      \
                                                                   \
            virtual void redo() override                           \
            {                                                      \
                _item->setter(_newValue);                          \
                _controller.signal1().emit(_item);                 \
                _controller.signal2().emit(_item);                 \
            }                                                      \
                                                                   \
            virtual bool mergeWith(MergeAction* o) override        \
            {                                                      \
                Action* other = dynamic_cast<Action*>(o);          \
                                                                   \
                if (other != nullptr) {                            \
                    if (this->_item == other->_item                \
                        && this->_newValue == other->_oldValue) {  \
                                                                   \
                        this->_newValue = other->_newValue;        \
                        return true;                               \
                    }                                              \
                }                                                  \
                                                                   \
                return false;                                      \
            }                                                      \
                                                                   \
            virtual const std::string& message() const override    \
            {                                                      \
                const static std::string message = actionText;     \
                return message;                                    \
            }                                                      \
                                                                   \
        private:                                                   \
            ControllerCls& _controller;                            \
            ItemCls* _item;                                        \
            const type _oldValue;                                  \
            type _newValue;                                        \
        };                                                         \
                                                                   \
        ItemCls* item = this->selected_editable();                 \
        if (item) {                                                \
            type oldValue = item->getter();                        \
                                                                   \
            item->setter(value);                                   \
            signal1().emit(item);                                  \
            signal2().emit(item);                                  \
                                                                   \
            type newValue = item->getter();                        \
                                                                   \
            if (oldValue != newValue) {                            \
                this->baseController().undoStack().add_undoMerge(  \
                    std::make_unique<Action>(                      \
                        *this, item, oldValue, newValue));         \
            }                                                      \
        }                                                          \
    }

#define CREATE_MERGE_INDEXED_ACTION(ControllerCls, name,           \
                                    ItemCls, type, accessor,       \
                                    signal, actionText)            \
                                                                   \
    void ControllerCls::name(size_t index, const type& newValue)   \
    {                                                              \
        typedef UnTech::Controller::Undo::MergeAction MergeAction; \
        class Action : public MergeAction {                        \
        public:                                                    \
            Action() = delete;                                     \
            Action(ControllerCls& controller,                      \
                   ItemCls* item, size_t index,                    \
                   const type& oldValue, const type& newValue)     \
                : _controller(controller)                          \
                , _item(item)                                      \
                , _index(index)                                    \
                , _oldValue(oldValue)                              \
                , _newValue(newValue)                              \
            {                                                      \
            }                                                      \
                                                                   \
            virtual ~Action() override = default;                  \
                                                                   \
            virtual void undo() override                           \
            {                                                      \
                _item->accessor(_index) = _oldValue;               \
                _controller.signal().emit(_item);                  \
            }                                                      \
                                                                   \
            virtual void redo() override                           \
            {                                                      \
                _item->accessor(_index) = _newValue;               \
                _controller.signal().emit(_item);                  \
            }                                                      \
                                                                   \
            virtual bool mergeWith(MergeAction* o) override        \
            {                                                      \
                Action* other = dynamic_cast<Action*>(o);          \
                                                                   \
                if (other != nullptr) {                            \
                    if (this->_item == other->_item                \
                        && this->_index == other->_index           \
                        && this->_newValue == other->_oldValue) {  \
                                                                   \
                        this->_newValue = other->_newValue;        \
                        return true;                               \
                    }                                              \
                }                                                  \
                                                                   \
                return false;                                      \
            }                                                      \
                                                                   \
            virtual const std::string& message() const override    \
            {                                                      \
                const static std::string message = actionText;     \
                return message;                                    \
            }                                                      \
                                                                   \
        private:                                                   \
            ControllerCls& _controller;                            \
            ItemCls* _item;                                        \
            const size_t _index;                                   \
            const type _oldValue;                                  \
            type _newValue;                                        \
        };                                                         \
                                                                   \
        ItemCls* item = this->selected_editable();                 \
        if (item) {                                                \
            type oldValue = item->accessor(index);                 \
                                                                   \
            if (oldValue != newValue) {                            \
                item->accessor(index) = newValue;                  \
                signal().emit(item);                               \
                                                                   \
                this->baseController().undoStack().add_undoMerge(  \
                    std::make_unique<Action>(                      \
                        *this, item, index, oldValue, newValue));  \
            }                                                      \
        }                                                          \
    }

#define CREATE_MERGE_PARAM_ACTION2(ControllerCls, name,            \
                                   ItemCls, parameter,             \
                                   type, getter, setter,           \
                                   signal1, signal2, actionText)   \
                                                                   \
    void ControllerCls::name(const type& value)                    \
    {                                                              \
        typedef UnTech::Controller::Undo::MergeAction MergeAction; \
        class Action : public MergeAction {                        \
        public:                                                    \
            Action() = delete;                                     \
            Action(ControllerCls& controller,                      \
                   ItemCls* item,                                  \
                   const type& oldValue, const type& newValue)     \
                : _controller(controller)                          \
                , _item(item)                                      \
                , _oldValue(oldValue)                              \
                , _newValue(newValue)                              \
            {                                                      \
            }                                                      \
                                                                   \
            virtual ~Action() override = default;                  \
                                                                   \
            virtual void undo() override                           \
            {                                                      \
                _item->parameter().setter(_oldValue);              \
                _controller.signal1().emit(_item);                 \
                _controller.signal2().emit(_item);                 \
            }                                                      \
                                                                   \
            virtual void redo() override                           \
            {                                                      \
                _item->parameter().setter(_newValue);              \
                _controller.signal1().emit(_item);                 \
                _controller.signal2().emit(_item);                 \
            }                                                      \
                                                                   \
            virtual bool mergeWith(MergeAction* o) override        \
            {                                                      \
                Action* other = dynamic_cast<Action*>(o);          \
                                                                   \
                if (other != nullptr) {                            \
                    if (this->_item == other->_item                \
                        && this->_newValue == other->_oldValue) {  \
                                                                   \
                        this->_newValue = other->_newValue;        \
                        return true;                               \
                    }                                              \
                }                                                  \
                                                                   \
                return false;                                      \
            }                                                      \
                                                                   \
            virtual const std::string& message() const override    \
            {                                                      \
                const static std::string message = actionText;     \
                return message;                                    \
            }                                                      \
                                                                   \
        private:                                                   \
            ControllerCls& _controller;                            \
            ItemCls* _item;                                        \
            const type _oldValue;                                  \
            type _newValue;                                        \
        };                                                         \
                                                                   \
        ItemCls* item = this->selected_editable();                 \
        if (item) {                                                \
            type oldValue = item->parameter().getter();            \
                                                                   \
            item->parameter().setter(value);                       \
            signal1().emit(item);                                  \
            signal2().emit(item);                                  \
                                                                   \
            type newValue = item->parameter().getter();            \
                                                                   \
            if (oldValue != newValue) {                            \
                this->baseController().undoStack().add_undoMerge(  \
                    std::make_unique<Action>(                      \
                        *this, item, oldValue, newValue));         \
            }                                                      \
        }                                                          \
    }
