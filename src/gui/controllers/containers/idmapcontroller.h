#pragma once

#include "models/common/idmap.h"
#include <algorithm>
#include <cassert>
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

    IdMapController(ParentT& parent)
        : _parent(parent)
    {
        parent.signal_selectedChanged().connect(sigc::mem_fun(
            *this, &IdMapController::reloadMap));

        this->_signal_mapChanged.connect(sigc::mem_fun(
            *this, &IdMapController::validateSelectedId));
    }

    ParentT& parent() { return _parent; }
    const ParentT& parent() const { return _parent; }

    // Reloads the map from the parent
    void reloadMap()
    {
        map_type* m = editable_mapFromParent();
        if (m != _map) {
            _selectedId = idstring();
            _map = m;

            _signal_selectedChanged.emit();
            _signal_mapChanged.emit();
            _signal_anyChanged.emit();
        }
        else {
            validateSelectedId();
        }
    }

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

    void selectNone()
    {
        if (hasSelected()) {
            _selectedId = idstring();

            _signal_selectedChanged.emit();
            _signal_anyChanged.emit();
        }
    }

    void selectId(const idstring& id)
    {
        if (_map && _map->contains(id)) {
            _selectedId = id;

            _signal_selectedChanged.emit();
            _signal_anyChanged.emit();
        }
        else {
            selectNone();
        }
    }

    void validateSelectedId()
    {
        if (_selectedId.isValid()) {
            if (_map == nullptr || !_map->contains(_selectedId)) {
                selectNone();
            }
        }
    }

    bool canCreate() const
    {
        return _map;
    }
    bool canCreate(const idstring& newId) const
    {
        return _map && newId.isValid() && !_map->contains(newId);
    }
    void create(const idstring& newId)
    {
        if (canCreate(newId)) {
            // ::TODO undo engine::

            _map->create(newId);
            _selectedId = newId;

            _signal_mapChanged.emit();
            _signal_selectedChanged.emit();
            _signal_anyChanged.emit();
        }
    }

    bool canCloneSelected() const
    {
        return _map && hasSelected();
    }
    bool canCloneSelected(const idstring& newId) const
    {
        return _map && hasSelected() && newId.isValid() && !_map->contains(newId);
    }
    void cloneSelected(const idstring& newId)
    {
        if (canCloneSelected(newId)) {
            // ::TODO undo engine::

            _map->clone(_selectedId, newId);
            _selectedId = newId;

            _signal_mapChanged.emit();
            _signal_selectedChanged.emit();
            _signal_anyChanged.emit();
        }
    }

    bool canRenameSelected() const
    {
        return _map && hasSelected();
    }
    bool canRenameSelected(const idstring& newId) const
    {
        return _map && hasSelected() && !_map->contains(newId);
    }
    void renameSelected(const idstring& newId)
    {
        if (canRenameSelected(newId)) {
            // ::TODO undo engine::

            _map->rename(_selectedId, newId);
            _selectedId = newId;

            _signal_mapChanged.emit();
            _signal_selectedChanged.emit();
            _signal_anyChanged.emit();
        }
    }

    bool canRemoveSelected() const
    {
        return _map && hasSelected();
    }
    void removeSelected()
    {
        if (canRemoveSelected()) {
            // ::TODO undo engine::

            _map->remove(_selectedId);

            selectNone();
            _signal_mapChanged.emit();
            _signal_anyChanged.emit();
        }
    }

    auto& signal_anyChanged() { return _signal_anyChanged; }
    auto& signal_dataChanged() { return _signal_dataChanged; }
    auto& signal_mapChanged() { return _signal_mapChanged; }
    auto& signal_selectedChanged() { return _signal_selectedChanged; }

protected:
    // can return NULL if parent has nothing selected
    virtual map_type* editable_mapFromParent() = 0;

    // can return NULL is nothing is selected
    element_type* editable_selected()
    {
        if (_map && hasSelected()) {
            return &_map->at(_selectedId);
        }
        else {
            return nullptr;
        }
    }

    void edit_selected(std::function<void(element_type&)> const& fun)
    {
        if (_map && _selectedId.isValid()) {
            // ::TODO undo engine::

            fun(_map->at(_selectedId));
        }

        _signal_dataChanged.emit();
        _signal_anyChanged.emit();
    }

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
