#include "addtilesdialog.h"
#include "gui/widgets/defaults.h"
#include "models/metasprite/palette.h"

#include <glibmm/i18n.h>

using namespace UnTech;
using namespace UnTech::Widgets::MetaSprite;

AddTilesDialog::AddTilesDialog(MS::FrameSetController& controller, Gtk::Window& parent)
    : Gtk::Dialog(_("Add Tiles"), parent, false)
    , _controller(controller)
    , _grid()
    , _nNewSmallTilesSpin(Gtk::Adjustment::create(0.0, 0.0, 64.0))
    , _nNewLargeTilesSpin(Gtk::Adjustment::create(0.0, 0.0, 16.0))
    , _titleLabel(_("Number of new tiles to add:"), Gtk::ALIGN_START)
    , _smallLabel(_("Small Tiles:"), Gtk::ALIGN_START)
    , _largeLabel(_("Large Tiles:"), Gtk::ALIGN_START)
    , _smallCount("", Gtk::ALIGN_END)
    , _largeCount("", Gtk::ALIGN_END)
{
    set_default_response(Gtk::RESPONSE_ACCEPT);

    add_button(_("_OK"), Gtk::RESPONSE_ACCEPT);
    add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);

    // Expand the widgets to fit
    _grid.attach(_titleLabel, 0, 0, 3, 1);

    _grid.attach(_smallLabel, 0, 1, 1, 1);
    _grid.attach(_nNewSmallTilesSpin, 1, 1, 1, 1);
    _grid.attach(_smallCount, 2, 1, 1, 1);

    _grid.attach(_largeLabel, 0, 2, 1, 1);
    _grid.attach(_nNewLargeTilesSpin, 1, 2, 1, 1);
    _grid.attach(_largeCount, 2, 2, 1, 1);

    get_content_area()->add(_grid);

    updateGuiValues();

    show_all_children();

    // SIGNALS
    // =======

    _nNewSmallTilesSpin.signal_value_changed().connect(sigc::mem_fun(
        *this, &AddTilesDialog::updateGuiValues));

    _nNewLargeTilesSpin.signal_value_changed().connect(sigc::mem_fun(
        *this, &AddTilesDialog::updateGuiValues));

    signal_response().connect([this](int response) {
        if (response == Gtk::RESPONSE_ACCEPT) {
            _controller.selected_addTiles(
                _nNewSmallTilesSpin.get_value(),
                _nNewLargeTilesSpin.get_value());
        }
    });
}

void AddTilesDialog::updateGuiValues()
{
    const MS::FrameSet* frameSet = _controller.selected();

    if (frameSet) {
        unsigned nSmall = _nNewSmallTilesSpin.get_value() + frameSet->smallTileset().size();
        unsigned nLarge = _nNewLargeTilesSpin.get_value() + frameSet->largeTileset().size();

        _smallCount.set_text(Glib::ustring::compose("(%1 total)", nSmall));
        _largeCount.set_text(Glib::ustring::compose("(%1 total)", nLarge));
    }
}
