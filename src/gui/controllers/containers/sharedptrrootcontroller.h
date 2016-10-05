#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <sigc++/signal.h>

namespace UnTech {
namespace Controller {

template <class T>
class SharedPtrRootController {
    using container_type = std::shared_ptr<T>;
    using value_type = T;

    const static value_type BLANK_T;

public:
    SharedPtrRootController() = default;
    SharedPtrRootController(const SharedPtrRootController&) = delete;
    virtual ~SharedPtrRootController() = default;

    bool hasSelected() const { return _root != nullptr; }

    const value_type& selected() const
    {
        if (_root) {
            return *_root;
        }
        else {
            return BLANK_T;
        }
    }

    auto& signal_anyChanged() { return _signal_anyChanged; }
    auto& signal_dataChanged() { return _signal_dataChanged; }
    auto& signal_selectedChanged() { return _signal_selectedChanged; }

protected:
    container_type getRoot() { return _root; }

    void setRoot(std::shared_ptr<value_type>& s)
    {
        _root = s;

        _signal_selectedChanged.emit();
        _signal_dataChanged.emit();
        _signal_anyChanged.emit();
    }

    // can return NULL is nothing is selected
    value_type* editable_selected() { return _root.get(); }

    void edit_selected(std::function<void(value_type&)> const& fun)
    {
        if (_root) {
            // ::TODO undo engine::

            fun(*_root);
        }

        _signal_dataChanged.emit();
        _signal_anyChanged.emit();
    }

protected:
    container_type _root;

    sigc::signal<void> _signal_anyChanged;
    sigc::signal<void> _signal_dataChanged;
    sigc::signal<void> _signal_selectedChanged;
};

template <typename T>
const T SharedPtrRootController<T>::BLANK_T = T();
}
}
