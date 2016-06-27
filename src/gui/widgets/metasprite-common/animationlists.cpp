#include "animationlists.h"

#include <glibmm/i18n.h>
#include <iomanip>

using namespace UnTech::Widgets::MetaSpriteCommon::Private;

AnimationInstructionModelColumns::AnimationInstructionModelColumns()
{
    add(col_item);
    add(col_id);
    add(col_operation);
    add(col_frame);
    add(col_param);
}

void AnimationInstructionModelColumns::buildTreeViewColumns(Gtk::TreeView& treeView)
{
    treeView.append_column(_("Instruction"), col_operation);
    treeView.append_column(_("Frame"), col_frame);
    treeView.append_column(_("Parameter"), col_param);
}

void AnimationInstructionModelColumns::setRowData(Gtk::TreeRow& row, const MSC::AnimationInstruction* inst)
{
    typedef MSC::AnimationBytecode::Enum BC;

    const MSC::AnimationBytecode& op = inst->operation();

    row[col_operation] = op.string();

    switch (op.value()) {
    case BC::STOP:
    case BC::GOTO_START:
        row[col_frame] = "";
        row[col_param] = "";
        break;

    case BC::GOTO_ANIMATION:
        row[col_frame] = "";
        row[col_param] = inst->gotoLabel();
        break;

    case BC::GOTO_OFFSET: {
        row[col_frame] = "";

        int p = inst->parameter();
        if (p > 0) {
            row[col_param] = "+" + Glib::ustring::format(p);
        }
        else if (p < 0) {
            // format will include minus sign.
            row[col_param] = Glib::ustring::format(p);
        }
        else {
            row[col_param] = "";
        }
        break;
    }

    case BC::SET_FRAME_AND_WAIT_FRAMES:
        row[col_frame] = formatFrame(inst->frame());
        row[col_param] = Glib::ustring::compose(_("%1 frames"),
                                                inst->parameter());
        break;

    case BC::SET_FRAME_AND_WAIT_TIME:
        row[col_frame] = formatFrame(inst->frame());
        row[col_param] = Glib::ustring::compose("%1 (%2 ms)",
                                                inst->parameter(),
                                                inst->parameter() * 1000 / 75);
        break;

    case BC::SET_FRAME_AND_WAIT_XVECL:
    case BC::SET_FRAME_AND_WAIT_YVECL:
        row[col_frame] = formatFrame(inst->frame());
        row[col_param] = Glib::ustring::compose(
            "%1 (%2 px)",
            inst->parameter(),
            Glib::ustring::format(std::setprecision(3),
                                  (double)inst->parameter() / 32));
        break;
    }
}

Glib::ustring AnimationInstructionModelColumns::formatFrame(const MSC::FrameReference& ref)
{
    if (!ref.hFlip) {
        if (!ref.vFlip) {
            return ref.frameName;
        }
        else {
            return Glib::ustring::compose("%1 (vflip)", ref.frameName);
        }
    }
    else {
        if (!ref.vFlip) {
            return Glib::ustring::compose("%1 (hflip)", ref.frameName);
        }
        else {
            return Glib::ustring::compose("%1 (hvflip)", ref.frameName);
        }
    }
}

AnimationModelColumns::AnimationModelColumns()
{
    add(col_item);
    add(col_id);
}

void AnimationModelColumns::buildTreeViewColumns(Gtk::TreeView& treeView)
{
    int nameId = treeView.append_column(_("Name"), col_id) - 1;
    Gtk::TreeViewColumn* nameColumn = treeView.get_column(nameId);
    nameColumn->set_sort_column(col_id);
    nameColumn->set_expand(true);

    treeView.set_headers_clickable(true);
    treeView.set_search_column(col_id);
}

void AnimationModelColumns::setRowData(Gtk::TreeRow&, const MSC::Animation*)
{
}
