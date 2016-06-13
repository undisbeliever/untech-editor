#include "metaspriteeditor.h"
#include "gui/widgets/defaults.h"
#include <glibmm/i18n.h>

using namespace UnTech::Widgets::MetaSprite;

typedef MS::MetaSpriteController::SelectedTypeController::Type SelectionType;

const int SCROLL_MAX = UnTech::int_ms8_t::MAX + 16;

MetaSpriteEditor::MetaSpriteEditor(MS::MetaSpriteController& controller)
    : widget()
    , _controller(controller)
    , _rightSideBox(Gtk::ORIENTATION_VERTICAL)
    , _selectedGraphicalEditor(0)
    , _graphicalEditor0(_controller)
    , _graphicalEditor1(_controller)
    , _graphicalContainer(Gtk::ORIENTATION_HORIZONTAL)
    , _graphicalHScroll(Gtk::Adjustment::create(0.0, -SCROLL_MAX, SCROLL_MAX, 1.0, 16.0, 16.0),
                        Gtk::ORIENTATION_HORIZONTAL)
    , _graphicalVScroll(Gtk::Adjustment::create(0.0, -SCROLL_MAX, SCROLL_MAX, 1.0, 16.0, 16.0),
                        Gtk::ORIENTATION_VERTICAL)
    , _graphicalGrid()
    , _tilesetEditor(_controller)
    , _sidebar()
    , _framePane(Gtk::ORIENTATION_VERTICAL)
    , _frameSetBox(Gtk::ORIENTATION_VERTICAL)
    , _frameSetPropertiesEditor(_controller.abstractFrameSetController())
    , _paletteFrame(_("Palettes:"))
    , _paletteBox(Gtk::ORIENTATION_VERTICAL)
    , _paletteList(_controller.paletteController())
    , _paletteEditor(_controller.paletteController())
    , _frameList(_controller.frameController())
    , _frameNotebook()
    , _frameParameterEditor(_controller.frameController())
    , _frameObjectBox(Gtk::ORIENTATION_VERTICAL)
    , _frameObjectList(_controller.frameObjectController())
    , _frameObjectEditor(_controller)
    , _actionPointBox(Gtk::ORIENTATION_VERTICAL)
    , _actionPointList(_controller.actionPointController())
    , _actionPointEditor(_controller.actionPointController())
    , _entityHitboxBox(Gtk::ORIENTATION_VERTICAL)
    , _entityHitboxList(_controller.entityHitboxController())
    , _entityHitboxEditor(_controller.entityHitboxController())
{

    // Graphical
    _selectedGraphicalEditor = 0;
    _graphicalContainer.pack1(_graphicalEditor0, true, true);
    _graphicalGrid.attach(_graphicalContainer, 0, 0, 1, 1);
    _graphicalGrid.attach(_graphicalHScroll, 0, 1, 1, 1);
    _graphicalGrid.attach(_graphicalVScroll, 1, 0, 1, 1);

    // Tileset
    _rightSideBox.pack_start(_graphicalGrid, true, true);
    _rightSideBox.pack_start(_tilesetEditor.widget, false, false);

    // Sidebar
    _frameNotebook.set_size_request(-1, 350);
    _frameNotebook.set_scrollable(true);
    _frameNotebook.popup_enable();

    // FrameSet Pane
    _paletteBox.pack_start(_paletteList.widget, Gtk::PACK_EXPAND_WIDGET);
    _paletteBox.pack_start(_paletteEditor.widget, Gtk::PACK_SHRINK);

    _paletteFrame.set_shadow_type(Gtk::SHADOW_ETCHED_OUT);
    _paletteFrame.add(_paletteBox);

    _frameSetBox.set_border_width(DEFAULT_BORDER);
    _frameSetBox.set_spacing(DEFAULT_ROW_SPACING);
    _frameSetBox.pack_start(_frameSetPropertiesEditor.widget, Gtk::PACK_EXPAND_WIDGET);
    _frameSetBox.pack_start(_paletteFrame, Gtk::PACK_EXPAND_WIDGET);

    // Frame Pane
    _frameNotebook.append_page(_frameParameterEditor.widget, _("Frame"));

    _frameObjectBox.set_border_width(DEFAULT_BORDER);
    _frameObjectBox.pack_start(_frameObjectList.widget, Gtk::PACK_EXPAND_WIDGET);
    _frameObjectBox.pack_start(_frameObjectEditor.widget, Gtk::PACK_SHRINK);
    _frameNotebook.append_page(_frameObjectBox, _("Objects"));

    _actionPointBox.set_border_width(DEFAULT_BORDER);
    _actionPointBox.pack_start(_actionPointList.widget, Gtk::PACK_EXPAND_WIDGET);
    _actionPointBox.pack_start(_actionPointEditor.widget, Gtk::PACK_SHRINK);
    _frameNotebook.append_page(_actionPointBox, _("Action Points"));

    _entityHitboxBox.set_border_width(DEFAULT_BORDER);
    _entityHitboxBox.pack_start(_entityHitboxList.widget, Gtk::PACK_EXPAND_WIDGET);
    _entityHitboxBox.pack_start(_entityHitboxEditor.widget, Gtk::PACK_SHRINK);
    _frameNotebook.append_page(_entityHitboxBox, _("Entity Hitboxes"));

    _sidebar.append_page(_frameSetBox, _("Frame Set"));
    _sidebar.append_page(_framePane, _("Frames"));

    _framePane.set_border_width(DEFAULT_BORDER);
    _framePane.pack1(_frameList.widget, true, false);
    _framePane.pack2(_frameNotebook, false, false);

    widget.pack1(_rightSideBox, true, true);
    widget.pack2(_sidebar, false, false);

    /*
     * SLOTS
     * =====
     */

    // Controller Signals
    _controller.frameSetController().signal_selectedChanged().connect([this](void) {
        _sidebar.set_current_page(SidebarPages::FRAMESET_PAGE);
    });

    _controller.paletteController().signal_selectedChanged().connect([this](void) {
        _sidebar.set_current_page(SidebarPages::FRAMESET_PAGE);
    });

    _controller.frameController().signal_selectedChanged().connect([this](void) {
        const MS::Frame* frame = _controller.frameController().selected();

        if (_selectedGraphicalEditor == 0) {
            _graphicalEditor0.setFrame(frame);
        }
        else {
            _graphicalEditor1.setFrame(frame);
        }

        if (frame) {
            _sidebar.set_current_page(SidebarPages::FRAME_PAGE);
            _frameNotebook.set_current_page(FramePages::FRAME_PARAMETERS_PAGE);
        }
    });

    _controller.frameObjectController().signal_selectedChanged().connect([this](void) {
        if (_controller.frameObjectController().selected() != nullptr) {
            _frameNotebook.set_current_page(FramePages::FRAME_OBJECT_PAGE);
        }
    });

    _controller.actionPointController().signal_selectedChanged().connect([this](void) {
        if (_controller.actionPointController().selected() != nullptr) {
            _frameNotebook.set_current_page(FramePages::FRAME_OBJECT_PAGE);
        }
    });

    _controller.entityHitboxController().signal_selectedChanged().connect([this](void) {
        if (_controller.entityHitboxController().selected() != nullptr) {
            _frameNotebook.set_current_page(FramePages::ENTITY_HITBOX_PAGE);
        }
    });

    // Reset scrollbars to 0 on right click
    _graphicalHScroll.add_events(Gdk::BUTTON_RELEASE_MASK);
    _graphicalHScroll.signal_button_release_event().connect([&](GdkEventButton* event) {
        if (event->button == 3) {
            _graphicalHScroll.set_value(0);
            return true;
        }
        return false;
    });
    _graphicalVScroll.add_events(Gdk::BUTTON_RELEASE_MASK);
    _graphicalVScroll.signal_button_release_event().connect([&](GdkEventButton* event) {
        if (event->button == 3) {
            _graphicalVScroll.set_value(0);
            return true;
        }
        return false;
    });

    _graphicalHScroll.signal_value_changed().connect(
        sigc::mem_fun(*this, &MetaSpriteEditor::on_scroll_changed));
    _graphicalVScroll.signal_value_changed().connect(
        sigc::mem_fun(*this, &MetaSpriteEditor::on_scroll_changed));

    // Set _selectedGraphicalEditor when an editor is clicked
    _graphicalEditor0.signal_grab_focus().connect([this](void) {
        _selectedGraphicalEditor = 0;
    });
    // Set _selectedGraphicalEditor when an editor is clicked
    _graphicalEditor1.signal_grab_focus().connect([this](void) {
        _selectedGraphicalEditor = 1;
    });
}

void MetaSpriteEditor::setShowTwoEditors(bool showTwoEditors)
{
    if (showTwoEditors == true) {
        _graphicalContainer.pack1(_graphicalEditor0, true, true);
        _graphicalContainer.pack2(_graphicalEditor1, true, true);

        _graphicalContainer.show_all_children();
    }
    else {
        // remove unused editor
        if (_selectedGraphicalEditor == 0) {
            _graphicalContainer.remove(_graphicalEditor1);
            _graphicalEditor1.hide();
        }
        else {
            _graphicalContainer.remove(_graphicalEditor0);
            _graphicalEditor0.hide();
        }
    }
}

void MetaSpriteEditor::on_scroll_changed()
{
    _graphicalEditor0.setCenter(_graphicalHScroll.get_value(),
                                _graphicalVScroll.get_value());
    _graphicalEditor1.setCenter(_graphicalHScroll.get_value(),
                                _graphicalVScroll.get_value());
}

void MetaSpriteEditor::setZoom(int zoom, double aspectRatio)
{
    _graphicalEditor0.setZoom(zoom * aspectRatio, zoom);
    _graphicalEditor1.setZoom(zoom * aspectRatio, zoom);
    _tilesetEditor.setZoom(zoom * aspectRatio, zoom);
}
