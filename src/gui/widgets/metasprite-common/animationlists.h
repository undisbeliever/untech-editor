#pragma once

#include "gui/widgets/common/namedlist.h"
#include "gui/widgets/common/orderedlist.h"
#include "models/metasprite-common/animation.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSpriteCommon {

namespace MSC = UnTech::MetaSpriteCommon;

namespace Private {

class AnimationInstructionModelColumns : public Gtk::TreeModel::ColumnRecord {
public:
    AnimationInstructionModelColumns();

    Gtk::TreeModelColumn<const MSC::AnimationInstruction*> col_item;
    Gtk::TreeModelColumn<unsigned int> col_id;
    Gtk::TreeModelColumn<Glib::ustring> col_operation;
    Gtk::TreeModelColumn<Glib::ustring> col_frame;
    Gtk::TreeModelColumn<Glib::ustring> col_param;

    void buildTreeViewColumns(Gtk::TreeView& treeView);
    void setRowData(Gtk::TreeRow& row, const MSC::AnimationInstruction* inst);
    static Glib::ustring formatFrame(const MSC::FrameReference& ref);
};

class AnimationModelColumns : public Gtk::TreeModel::ColumnRecord {
public:
    AnimationModelColumns();

    Gtk::TreeModelColumn<const MSC::Animation*> col_item;
    Gtk::TreeModelColumn<std::string> col_id;

    void buildTreeViewColumns(Gtk::TreeView& treeView);
    void setRowData(Gtk::TreeRow&, const MSC::Animation*);
};
}

typedef OrderedListEditor<MSC::AnimationInstruction, Private::AnimationInstructionModelColumns> AnimationInstructionListEditor;
typedef NamedListEditor<MSC::Animation, Private::AnimationModelColumns> AnimationListEditor;
}
}
}
