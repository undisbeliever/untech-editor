#include "idmapcontroller.h"

namespace UnTech {
namespace Controller {

template <typename ElementT, class ParentT>
const ElementT IdMapController<ElementT, ParentT>::BLANK_T = ElementT();

template <class T, class ParentT>
IdMapController<T, ParentT>::IdMapController(ParentT& parent)
    : _parent(parent)
{
    parent.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &IdMapController::reloadMap));

    this->_signal_mapChanged.connect(sigc::mem_fun(
        *this, &IdMapController::validateSelectedId));
}

template <class T, class PT>
void IdMapController<T, PT>::reloadMap()
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

template <class T, class PT>
void IdMapController<T, PT>::selectNone()
{
    if (hasSelected()) {
        _selectedId = idstring();

        _signal_selectedChanged.emit();
        _signal_anyChanged.emit();
    }
}

template <class T, class PT>
void IdMapController<T, PT>::selectId(const idstring& id)
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

template <class T, class PT>
void IdMapController<T, PT>::validateSelectedId()
{
    if (_selectedId.isValid()) {
        if (_map == nullptr || !_map->contains(_selectedId)) {
            selectNone();
        }
    }
}

template <class T, class PT>
bool IdMapController<T, PT>::canCreate() const
{
    return _map;
}

template <class T, class PT>
bool IdMapController<T, PT>::canCreate(const idstring& newId) const
{
    return _map && newId.isValid() && !_map->contains(newId);
}

template <class T, class PT>
void IdMapController<T, PT>::create(const idstring& newId)
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

template <class T, class PT>
bool IdMapController<T, PT>::canCloneSelected() const
{
    return _map && hasSelected();
}

template <class T, class PT>
bool IdMapController<T, PT>::canCloneSelected(const idstring& newId) const
{
    return _map && hasSelected() && newId.isValid() && !_map->contains(newId);
}

template <class T, class PT>
void IdMapController<T, PT>::cloneSelected(const idstring& newId)
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

template <class T, class PT>
bool IdMapController<T, PT>::canRenameSelected() const
{
    return _map && hasSelected();
}

template <class T, class PT>
bool IdMapController<T, PT>::canRenameSelected(const idstring& newId) const
{
    return _map && hasSelected() && !_map->contains(newId);
}

template <class T, class PT>
void IdMapController<T, PT>::renameSelected(const idstring& newId)
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

template <class T, class PT>
bool IdMapController<T, PT>::canRemoveSelected() const
{
    return _map && hasSelected();
}

template <class T, class PT>
void IdMapController<T, PT>::removeSelected()
{
    if (canRemoveSelected()) {
        // ::TODO undo engine::

        _map->remove(_selectedId);

        selectNone();
        _signal_mapChanged.emit();
        _signal_anyChanged.emit();
    }
}

template <class T, class PT>
T* IdMapController<T, PT>::editable_selected()
{
    if (_map && hasSelected()) {
        return &_map->at(_selectedId);
    }
    else {
        return nullptr;
    }
}

template <class T, class PT>
void IdMapController<T, PT>::edit_selected(std::function<void(T&)> const& fun)
{
    if (_map && _selectedId.isValid()) {
        // ::TODO undo engine::

        fun(_map->at(_selectedId));
    }

    _signal_dataChanged.emit();
    _signal_anyChanged.emit();
}
}
}
