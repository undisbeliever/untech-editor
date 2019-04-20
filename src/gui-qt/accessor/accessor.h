/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
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

template <class T, class U>
class ListUndoHelper;

template <class T>
class ListWithNoSelectionUndoHelper;
template <class T>
class ListAndSelectionUndoHelper;
template <class T>
class ListAndMultipleSelectionUndoHelper;

template <class T>
class ResourceItemUndoHelper;

template <class T>
class ProjectSettingsUndoHelper;

template <class T>
class GridUndoHelper;
}
}
}
