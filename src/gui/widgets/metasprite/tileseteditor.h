#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_TILESETEDITOR_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_TILESETEDITOR_H_

#include "tilesetgraphicaleditor.h"

#include <glibmm/i18n.h>
#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class TilesetEditor {
public:
    TilesetEditor(Selection& selection);

    ~TilesetEditor() = default;

    void setZoom(double x, double y);

public:
    Gtk::Box widget;

private:
    Gtk::ScrolledWindow _smallTilesetWindow;
    Gtk::ScrolledWindow _largeTilesetWindow;
    TilesetGraphicalEditor<UnTech::Snes::Tileset4bpp8px> _smallTilesetEditor;
    TilesetGraphicalEditor<UnTech::Snes::Tileset4bpp16px> _largeTilesetEditor;
};
}
}
}

#endif
