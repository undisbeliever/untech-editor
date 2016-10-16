#include "sharedptrrootcontroller.h"
#include "models/common/humantypename.h"

namespace UnTech {
namespace Controller {

template <class T>
class SharedPtrRootController<T>::MementoUndoAction : public Undo::Action {
public:
    MementoUndoAction(const Undo::ActionType* actionType,
                      SharedPtrRootController& controller,
                      const UndoRef& ref,
                      const T& value)
        : Undo::Action(actionType)
        , _controller(controller)
        , _ref(ref)
        , _oldValue(value)
        , _newValue()
    {
    }
    virtual ~MementoUndoAction() override = default;

    virtual void undo() final
    {
        elementFromUndoRef(_ref) = _oldValue;

        _controller._signal_dataChanged.emit();
        _controller._signal_anyChanged.emit();
    }

    virtual void redo() final
    {
        elementFromUndoRef(_ref) = _newValue;

        _controller._signal_dataChanged.emit();
        _controller._signal_anyChanged.emit();
    }

    const UndoRef& undoRef() const { return _ref; }

    void setNewValue(const T& value) { _newValue = value; }

private:
    SharedPtrRootController& _controller;
    UndoRef _ref;
    const T _oldValue;
    T _newValue;
};

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
T& SharedPtrRootController<T>::elementFromUndoRef(const UndoRef& ref)
{
    if (ref) {
        return *ref;
    }
    else {
        throw std::logic_error("Root is empty");
    }
}

template <typename T>
void SharedPtrRootController<T>::setRoot(std::shared_ptr<T> newRoot)
{
    std::shared_ptr<T> oldRoot = _root;

    _root = newRoot;

    _signal_selectedChanged.emit();
    _signal_dataChanged.emit();
    _signal_anyChanged.emit();
}

template <typename T>
template <class UndoActionT>
void SharedPtrRootController<T>::edit_selected(
    const Undo::ActionType* actionType,
    std::function<bool(const T&)> const& validate,
    std::function<void(T&)> const& fun)
{
    if (_root) {
        auto& value = *_root;

        if (validate(value)) {
            auto& undoStack = _baseController.undoStack();

            UndoActionT* prevAction = dynamic_cast<UndoActionT*>(
                undoStack.retrieveMergableAction(actionType));

            const auto& undoRef = undoRefForSelected();

            bool canMerge = prevAction && prevAction->undoRef() == undoRef;

            if (canMerge) {
                fun(value);
                prevAction->setNewValue(value);
            }
            else {
                auto action = std::make_unique<UndoActionT>(
                    actionType, *this, undoRef, value);
                fun(value);

                action->setNewValue(value);
                _baseController.undoStack().add_undo(std::move(action));
            }
        }
    }

    _signal_dataChanged.emit();
    _signal_anyChanged.emit();
}
}
}
