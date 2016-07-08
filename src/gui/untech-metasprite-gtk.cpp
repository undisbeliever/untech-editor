#include "widgets/metasprite/metaspriteapplication.h"
#include "widgets/metasprite/metaspritewindow.h"

namespace UTWMS = UnTech::Widgets::MetaSprite;

int main(int argc, char* argv[])
{
    auto app = UTWMS::MetaSpriteApplication::create();

    return app->run(argc, argv);
}
