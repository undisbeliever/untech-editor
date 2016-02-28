#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SIDEBAR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_SIDEBAR_H_

#include "frameeditor.h"
#include "framelist.h"
#include "framesetlist.h"
#include "framesetpropertieseditor.h"
#include "models/sprite-importer.h"
#include "gui/widgets/defaults.h"

#include <gtkmm.h>
#include <glibmm/i18n.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class Sidebar {
public:
    Sidebar()
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

    void setFrameSetList(SI::FrameSet::list_t* frameSetList)
    {
        // No need to test if changed, will only be called on new/load.
        _frameSetList.setList(frameSetList);
        setFrame(nullptr);
    }

    void setFrameSet(std::shared_ptr<SI::FrameSet> frameSet)
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

    void setFrame(std::shared_ptr<SI::Frame> frame)
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

    void setFrameObject(std::shared_ptr<SI::FrameObject> frameObject)
    {
        if (frameObject) {
            setFrame(frameObject->frame().lock());
        }
        else {
            setFrame(nullptr);
        }

        _frameEditor.setFrameObject(frameObject);
    }

    void setActionPoint(std::shared_ptr<SI::ActionPoint> actionPoint)
    {
        if (actionPoint) {
            setFrame(actionPoint->frame().lock());
        }
        else {
            setFrame(nullptr);
        }

        _frameEditor.setActionPoint(actionPoint);
    }

    void setEntityHitbox(std::shared_ptr<SI::EntityHitbox> entityHitbox)
    {
        if (entityHitbox) {
            setFrame(entityHitbox->frame().lock());
        }
        else {
            setFrame(nullptr);
        }

        _frameEditor.setEntityHitbox(entityHitbox);
    }

public:
    Gtk::Notebook widget;

private:
    std::shared_ptr<SI::FrameSet> _selectedFrameSet;
    std::shared_ptr<SI::Frame> _selectedFrame;

    Gtk::Paned _frameSetPane, _framePane;

    FrameSetListEditor _frameSetList;
    FrameSetPropertiesEditor _frameSetPropertiesEditor;

    FrameListEditor _frameList;
    FrameEditor _frameEditor;

    enum {
        FRAMESET_PAGE,
        FRAME_PAGE
    };
};
}
}
}

#endif
