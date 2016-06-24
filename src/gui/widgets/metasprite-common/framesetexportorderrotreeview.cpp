#include "framesetexportorderrotreeview.h"
#include "gui/widgets/common/errormessagedialog.h"
#include "gui/widgets/defaults.h"
#include <glibmm/i18n.h>

using namespace UnTech::Widgets::MetaSpriteCommon;
namespace FSEO = UnTech::MetaSpriteCommon::FrameSetExportOrder;

FrameSetExportOrderRoTreeView::ModelColumns::ModelColumns()
{
    add(pos);
    add(name);
}

FrameSetExportOrderRoTreeView::FrameSetExportOrderRoTreeView()
    : Gtk::TreeView()
    , _columns()
    , _treeStore(Gtk::TreeStore::create(_columns.record()))
{
    set_model(_treeStore);

    append_column("name", _columns.name);
    // ::TODO success image::
}

void FrameSetExportOrderRoTreeView::loadData(std::shared_ptr<const FSEO::ExportOrderDocument> document)
{
    _treeStore->clear();

    if (document != nullptr) {
        auto loadExportNames = [this](const Glib::ustring& name,
                                      const FSEO::ExportName::list_t& exportNames) {

            auto& row = *_treeStore->append();

            row[_columns.name] = name;

            unsigned pos = 0;
            for (const auto& it : exportNames) {
                auto& enRow = *_treeStore->append(row.children());

                enRow[_columns.pos] = pos;
                pos++;

                enRow[_columns.name] = it.first;
            }
        };

        const FSEO::ExportOrder& eo = document->exportOrder();

        loadExportNames(_("Still Frames"), eo.stillFrames());
        loadExportNames(_("Animations"), eo.animations());
    }
}
