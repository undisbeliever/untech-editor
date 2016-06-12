#pragma once

#include "gui/controllers/basecontroller.h"
#include "models/common/namedlist.h"

namespace UnTech {
namespace Controller {

template <class T>
class NamedListController {
public:
    NamedListController(BaseController& baseController);
    NamedListController(const NamedListController&) = delete;
    ~NamedListController() = default;

    BaseController& baseController() { return _baseController; }

    const typename T::list_t* list() const { return _list; }
    void setList(typename T::list_t* list);

    const T* selected() const { return _selected; }
    std::string selectedName() const;
    void setSelected(const T* item);

    void create(const std::string& name);
    void selected_clone(const std::string& newName);
    void selected_remove();
    void selected_rename(const std::string& newName);

    auto& signal_selectedChanged() { return _signal_selectedChanged; }
    auto& signal_itemRenamed() { return _signal_itemRenamed; }
    auto& signal_dataChanged() { return _signal_dataChanged; }

    auto& signal_listChanged() { return _signal_listChanged; }
    auto& signal_listDataChanged() { return _signal_listDataChanged; }

    auto& signal_selectedDataChanged() { return _signal_selectedDataChanged; }

protected:
    T* selected_editable() { return _selected; }

private:
    BaseController& _baseController;

    typename T::list_t* _list;
    T* _selected;

    sigc::signal<void> _signal_selectedChanged;
    sigc::signal<void, const T*> _signal_dataChanged;
    sigc::signal<void, const T*> _signal_itemRenamed;

    sigc::signal<void> _signal_listChanged;
    sigc::signal<void, const typename T::list_t*> _signal_listDataChanged;

    sigc::signal<void> _signal_selectedDataChanged;
};
}
}
