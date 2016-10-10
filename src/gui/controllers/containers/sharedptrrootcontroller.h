#pragma once

#include "gui/controllers/basecontroller.h"
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
    SharedPtrRootController(const SharedPtrRootController&) = delete;
    virtual ~SharedPtrRootController() = default;

    SharedPtrRootController(BaseController& baseController);

    BaseController& baseController() { return _baseController; }
    const BaseController& baseController() const { return _baseController; }

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

    void setRoot(std::shared_ptr<value_type> s);

    // can return NULL is nothing is selected
    value_type* editable_selected() { return _root.get(); }

    void edit_selected(std::function<void(value_type&)> const& fun);

protected:
    BaseController& _baseController;

    container_type _root;

    sigc::signal<void> _signal_anyChanged;
    sigc::signal<void> _signal_dataChanged;
    sigc::signal<void> _signal_selectedChanged;
};
}
}
