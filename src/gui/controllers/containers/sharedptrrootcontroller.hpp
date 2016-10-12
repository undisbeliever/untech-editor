#include "sharedptrrootcontroller.h"

namespace UnTech {
namespace Controller {

template <typename T>
const T SharedPtrRootController<T>::BLANK_T = T();

template <typename T>
SharedPtrRootController<T>::SharedPtrRootController(BaseController& baseController)
    : _baseController(baseController)
{
}

template <typename T>
typename SharedPtrRootController<T>::UndoRef SharedPtrRootController<T>::undoRefForSelected() const
{
    if (_root) {
        return _root;
    }
    else {
        throw std::logic_error("No element selected");
    }
}

template <typename T>
T* SharedPtrRootController<T>::elementFromUndoRef(const UndoRef& ref)
{
    return ref.get();
}

template <typename T>
void SharedPtrRootController<T>::setRoot(std::shared_ptr<T> s)
{
    _root = s;

    _signal_selectedChanged.emit();
    _signal_dataChanged.emit();
    _signal_anyChanged.emit();
}

template <typename T>
void SharedPtrRootController<T>::edit_selected(
    std::function<bool(const T&)> const& validate,
    std::function<void(T&)> const& fun)
{
    class Action : public Undo::Action {
    public:
        Action() = delete;
        Action(SharedPtrRootController& controller, const T& value)
            : _controller(controller)
            , _ref(controller.undoRefForSelected())
            , _oldValue(value)
            , _newValue()
        {
        }
        virtual ~Action() override = default;

        virtual void undo() override
        {
            T* element = elementFromUndoRef(_ref);
            if (element) {
                *element = _oldValue;

                _controller._signal_dataChanged.emit();
                _controller._signal_anyChanged.emit();
            }
        }

        virtual void redo() override
        {
            T* element = elementFromUndoRef(_ref);
            if (element) {
                *element = _newValue;

                _controller._signal_dataChanged.emit();
                _controller._signal_anyChanged.emit();
            }
        }

        virtual const std::string& message() const override
        {
            // ::TODO undo message::
            static const std::string s = "edit_selected";
            return s;
        }

        void setNewValue(const T& value) { _newValue = value; }

    private:
        SharedPtrRootController& _controller;
        UndoRef _ref;
        const T _oldValue;
        T _newValue;
    };

    if (_root) {
        auto& value = *_root;

        if (validate(value)) {
            auto action = std::make_unique<Action>(*this, value);

            fun(value);

            action->setNewValue(value);
            _baseController.undoStack().add_undo(std::move(action));
        }
    }

    _signal_dataChanged.emit();
    _signal_anyChanged.emit();
}
}
}
