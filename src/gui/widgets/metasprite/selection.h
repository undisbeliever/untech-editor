#ifndef _UNTECH_GUI_WIDGETS_METASPRITE_SELECTION_H_
#define _UNTECH_GUI_WIDGETS_METASPRITE_SELECTION_H_

#include "gui/widgets/metasprite-format/selection.h"
#include "models/metasprite.h"

#include <glibmm/ustring.h>
#include <sigc++/signal.h>

namespace UnTech {
namespace Widgets {
namespace MetaSprite {

namespace MS = UnTech::MetaSprite;

class Selection : public MetaSpriteFormat::Selection {
public:
    enum class Type {
        NONE = 0,
        FRAME_OBJECT,
        ACTION_POINT,
        ENTITY_HITBOX
    };

    void setFrameSet(MS::FrameSet* frameSet);
    void setPalette(MS::Palette* palette);
    void setFrame(MS::Frame* frame);
    void setFrameObject(MS::FrameObject* frameObject);
    void setActionPoint(MS::ActionPoint* actionPoint);
    void setEntityHitbox(MS::EntityHitbox* entityHitbox);

    /**
     * Set the color to draw and edit tiles with.
     * If negative then not in edit tile mode.
     */
    void setEditTileColor(int colorId);

    inline void unsetEditTileColor()
    {
        setEditTileColor(-1);
    }

    // Will call dontMergeNextAction on the frameSet's undo stack
    // Always returns false so it can connect to a signal_focus_out_event
    bool dontMergeNextUndoAction();

    // WILL ALWAYS emit a selectionChanged
    void unselectAll();

    Glib::ustring typeString() const;

    Type type() const { return _type; }
    MS::FrameSet* frameSet() const { return _frameSet; }
    MS::Palette* palette() const { return _palette; }
    MS::Frame* frame() const { return _frame; }
    MS::FrameObject* frameObject() const { return _frameObject; }
    MS::ActionPoint* actionPoint() const { return _actionPoint; }
    MS::EntityHitbox* entityHitbox() const { return _entityHitbox; }

    /**
     * The color to draw and edit tiles with
     * If negative then not in edit tile mode.
     */
    int editTileColor() const { return _editTileColor; }
    bool inEditTileMode() const { return _editTileColor >= 0; }

    bool canCrudSelected() const { return _type != Type::NONE; }
    bool canMoveSelectedUp() const;
    bool canMoveSelectedDown() const;

    void createNewOfSelectedType();
    void cloneSelected();
    void removeSelected();
    void moveSelectedUp();
    void moveSelectedDown();

    sigc::signal<void> signal_selectionChanged;
    sigc::signal<void> signal_paletteChanged;
    sigc::signal<void> signal_frameChanged;
    sigc::signal<void> signal_frameObjectChanged;
    sigc::signal<void> signal_actionPointChanged;
    sigc::signal<void> signal_entityHitboxChanged;
    sigc::signal<void> signal_editTileColorChanged;

private:
    // updates current without nulling child selection
    void updateFrameSet(MS::FrameSet& frameSet);
    void updateFrame(MS::Frame& frame);

    Type _type = Type::NONE;
    MS::FrameSet* _frameSet = nullptr;
    MS::Palette* _palette = nullptr;
    MS::Frame* _frame = nullptr;
    MS::FrameObject* _frameObject = nullptr;
    MS::ActionPoint* _actionPoint = nullptr;
    MS::EntityHitbox* _entityHitbox = nullptr;
    int _editTileColor;
};
}
}
}
#endif
