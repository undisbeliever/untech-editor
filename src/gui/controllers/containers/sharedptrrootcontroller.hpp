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
void SharedPtrRootController<T>::setRoot(std::shared_ptr<T> s)
{
    _root = s;

    _signal_selectedChanged.emit();
    _signal_dataChanged.emit();
    _signal_anyChanged.emit();
}

template <typename T>
void SharedPtrRootController<T>::edit_selected(std::function<void(T&)> const& fun)
{
    if (_root) {
        // ::TODO undo engine::

        fun(*_root);
    }

    _signal_dataChanged.emit();
    _signal_anyChanged.emit();
}
}
}
