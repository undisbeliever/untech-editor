#pragma once

#include "gui/controllers/basecontroller.h"
#include "models/common/orderedlist.h"

namespace UnTech {
namespace Controller {

template <class T>
class OrderedListController {
public:
    OrderedListController(BaseController& baseController);
    OrderedListController(const OrderedListController&) = delete;
    ~OrderedListController() = default;

    BaseController& baseController() { return _baseController; }

    const typename T::list_t* list() const { return _list; }
    void setList(typename T::list_t* list);

    const T* selected() const { return _selected; }
    void setSelected(const T* item);
    void setSelected(int index);
    void setSelectedFromPtr(const void* item);

    void create();
    void selected_clone();
    void selected_remove();
    void selected_moveUp();
    void selected_moveDown();

    bool canCreate() const
    {
        return _list;
    }

    bool canCloneSelected() const
    {
        return _list && _selected;
    }

    bool canRemoveSelected() const
    {
        return _list && _selected;
    }

    bool canMoveSelectedUp() const
    {
        return _list && _selected && !_list->isLast(_selected);
    }

    bool canMoveSelectedDown() const
    {
        return _list && _selected && !_list->isFirst(_selected);
    }

    auto& signal_selectedChanged() { return _signal_selectedChanged; }
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

    sigc::signal<void> _signal_listChanged;
    sigc::signal<void, const typename T::list_t*> _signal_listDataChanged;

    sigc::signal<void> _signal_selectedDataChanged;
};
}
}
