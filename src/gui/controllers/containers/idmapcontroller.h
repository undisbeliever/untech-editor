#pragma once

#include "models/common/idmap.h"
#include <functional>
#include <sigc++/signal.h>

namespace UnTech {
namespace Controller {

template <class T, class ParentT>
class IdMapController {
public:
    using map_type = typename UnTech::idmap<T>;
    using element_type = T;

    const static element_type BLANK_T;

    IdMapController(const IdMapController&) = delete;
    virtual ~IdMapController() = default;

    IdMapController(ParentT& parent);

    ParentT& parent() { return _parent; }
    const ParentT& parent() const { return _parent; }

    // Reloads the map from the parent
    void reloadMap();

    // may be null if no parent is selected
    const map_type* map() const { return _map; }

    const idstring& selectedId() const { return _selectedId; }

    const element_type& selected() const
    {
        if (_map && hasSelected()) {
            return _map->at(_selectedId);
        }
        else {
            return BLANK_T;
        }
    }

    bool hasSelected() const { return _selectedId.isValid(); }

    void selectNone();
    void selectId(const idstring& id);

    void validateSelectedId();

    bool canCreate() const;
    bool canCreate(const idstring& newId) const;
    void create(const idstring& newId);

    bool canCloneSelected() const;
    bool canCloneSelected(const idstring& newId) const;
    void cloneSelected(const idstring& newId);

    bool canRenameSelected() const;
    bool canRenameSelected(const idstring& newId) const;
    void renameSelected(const idstring& newId);

    bool canRemoveSelected() const;
    void removeSelected();

    auto& signal_anyChanged() { return _signal_anyChanged; }
    auto& signal_dataChanged() { return _signal_dataChanged; }
    auto& signal_mapChanged() { return _signal_mapChanged; }
    auto& signal_selectedChanged() { return _signal_selectedChanged; }

protected:
    // can return NULL if parent has nothing selected
    virtual map_type* editable_mapFromParent() = 0;

    // called when element is created but before signals are emitted
    // can edit element_type
    virtual void onCreate(const idstring&, element_type&);

    // can return NULL is nothing is selected
    element_type* editable_selected();

    void edit_selected(std::function<void(element_type&)> const& fun);

protected:
    ParentT& _parent;
    map_type* _map;
    idstring _selectedId;

    sigc::signal<void> _signal_anyChanged;
    sigc::signal<void> _signal_dataChanged;
    sigc::signal<void> _signal_mapChanged;
    sigc::signal<void> _signal_selectedChanged;
};
}
}
