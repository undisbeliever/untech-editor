#include "spriteimportereditor.h"
#include "gui/widgets/defaults.h"

#include <glibmm/i18n.h>

using namespace UnTech::Widgets::SpriteImporter;

SpriteImporterEditor::SpriteImporterEditor(SI::SpriteImporterController& controller)
    : widget()
    , _controller(controller)
    , _graphicalWindow()
    , _graphicalEditor(controller)
    , _sidebar()
    , _framePane(Gtk::ORIENTATION_VERTICAL)
    , _frameSetPropertiesEditor(controller)
    , _frameList(controller.frameController())
    , _frameNotebook()
    , _frameParameterEditor(controller)
    , _frameObjectBox(Gtk::ORIENTATION_VERTICAL)
    , _frameObjectList(controller.frameObjectController())
    , _frameObjectEditor(controller)
    , _actionPointBox(Gtk::ORIENTATION_VERTICAL)
    , _actionPointList(controller.actionPointController())
    , _actionPointEditor(controller)
    , _entityHitboxBox(Gtk::ORIENTATION_VERTICAL)
    , _entityHitboxList(controller.entityHitboxController())
    , _entityHitboxEditor(controller)
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

    // Controller Signals
    _controller.frameSetController().signal_selectedChanged().connect([this](void) {
        _sidebar.set_current_page(SidebarPages::FRAMESET_PAGE);
    });

    _controller.frameController().signal_selectedChanged().connect([this](void) {
        const SI::Frame* frame = _controller.frameController().selected();
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
}
