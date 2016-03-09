#ifndef _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMESETGRAPHICALEDITOR_H_
#define _UNTECH_GUI_WIDGETS_SPRITEIMPORTER_FRAMESETGRAPHICALEDITOR_H_

#include "models/sprite-importer.h"
#include "gui/widgets/defaults.h"

#include <gtkmm.h>

namespace UnTech {
namespace Widgets {
namespace SpriteImporter {

namespace SI = UnTech::SpriteImporter;

class FrameSetGraphicalEditor : public Gtk::DrawingArea {
public:
    FrameSetGraphicalEditor();
    ~FrameSetGraphicalEditor() = default;

    void setFrameSet(std::shared_ptr<SI::FrameSet> frameSet);
    void setFrame(std::shared_ptr<SI::Frame> frame);
    void setFrameObject(std::shared_ptr<SI::FrameObject> frameObject);
    void setActionPoint(std::shared_ptr<SI::ActionPoint> actionPoint);
    void setEntityHitbox(std::shared_ptr<SI::EntityHitbox> entityHitbox);

    void setZoom(double x, double y);

protected:
    void resizeWidget();
    void loadAndScaleImage();

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

    void cr_zoom_rectangle(const Cairo::RefPtr<Cairo::Context>& cr,
                           unsigned x, unsigned y,
                           unsigned width, unsigned height);

private:
    std::shared_ptr<SI::FrameSet> _frameSet;
    std::shared_ptr<SI::Frame> _selectedFrame;

    // Currently void because its only used for highlighting.
    // actual control is handled by other widgets.
    const void* _selectedItem;

    double _zoomX, _zoomY;

    // A pre-scaled copy of the frameset image.
    Glib::RefPtr<Gdk::Pixbuf> _frameSetImage;
};
}
}
}

#endif
