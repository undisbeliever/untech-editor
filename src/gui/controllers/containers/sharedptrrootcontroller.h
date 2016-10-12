#pragma once

#include "gui/controllers/basecontroller.h"
#include <functional>
#include <memory>
#include <sigc++/signal.h>

namespace UnTech {
namespace Controller {

template <class T>
class SharedPtrRootController {
    template <typename, class, class>
    friend class CappedVectorController;
    template <class, class>
    friend class IdMapController;

public:
    using container_type = std::shared_ptr<T>;
    using value_type = T;
    using element_type = T;

    using UndoRef = std::shared_ptr<T>;

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

    // will only create an UndoAction and call fun if validate returns true.
    void edit_selected(
        std::function<bool(const element_type&)> const& validate,
        std::function<void(element_type&)> const& fun);

    UndoRef undoRefForSelected() const;
    static element_type* elementFromUndoRef(const UndoRef& ref);

protected:
    BaseController& _baseController;

    container_type _root;

    sigc::signal<void> _signal_anyChanged;
    sigc::signal<void> _signal_dataChanged;
    sigc::signal<void> _signal_selectedChanged;
};
}
}
