/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech {
namespace GuiQt {
namespace Accessor {

struct NamedListActions;
class NamedListView;
class NamedListModel;

class SelectedIndexHelper;
class MultipleSelectedIndexesHelper;

class ListActionHelper;

template <class T>
class ResourceItemUndoHelper;

template <class T>
class ProjectSettingsUndoHelper;

template <class T>
class ListUndoHelper;

template <class T>
class ListAndSelectionUndoHelper;

template <class T>
class ListAndMultipleSelectionUndoHelper;

template <class T>
class NamedListUndoHelper;

template <class T>
class NamedListAndSelectionUndoHelper;

template <class T>
class GridUndoHelper;
}
}
}
