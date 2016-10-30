#include "image2tileset.h"
#include "models/common/file.h"
#include <stdexcept>

using namespace UnTech;
using namespace UnTech::Snes;

template <size_t BD>
void ImageToTileset<BD>::convertAndSave(
    const IndexedImage& image,
    const std::string& tilesetFile, const std::string& paletteFile)
{
    ImageToTileset<BD> converter;
    converter.process(image);

    if (!tilesetFile.empty()) {
        converter.writeTileset(tilesetFile);
    }
    if (!paletteFile.empty()) {
        converter.writePalette(paletteFile);
    }
}

template <size_t BD>
void ImageToTileset<BD>::writeTileset(const std::string& filename) const
{
    std::vector<uint8_t> data = _tileset.snesData();
    File::atomicWrite(filename, data);
}

template <size_t BD>
void ImageToTileset<BD>::writePalette(const std::string& filename) const
{
    std::vector<uint8_t> data = _palette.paletteData();
    File::atomicWrite(filename, data);
}

template <size_t BD>
void ImageToTileset<BD>::process(const IndexedImage& image)
{
    processPalette(image);
    processTileset(image);
}

template <size_t BD>
void ImageToTileset<BD>::processPalette(const IndexedImage& image)
{
    if (image.palette().size() > _palette.N_COLORS) {
        throw std::runtime_error("Too many colors in image, maximum allowed is "
                                 + std::to_string(_palette.N_COLORS));
    }

    assert(image.palette().size() <= _palette.colors().size());
    std::transform(image.palette().begin(), image.palette().end(),
                   _palette.colors().begin(),
                   [](const rgba& c) { return SnesColor(c); });
}

template <size_t BD>
void ImageToTileset<BD>::processTileset(const IndexedImage& image)
{
    constexpr uint8_t PIXEL_MASK = TileT::PIXEL_MASK;
    constexpr unsigned TILE_SIZE = TileT::TILE_SIZE;

    if (image.size().width % TILE_SIZE != 0
        || image.size().height % TILE_SIZE != 0) {

        throw std::runtime_error("Image size is not a multiple of " + std::to_string(TILE_SIZE));
    }

    unsigned tileWidth = image.size().width / TILE_SIZE;
    unsigned tileHeight = image.size().height / TILE_SIZE;

    for (unsigned tileY = 0; tileY < tileHeight; tileY++) {
        for (unsigned tileX = 0; tileX < tileWidth; tileX++) {
            TileT tile;
            uint8_t* tData = tile.rawData();

            for (unsigned py = 0; py < TILE_SIZE; py++) {
                const uint8_t* imgData = image.scanline(tileY * TILE_SIZE + py) + tileX * TILE_SIZE;

                for (unsigned px = 0; px < TILE_SIZE; px++) {
                    *tData++ = *imgData++ & PIXEL_MASK;
                }
            }

            _tileset.addTile(tile);
        }
    }
}

template class Snes::ImageToTileset<2>;
template class Snes::ImageToTileset<4>;
template class Snes::ImageToTileset<8>;
