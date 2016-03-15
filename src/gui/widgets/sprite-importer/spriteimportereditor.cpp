#include "spriteimportereditor.h"

using namespace UnTech::Widgets::SpriteImporter;
namespace SI = UnTech::SpriteImporter;

SpriteImporterEditor::SpriteImporterEditor()
    : widget(Gtk::ORIENTATION_HORIZONTAL)
    , _selectedFrameSet(nullptr)
    , _selectedFrame(nullptr)
    , _graphicalWindow()
    , _graphicalEditor()
    , _sidebar()
    , _frameSetPane(Gtk::ORIENTATION_VERTICAL)
    , _framePane(Gtk::ORIENTATION_VERTICAL)
    , _frameSetList()
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

    _sidebar.append_page(_frameSetPane, _("Frame Set"));
    _sidebar.append_page(_framePane, _("Frames"));

    _frameSetPane.set_border_width(DEFAULT_BORDER);
    _frameSetPane.pack1(_frameSetList.widget, true, false);
    _frameSetPane.pack2(_frameSetPropertiesEditor.widget, false, false);

    _framePane.set_border_width(DEFAULT_BORDER);
    _framePane.pack1(_frameList.widget, true, false);
    _framePane.pack2(_frameNotebook, false, false);

    widget.pack1(_graphicalWindow, true, false);
    widget.pack2(_sidebar, false, false);

    /*
     * SLOTS
     * =====
     */
    _frameSetList.signal_selected_changed().connect([this](void) {
        setFrameSet(_frameSetList.getSelected());
    });
    _frameList.signal_selected_changed().connect([this](void) {
        setFrame(_frameList.getSelected());
    });
    _frameObjectList.signal_selected_changed().connect([this](void) {
        setFrameObject(_frameObjectList.getSelected());
    });
    _actionPointList.signal_selected_changed().connect([this](void) {
        setActionPoint(_actionPointList.getSelected());
    });
    _entityHitboxList.signal_selected_changed().connect([this](void) {
        setEntityHitbox(_entityHitboxList.getSelected());
    });

    _graphicalEditor.signal_selectFrame.connect(sigc::mem_fun(*this, &SpriteImporterEditor::setFrame));
    _graphicalEditor.signal_selectFrameObject.connect(sigc::mem_fun(*this, &SpriteImporterEditor::setFrameObject));
    _graphicalEditor.signal_selectActionPoint.connect(sigc::mem_fun(*this, &SpriteImporterEditor::setActionPoint));
    _graphicalEditor.signal_selectEntityHitbox.connect(sigc::mem_fun(*this, &SpriteImporterEditor::setEntityHitbox));
}

void SpriteImporterEditor::setFrameSetList(SI::FrameSet::list_t* frameSetList)
{
    // No need to test if changed, will only be called on new/load.
    _frameSetList.setList(frameSetList);
    setFrame(nullptr);
}

void SpriteImporterEditor::setFrameSet(std::shared_ptr<SI::FrameSet> frameSet)
{
    if (_selectedFrameSet != frameSet) {
        _selectedFrameSet = frameSet;

        if (frameSet) {
            _frameList.setList(frameSet->frames());
        }
        else {
            _frameList.setList(nullptr);
        }

        _frameSetPropertiesEditor.setFrameSet(frameSet);
        _graphicalEditor.setFrameSet(frameSet);

        _sidebar.set_current_page(FRAMESET_PAGE);
    }
}

void SpriteImporterEditor::setFrame(std::shared_ptr<SI::Frame> frame)
{
    if (_selectedFrame != frame) {
        _selectedFrame = frame;

        if (frame) {
            setFrameSet(frame->frameSet());

            _frameObjectList.setList(frame->objects());
            _actionPointList.setList(frame->actionPoints());
            _entityHitboxList.setList(frame->entityHitboxes());

            _frameNotebook.set_sensitive(true);
        }
        else {
            _frameObjectList.setList(nullptr);
            _actionPointList.setList(nullptr);
            _entityHitboxList.setList(nullptr);

            _frameNotebook.set_sensitive(false);
        }

        setFrameObject(nullptr);
        setActionPoint(nullptr);
        setEntityHitbox(nullptr);

        _frameList.selectItem(frame);
        _graphicalEditor.setFrame(frame);

        _sidebar.set_current_page(FRAME_PAGE);
    }
}

void SpriteImporterEditor::setFrameObject(std::shared_ptr<SI::FrameObject> frameObject)
{
    if (frameObject) {
        setFrame(frameObject->frame());
        _frameNotebook.set_current_page(FRAME_OBJECT_PAGE);
    }

    _frameObjectList.selectItem(frameObject);
    _frameObjectEditor.setFrameObject(frameObject);
    _graphicalEditor.setFrameObject(frameObject);
}

void SpriteImporterEditor::setActionPoint(std::shared_ptr<SI::ActionPoint> actionPoint)
{
    if (actionPoint) {
        setFrame(actionPoint->frame());
        _frameNotebook.set_current_page(ACTION_POINT_PAGE);
    }

    _actionPointList.selectItem(actionPoint);
    _actionPointEditor.setActionPoint(actionPoint);
    _graphicalEditor.setActionPoint(actionPoint);
}

void SpriteImporterEditor::setEntityHitbox(std::shared_ptr<SI::EntityHitbox> entityHitbox)
{
    if (entityHitbox) {
        setFrame(entityHitbox->frame());
        _frameNotebook.set_current_page(ENTITY_HITBOX_PAGE);
    }

    _entityHitboxList.selectItem(entityHitbox);
    _entityHitboxEditor.setEntityHitbox(entityHitbox);
    _graphicalEditor.setEntityHitbox(entityHitbox);
}
