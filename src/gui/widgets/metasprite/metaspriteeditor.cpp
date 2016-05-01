#include "metaspriteeditor.h"

using namespace UnTech::Widgets::MetaSprite;
namespace MS = UnTech::MetaSprite;

const int SCROLL_MAX = UnTech::int_ms8_t::MAX + 16;

MetaSpriteEditor::MetaSpriteEditor()
    : _document()
    , _selection()
    , _rightSideBox(Gtk::ORIENTATION_VERTICAL)
    , _selectedGraphicalEditor(0)
    , _graphicalEditor0(_selection)
    , _graphicalEditor1(_selection)
    , _graphicalContainer(Gtk::ORIENTATION_HORIZONTAL)
    , _graphicalHScroll(Gtk::Adjustment::create(0.0, -SCROLL_MAX, SCROLL_MAX, 1.0, 16.0, 16.0),
                        Gtk::ORIENTATION_HORIZONTAL)
    , _graphicalVScroll(Gtk::Adjustment::create(0.0, -SCROLL_MAX, SCROLL_MAX, 1.0, 16.0, 16.0),
                        Gtk::ORIENTATION_VERTICAL)
    , _graphicalGrid()
    , _tilesetEditor(_selection)
    , _sidebar()
    , _framePane(Gtk::ORIENTATION_VERTICAL)
    , _frameSetBox(Gtk::ORIENTATION_VERTICAL)
    , _frameSetPropertiesEditor(_selection)
    , _paletteList()
    , _paletteEditor(_selection)
    , _frameList()
    , _frameNotebook()
    , _frameParameterEditor(_selection)
    , _frameObjectBox(Gtk::ORIENTATION_VERTICAL)
    , _frameObjectList()
    , _frameObjectEditor(_selection)
    , _actionPointBox(Gtk::ORIENTATION_VERTICAL)
    , _actionPointList()
    , _actionPointEditor(_selection)
    , _entityHitboxBox(Gtk::ORIENTATION_VERTICAL)
    , _entityHitboxList()
    , _entityHitboxEditor(_selection)
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

    // FrameSet
    _frameSetBox.pack_start(_frameSetPropertiesEditor.widget, Gtk::PACK_SHRINK);
    _frameSetBox.pack_start(_paletteList.widget, Gtk::PACK_EXPAND_WIDGET);
    _frameSetBox.pack_start(_paletteEditor.widget, Gtk::PACK_SHRINK);

    // Frame
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

    _selection.signal_frameSetChanged.connect([this](void) {
        auto frameSet = _selection.frameSet();

        if (frameSet) {
            _frameList.setList(frameSet->frames());
            _paletteList.setList(frameSet->palettes());

            _sidebar.set_current_page(FRAMESET_PAGE);
        }
        else {
            _frameList.setList(nullptr);
            _paletteList.setList(nullptr);
        }
    });

    _selection.signal_paletteChanged.connect([this](void) {
        _paletteList.selectItem(_selection.palette());
    });

    _selection.signal_frameChanged.connect([this](void) {
        auto frame = _selection.frame();

        if (_selectedGraphicalEditor == 0) {
            _graphicalEditor0.setFrame(frame);
        }
        else {
            _graphicalEditor1.setFrame(frame);
        }

        if (frame) {
            _frameList.selectItem(frame);

            _frameObjectList.setList(frame->objects());
            _actionPointList.setList(frame->actionPoints());
            _entityHitboxList.setList(frame->entityHitboxes());

            _frameNotebook.set_sensitive(true);

            _sidebar.set_current_page(FRAME_PAGE);
        }
        else {
            _frameObjectList.setList(nullptr);
            _actionPointList.setList(nullptr);
            _entityHitboxList.setList(nullptr);

            _frameNotebook.set_sensitive(false);
        }
    });

    _selection.signal_frameObjectChanged.connect([this](void) {
        _frameObjectList.selectItem(_selection.frameObject());
    });

    _selection.signal_actionPointChanged.connect([this](void) {
        _actionPointList.selectItem(_selection.actionPoint());
    });

    _selection.signal_entityHitboxChanged.connect([this](void) {
        _entityHitboxList.selectItem(_selection.entityHitbox());
    });

    /** Change active tab depending on selection */
    _selection.signal_selectionChanged.connect([this](void) {
        switch (_selection.type()) {
        case Selection::Type::FRAME_OBJECT:
            _frameNotebook.set_current_page(FramePages::FRAME_OBJECT_PAGE);
            break;
        case Selection::Type::ACTION_POINT:
            _frameNotebook.set_current_page(FramePages::ACTION_POINT_PAGE);
            break;

        case Selection::Type::ENTITY_HITBOX:
            _frameNotebook.set_current_page(FramePages::ENTITY_HITBOX_PAGE);
            break;

        default:
            break;
        }
    });

    _paletteList.signal_selected_changed().connect([this](void) {
        _selection.setPalette(_paletteList.getSelected());
    });
    _frameList.signal_selected_changed().connect([this](void) {
        _selection.setFrame(_frameList.getSelected());
    });
    _frameObjectList.signal_selected_changed().connect([this](void) {
        _selection.setFrameObject(_frameObjectList.getSelected());
    });
    _actionPointList.signal_selected_changed().connect([this](void) {
        _selection.setActionPoint(_actionPointList.getSelected());
    });
    _entityHitboxList.signal_selected_changed().connect([this](void) {
        _selection.setEntityHitbox(_entityHitboxList.getSelected());
    });
}

void MetaSpriteEditor::setDocument(std::unique_ptr<Document> document)
{
    if (_document != document) {
        _selection.setFrameSet(nullptr);

        _document = std::move(document);

        if (_document) {
            _selection.setFrameSet(&_document->frameSet());
        }
    }
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
