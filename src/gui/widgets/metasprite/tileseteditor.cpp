#include "tileseteditor.h"
#include "tilesetgraphicaleditor.hpp"

using namespace UnTech::Widgets::MetaSprite;
namespace MS = UnTech::MetaSprite;

TilesetEditor::TilesetEditor(MS::MetaSpriteController& controller)
    : widget(Gtk::ORIENTATION_VERTICAL)
    , _smallTilesetWindow()
    , _largeTilesetWindow()
    , _smallTilesetEditor(controller)
    , _largeTilesetEditor(controller)
{
    _smallTilesetWindow.add(_smallTilesetEditor);
    _smallTilesetWindow.set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_NEVER);
    _smallTilesetWindow.set_overlay_scrolling(false);

    _largeTilesetWindow.add(_largeTilesetEditor);
    _largeTilesetWindow.set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_NEVER);
    _largeTilesetWindow.set_overlay_scrolling(false);

    widget.add(_smallTilesetWindow);
    widget.add(_largeTilesetWindow);
}

void TilesetEditor::setZoom(double x, double y)
{
    _smallTilesetEditor.setZoom(x, y);
    _largeTilesetEditor.setZoom(x, y);
}
