#pragma once

#include "gui/controllers/basecontroller.h"
#include "models/common/orderedlist.h"

namespace UnTech {
namespace Controller {

template <class T>
class SingleItemController {
public:
    SingleItemController(BaseController& baseController)
        : _baseController(baseController)
        , _selected(nullptr)
    {
    }
    SingleItemController(const SingleItemController&) = delete;
    ~SingleItemController() = default;

    BaseController& baseController() { return _baseController; }

    const T* selected() const { return _selected; }

    //sigc::signal<void>&
    auto signal_selectedChanged() { return _signal_selectedChanged; }
    //sigc::signal<void, const T*>&
    auto& signal_dataChanged() { return _signal_dataChanged; }

protected:
    T* selected_editable() { return _selected; }
    void setSelected(T* item)
    {
        if (_selected != item) {
            _selected = item;
            signal_selectedChanged().emit();
        }
    }

private:
    BaseController& _baseController;

    T* _selected;

    sigc::signal<void> _signal_selectedChanged;
    sigc::signal<void, const T*> _signal_dataChanged;
};
}
}
