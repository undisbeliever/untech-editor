#include "widgets/sprite-importer/spriteimporterapplication.h"
#include "widgets/sprite-importer/spriteimporterwindow.h"

namespace UTWSI = UnTech::Widgets::SpriteImporter;
namespace SI = UnTech::SpriteImporter;

int main(int argc, char* argv[])
{
    auto app = UTWSI::SpriteImporterApplication::create();

    return app->run(argc, argv);
}
