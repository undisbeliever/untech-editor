#include "sidebar.h"

using namespace UnTech::Widgets::SpriteImporter;
namespace SI = UnTech::SpriteImporter;

Sidebar::Sidebar()
    : widget()
    , _selectedFrameSet(nullptr)
    , _selectedFrame(nullptr)
    , _frameSetPane(Gtk::ORIENTATION_VERTICAL)
    , _framePane(Gtk::ORIENTATION_VERTICAL)
    , _frameSetList()
    , _frameSetPropertiesEditor()
    , _frameList()
    , _frameEditor()
{
    widget.append_page(_frameSetPane, _("Frame Set"));
    widget.append_page(_framePane, _("Frames"));

    _frameSetPane.set_border_width(DEFAULT_BORDER);
    _frameSetPane.pack1(_frameSetList.widget, true, false);
    _frameSetPane.pack2(_frameSetPropertiesEditor.widget, false, false);

    _framePane.set_border_width(DEFAULT_BORDER);
    _framePane.pack1(_frameList.widget, true, false);
    _framePane.pack2(_frameEditor.widget, false, false);

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

void Sidebar::setFrameSetList(SI::FrameSet::list_t* frameSetList)
{
    // No need to test if changed, will only be called on new/load.
    _frameSetList.setList(frameSetList);
    setFrame(nullptr);
}

void Sidebar::setFrameSet(std::shared_ptr<SI::FrameSet> frameSet)
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

        widget.set_current_page(FRAMESET_PAGE);
    }
}

void Sidebar::setFrame(std::shared_ptr<SI::Frame> frame)
{
    if (_selectedFrame != frame) {
        _selectedFrame = frame;

        if (frame) {
            auto fs = frame->frameSet().lock();
            if (fs) {
                _selectedFrameSet = fs;
                _frameList.setList(fs->frames());
            }
        }

        _frameEditor.setFrame(frame);
        _frameList.selectItem(frame);

        widget.set_current_page(FRAME_PAGE);
    }
}

void Sidebar::setFrameObject(std::shared_ptr<SI::FrameObject> frameObject)
{
    if (frameObject) {
        setFrame(frameObject->frame().lock());
    }
    else {
        setFrame(nullptr);
    }

    _frameEditor.setFrameObject(frameObject);
}

void Sidebar::setActionPoint(std::shared_ptr<SI::ActionPoint> actionPoint)
{
    if (actionPoint) {
        setFrame(actionPoint->frame().lock());
    }
    else {
        setFrame(nullptr);
    }

    _frameEditor.setActionPoint(actionPoint);
}

void Sidebar::setEntityHitbox(std::shared_ptr<SI::EntityHitbox> entityHitbox)
{
    if (entityHitbox) {
        setFrame(entityHitbox->frame().lock());
    }
    else {
        setFrame(nullptr);
    }

    _frameEditor.setEntityHitbox(entityHitbox);
}
