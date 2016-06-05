#include "addtilesdialog.h"
#include "signals.h"
#include "gui/undo/undostack.h"
#include "gui/widgets/defaults.h"
#include "models/metasprite/palette.h"

#include <glibmm/i18n.h>

using namespace UnTech;
using namespace UnTech::Widgets::MetaSprite;

inline void frameSet_addTiles(MS::FrameSet& frameSet,
                              unsigned nNewSmallTiles, unsigned nNewLargeTiles)
{
    class Action : public ::UnTech::Undo::Action {
    public:
        Action() = delete;
        Action(MS::FrameSet& frameSet,
               const unsigned nNewSmallTiles,
               const unsigned nNewLargeTiles)
            : _frameSet(frameSet)
            , _nNewSmallTiles(nNewSmallTiles)
            , _nNewLargeTiles(nNewLargeTiles)
        {
        }

        virtual ~Action() override = default;

        virtual void undo() override
        {
            for (unsigned t = 0; t < _nNewSmallTiles; t++) {
                _frameSet.smallTileset().removeLastTile();
            }
            for (unsigned t = 0; t < _nNewLargeTiles; t++) {
                _frameSet.largeTileset().removeLastTile();
            }

            Signals::frameSetTilesetCountChanged.emit(&_frameSet);
        }

        virtual void redo() override
        {
            for (unsigned t = 0; t < _nNewSmallTiles; t++) {
                _frameSet.smallTileset().addTile();
            }
            for (unsigned t = 0; t < _nNewLargeTiles; t++) {
                _frameSet.largeTileset().addTile();
            }

            Signals::frameSetTilesetCountChanged.emit(&_frameSet);
        }

        virtual const Glib::ustring& message() const override
        {
            const static Glib::ustring message = _("Add Tiles");
            return message;
        }

    private:
        MS::FrameSet& _frameSet;
        unsigned _nNewSmallTiles;
        unsigned _nNewLargeTiles;
    };

    if (nNewSmallTiles > 0 || nNewLargeTiles > 0) {
        auto a = std::make_unique<Action>(frameSet, nNewSmallTiles, nNewLargeTiles);
        a->redo();

        Undo::UndoStack* undoStack = frameSet.document().undoStack();
        if (undoStack) {
            undoStack->add_undo(std::move(a));
        }
    }
}

AddTilesDialog::AddTilesDialog(MS::FrameSet& frameSet, Gtk::Window& parent)
    : Gtk::Dialog(_("Add Tiles"), parent, false)
    , _frameSet(frameSet)
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
            frameSet_addTiles(_frameSet,
                              _nNewSmallTilesSpin.get_value(),
                              _nNewLargeTilesSpin.get_value());
        }
    });
}

void AddTilesDialog::updateGuiValues()
{
    unsigned nSmall = _nNewSmallTilesSpin.get_value() + _frameSet.smallTileset().size();
    unsigned nLarge = _nNewLargeTilesSpin.get_value() + _frameSet.largeTileset().size();

    _smallCount.set_text(Glib::ustring::compose("(%1 total)", nSmall));
    _largeCount.set_text(Glib::ustring::compose("(%1 total)", nLarge));
}
