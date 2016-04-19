#include "spriteimportereditor.h"

using namespace UnTech::Widgets::SpriteImporter;
namespace SI = UnTech::SpriteImporter;

SpriteImporterEditor::SpriteImporterEditor()
    : _document()
    , _selection()
    , _graphicalWindow()
    , _graphicalEditor(_selection)
    , _sidebar()
    , _framePane(Gtk::ORIENTATION_VERTICAL)
    , _frameSetPropertiesEditor()
    , _frameList()
    , _frameNotebook()
    , _frameParameterEditor()
    , _frameObjectBox(Gtk::ORIENTATION_VERTICAL)
    , _frameObjectList()
    , _frameObjectEditor()
    , _actionPointBox(Gtk::ORIENTATION_VERTICAL)
    , _actionPointList()
    , _actionPointEditor()
    , _entityHitboxBox(Gtk::ORIENTATION_VERTICAL)
    , _entityHitboxList()
    , _entityHitboxEditor()
{
    _frameNotebook.set_scrollable(true);
    _frameNotebook.popup_enable();

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

    _graphicalWindow.add(_graphicalEditor);
    _graphicalWindow.set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);

    _sidebar.append_page(_frameSetPropertiesEditor.widget, _("Frame Set"));
    _sidebar.append_page(_framePane, _("Frames"));

    _framePane.set_border_width(DEFAULT_BORDER);
    _framePane.pack1(_frameList.widget, true, false);
    _framePane.pack2(_frameNotebook, false, false);

    widget.pack1(_graphicalWindow, true, false);
    widget.pack2(_sidebar, false, false);

    /*
     * SLOTS
     * =====
     */
    _selection.signal_frameSetChanged.connect([this](void) {
        auto frameSet = _selection.frameSet();

        if (frameSet) {
            _frameList.setList(frameSet->frames());

            _sidebar.set_current_page(FRAMESET_PAGE);
        }
        else {
            _frameList.setList(nullptr);
        }

        _frameSetPropertiesEditor.setFrameSet(frameSet);

    });

    _selection.signal_frameChanged.connect([this](void) {
        auto frame = _selection.frame();

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

        _frameParameterEditor.setFrame(frame);
    });

    _selection.signal_frameObjectChanged.connect([this](void) {
        _frameObjectList.selectItem(_selection.frameObject());
        _frameObjectEditor.setFrameObject(_selection.frameObject());
    });

    _selection.signal_actionPointChanged.connect([this](void) {
        _actionPointList.selectItem(_selection.actionPoint());
        _actionPointEditor.setActionPoint(_selection.actionPoint());
    });

    _selection.signal_entityHitboxChanged.connect([this](void) {
        _entityHitboxList.selectItem(_selection.entityHitbox());
        _entityHitboxEditor.setEntityHitbox(_selection.entityHitbox());
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

    _frameSetPropertiesEditor.signal_selectTransparentClicked().connect(
        sigc::mem_fun(_graphicalEditor, &FrameSetGraphicalEditor::enableSelectTransparentColor));

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

void SpriteImporterEditor::setDocument(std::unique_ptr<Document> document)
{
    if (_document != document) {
        _selection.setFrameSet(document->frameSet());

        _document = std::move(document);
    }
}

void SpriteImporterEditor::setZoom(int zoom, double aspectRatio)
{
    _graphicalEditor.setZoom(zoom * aspectRatio, zoom);
}
