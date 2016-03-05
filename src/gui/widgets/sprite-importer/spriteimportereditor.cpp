#include "spriteimportereditor.h"

using namespace UnTech::Widgets::SpriteImporter;
namespace SI = UnTech::SpriteImporter;

SpriteImporterEditor::SpriteImporterEditor()
    : widget(Gtk::ORIENTATION_HORIZONTAL)
    , _graphicalEditor()
    , _sidebar()
    , _selectedFrameSet(nullptr)
    , _selectedFrame(nullptr)
    , _frameSetPane(Gtk::ORIENTATION_VERTICAL)
    , _framePane(Gtk::ORIENTATION_VERTICAL)
    , _frameSetList()
    , _frameSetPropertiesEditor()
    , _frameList()
    , _frameEditor()
{
    _sidebar.append_page(_frameSetPane, _("Frame Set"));
    _sidebar.append_page(_framePane, _("Frames"));

    _frameSetPane.set_border_width(DEFAULT_BORDER);
    _frameSetPane.pack1(_frameSetList.widget, true, false);
    _frameSetPane.pack2(_frameSetPropertiesEditor.widget, false, false);

    _framePane.set_border_width(DEFAULT_BORDER);
    _framePane.pack1(_frameList.widget, true, false);
    _framePane.pack2(_frameEditor.widget, false, false);

    // ::TODO frameset graphical editor widget::
    widget.pack1(_graphicalEditor, true, false);
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

        _frameSetPropertiesEditor.setFrameSet(frameSet);

        if (frameSet != nullptr) {
            _frameList.setList(frameSet->frames());
        }
        else {
            _frameList.setList(nullptr);
        }

        _sidebar.set_current_page(FRAMESET_PAGE);
    }
}

void SpriteImporterEditor::setFrame(std::shared_ptr<SI::Frame> frame)
{
    if (_selectedFrame != frame) {
        _selectedFrame = frame;

        if (frame) {
            setFrameSet(frame->frameSet().lock());
        }

        _frameEditor.setFrame(frame);
        _frameList.selectItem(frame);

        _sidebar.set_current_page(FRAME_PAGE);
    }
}

void SpriteImporterEditor::setFrameObject(std::shared_ptr<SI::FrameObject> frameObject)
{
    if (frameObject) {
        setFrame(frameObject->frame().lock());
    }
    else {
        setFrame(nullptr);
    }

    _frameEditor.setFrameObject(frameObject);
}

void SpriteImporterEditor::setActionPoint(std::shared_ptr<SI::ActionPoint> actionPoint)
{
    if (actionPoint) {
        setFrame(actionPoint->frame().lock());
    }
    else {
        setFrame(nullptr);
    }

    _frameEditor.setActionPoint(actionPoint);
}

void SpriteImporterEditor::setEntityHitbox(std::shared_ptr<SI::EntityHitbox> entityHitbox)
{
    if (entityHitbox) {
        setFrame(entityHitbox->frame().lock());
    }
    else {
        setFrame(nullptr);
    }

    _frameEditor.setEntityHitbox(entityHitbox);
}
