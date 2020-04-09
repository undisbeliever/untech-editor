/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech {
namespace GuiQt {
namespace Accessor {

class AbstractListAccessor;
class AbstractListSingleSelectionAccessor;
class AbstractListMultipleSelectionAccessor;
class AbstractNamedListAccessor;
class AbstractChildListSingleSelectionAccessor;
class AbstractChildListMultipleSelectionAccessor;

struct ListActions;
struct MultiListActions;
struct NamedListActions;

class NamedListView;
class NamedListModel;
class NamedListDock;

class MultipleSelectionTableView;
class MultipleSelectionTableDock;

template <class, class>
class ListUndoHelper;

template <class>
class ListWithNoSelectionUndoHelper;
template <class>
class ListAndSelectionUndoHelper;
template <class>
class ListAndMultipleSelectionUndoHelper;

template <class>
class ResourceItemUndoHelper;

template <class>
class ProjectSettingsUndoHelper;

template <class>
class GridUndoHelper;

template <class, class>
class VectorSingleSelectionAccessor;

template <class, class>
class NamedListAccessor;

template <class, class>
class ChildVectorAccessor;

template <class, class>
class ChildVectorMultipleSelectionAccessor;

template <class, class, class>
class NestedNlvMulitpleSelectionAccessor;

}
}
}
