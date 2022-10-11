/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entity-graphics.h"

#include "models/common/iterators.h"
#include "models/metasprite/compiler/framesetcompiler.h"
#include "models/metasprite/drawing.hpp"
#include "models/project/project-data.h"
#include "models/project/project.h"
#include "models/snes/convert-snescolor.h"

namespace UnTech::Gui {

namespace MS = UnTech::MetaSprite::MetaSprite;

static std::shared_ptr<const EntityGraphics> blankEntityGraphics();

EntityGraphicsStore entityGraphicsStore;

static constexpr unsigned MIN_TEXTURE_WIDTH = 256;
static constexpr unsigned MIN_TEXTURE_HEIGHT = 64;
static constexpr unsigned MAX_TEXTURE_SIZE = 1024;

static constexpr int INVALID_FRAME_SIZE = 16;

static constexpr TwoPointRect NOT_SOLID_HITBOX_RECT{ -3, 3, -3, 3 };

EntityGraphics::EntityGraphics(const usize& imageSize)
    : image(imageSize)
{
}

EntityGraphicsStore::EntityGraphicsStore()
    : _mutex()
    , _data(blankEntityGraphics())
    , _entityRomDataCompileId(UINT_MAX)
{
    assert(_data);
}

static void drawInvalidSymbol(Image& image, const unsigned xPos, const unsigned yPos, const unsigned width, const unsigned height)
{
    assert(xPos + width <= image.size().width);
    assert(yPos + height <= image.size().height);

    const rgba color1{ 255, 0, 0, 192 };
    const rgba color2{ 255, 0, 0, 32 };

    bool lineToggle = false;

    for (const auto iy : range(yPos, yPos + height)) {
        auto imgBits = image.scanline(iy);

        lineToggle = !lineToggle;
        bool toggle = lineToggle;

        for (const auto ix : range(xPos, xPos + width)) {
            imgBits[ix] = toggle ? color1 : color2;
            toggle = !toggle;
        }
    }
}

static std::shared_ptr<const EntityGraphics> blankEntityGraphics()
{
    const static std::shared_ptr<const EntityGraphics> eg = []() {
        const usize textureSize(INVALID_FRAME_SIZE, INVALID_FRAME_SIZE);

        auto eg = std::make_shared<EntityGraphics>(textureSize);

        const int h = INVALID_FRAME_SIZE / 2;

        eg->nullSetting.imageRect = TwoPointRect(-h, h, -h, h);
        eg->nullSetting.hitboxRect = NOT_SOLID_HITBOX_RECT;
        eg->nullSetting.uvMin = ImVec2(0, 0);
        eg->nullSetting.uvMax = ImVec2(1, 1);

        drawInvalidSymbol(eg->image, 0, 0, INVALID_FRAME_SIZE, INVALID_FRAME_SIZE);

        return eg;
    }();

    return eg;
}

// THREAD/MEMORY SAFETY: EntityFrame MUST NOT EXIST outside of `processEntityGraphics()`
struct EntityFrame {
    // add fsData here to prevent frameSet/frame references from being invalidated if the MetaSprite is compiled again.
    const std::shared_ptr<const MetaSprite::Compiler::FrameSetData> fsData;

    // It is safe to use a frameSet/frame reference as the ProjectFile is held behind a shared_mutex and cannot be edited
    // while the `processEntityGraphics()` is running.
    const MetaSprite::MetaSprite::FrameSet& frameSet;
    const MetaSprite::MetaSprite::Frame& frame;

    unsigned palette;

    idstring name;
    unsigned entityIndex;
    bool isEntity;
};

struct EntityPackingNode {
    // Input variables
    unsigned entityFrameId; // if UINT_MAX then the id is the missing Image symbol

    unsigned width;
    unsigned height;
    int originX;
    int originY;

    // Output variables
    unsigned x;
    unsigned y;
};

static std::optional<EntityFrame> entityFrame(const Entity::EntityRomEntry& entry,
                                              const MetaSprite::MetaSprite::FrameSet& frameSet,
                                              std::shared_ptr<const MetaSprite::Compiler::FrameSetData> fsData)
{
    auto frame = frameSet.frames.find(entry.displayFrame);
    if (!frame) {
        return std::nullopt;
    }

    if (frame->objects.empty()) {
        return std::nullopt;
    }

    return EntityFrame{
        std::move(fsData),
        frameSet,
        *frame,
        std::min<unsigned>(entry.defaultPalette, frameSet.palettes.size()),
        entry.name,
        0,
        0,
    };
}

static std::optional<EntityFrame> findMetaSprite(const Entity::EntityRomEntry& entry,
                                                 const Project::ProjectFile& projectFile,
                                                 const Project::ProjectData& projectData)
{
    auto frameSetIndex = projectData.frameSets().indexOf(entry.frameSetId);

    if (!frameSetIndex) {
        return std::nullopt;
    }

    if (const auto fs = projectData.frameSets().at(entry.frameSetId)) {
        if (fs->msFrameSet) {
            return entityFrame(entry, *fs->msFrameSet, std::move(fs));
        }
    }

    if (frameSetIndex < projectFile.frameSets.size()) {
        const auto& fsf = projectFile.frameSets.at(*frameSetIndex);
        if (fsf.msFrameSet) {
            return entityFrame(entry, *fsf.msFrameSet, nullptr);
        }
    }

    return std::nullopt;
}

static std::vector<EntityFrame> buildEntityFrameList(const Project::ProjectFile& projectFile,
                                                     const Project::ProjectData& projectData)
{
    const auto& erd = projectFile.entityRomData;

    std::vector<EntityFrame> frames;

    frames.reserve(erd.entities.size() + erd.players.size());

    auto addFrames = [&](const auto& list, const bool isPlayer) {
        for (auto [i, entry] : enumerate(list)) {
            auto ef = findMetaSprite(entry, projectFile, projectData);

            if (ef) {
                ef->isEntity = isPlayer;
                ef->entityIndex = i;

                frames.push_back(std::move(*ef));
            }
        }
    };

    addFrames(erd.entities, true);
    addFrames(erd.players, false);

    return frames;
}

static TwoPointRect frameBounds(const MetaSprite::MetaSprite::Frame& frame)
{
    int minX = INT_MAX;
    int maxX = INT_MIN;
    int minY = INT_MAX;
    int maxY = INT_MIN;

    for (auto& obj : frame.objects) {
        if (obj.location.x < minX) {
            minX = obj.location.x;
        }
        const int x2 = obj.location.x + obj.sizePx();
        if (x2 > maxX) {
            maxX = x2;
        }
        if (obj.location.y < minY) {
            minY = obj.location.y;
        }
        const int y2 = obj.location.y + obj.sizePx();
        if (y2 > maxY) {
            maxY = y2;
        }
    }

    return { minX, maxX, minY, maxY };
}

// An extremely simple row-packing algorithm.
// Returns height of texture
static unsigned packFramesGivenWidth(std::vector<EntityPackingNode>& nodes, const unsigned width)
{
    constexpr unsigned padding = 1;

    unsigned xPos = 0;
    unsigned yPos = 0;
    unsigned rowHeight = 0;

    for (auto& n : nodes) {
        if (n.width + xPos > width) {
            xPos = 0;
            yPos += rowHeight + padding;
            rowHeight = 0;
        }

        n.x = xPos;
        n.y = yPos;

        xPos += n.width + padding;
        rowHeight = std::max(rowHeight, n.height);
    }

    const unsigned height = std::min<unsigned>(yPos + rowHeight, MAX_TEXTURE_SIZE * 4);

    unsigned textureHeight = MIN_TEXTURE_HEIGHT;
    while (textureHeight < height) {
        textureHeight <<= 1;
    }
    return textureHeight;
}

static usize packFrames(std::vector<EntityPackingNode>& nodes)
{
    // This function will only test 3 different texture widths.
    static_assert(MIN_TEXTURE_WIDTH << (3 - 1) == MAX_TEXTURE_SIZE);

    unsigned w = MIN_TEXTURE_WIDTH;
    unsigned h = 64;

    while (w <= MAX_TEXTURE_SIZE) {
        h = packFramesGivenWidth(nodes, w);

        if (h <= w) {
            return usize{ w, h };
        }
        w <<= 1;
    }
    return { w, h };
}

static std::pair<std::vector<EntityPackingNode>, usize>
packEntityFrames(const std::vector<EntityFrame>& entityFrames)
{
    std::vector<EntityPackingNode> nodes;
    nodes.resize(entityFrames.size() + 1);

    {
        auto it = nodes.begin();

        // Invalid entity frame
        {
            it->entityFrameId = UINT_MAX;
            it->width = INVALID_FRAME_SIZE;
            it->height = INVALID_FRAME_SIZE;
            it->originX = -INVALID_FRAME_SIZE / 2;
            it->originY = -INVALID_FRAME_SIZE / 2;

            it++;
        }

        for (auto [i, ef] : enumerate(entityFrames)) {
            const auto bounds = frameBounds(ef.frame);

            assert(bounds.x2 > bounds.x1 && bounds.y2 > bounds.y1);

            it->entityFrameId = i;
            it->width = bounds.x2 - bounds.x1;
            it->height = bounds.y2 - bounds.y1;
            it->originX = bounds.x1;
            it->originY = bounds.y1;

            it++;
        }

        assert(it == nodes.end());
    }

    std::sort(nodes.begin(), nodes.end(),
              [](const auto& a, const auto& b) { return a.height > b.height; });

    usize size = packFrames(nodes);

    return { nodes, size };
}

void processEntityGraphics(const Project::ProjectFile& projectFile,
                           const Project::ProjectData& projectData)
{
    const auto erdCompileId = projectData.projectSettingsStatus().compileId(unsigned(ProjectSettingsIndex::EntityRomData));

    if (erdCompileId == entityGraphicsStore.getEntityRomDataCompileId()) {
        return;
    }

    const std::vector<EntityFrame> entityFrames = buildEntityFrameList(projectFile, projectData);

    const auto [packingNodes, textureSize] = packEntityFrames(entityFrames);

    if (textureSize.height > MAX_TEXTURE_SIZE) {
        entityGraphicsStore.set(blankEntityGraphics(), erdCompileId);
        return;
    }

    const float uvX = 1.0f / textureSize.width;
    const float uvY = 1.0f / textureSize.height;

    auto eg = std::make_shared<EntityGraphics>(textureSize);
    eg->image.fill(rgba());

    {
        auto it = std::find_if(packingNodes.begin(), packingNodes.end(),
                               [&](const auto& node) { return node.entityFrameId > entityFrames.size(); });
        assert(it != packingNodes.end());

        const auto& node = *it;
        DrawEntitySettings& ds = eg->nullSetting;

        ds.imageRect.x1 = node.originX;
        ds.imageRect.x2 = node.originX + int(node.width);
        ds.imageRect.y1 = node.originY;
        ds.imageRect.y2 = node.originY + int(node.height);

        ds.uvMin = ImVec2(node.x * uvX, node.y * uvY);
        ds.uvMax = ImVec2((node.x + node.width) * uvX, (node.y + node.height) * uvY);

        ds.hitboxRect = NOT_SOLID_HITBOX_RECT;

        drawInvalidSymbol(eg->image, node.x, node.y, node.width, node.height);
    }

    eg->entities.resize(projectFile.entityRomData.entities.size(), eg->nullSetting);
    eg->players.resize(projectFile.entityRomData.players.size(), eg->nullSetting);

    for (const auto& node : packingNodes) {
        if (node.entityFrameId < entityFrames.size()) {
            const auto& ef = entityFrames.at(node.entityFrameId);

            DrawEntitySettings ds;

            ds.name = ef.name;

            ds.imageRect.x1 = node.originX;
            ds.imageRect.x2 = node.originX + int(node.width);
            ds.imageRect.y1 = node.originY;
            ds.imageRect.y2 = node.originY + int(node.height);

            ds.uvMin = ImVec2(node.x * uvX, node.y * uvY);
            ds.uvMax = ImVec2((node.x + node.width) * uvX, (node.y + node.height) * uvY);

            if (ef.frame.tileHitbox.exists) {
                ds.hitboxRect = TwoPointRect(ef.frame.tileHitbox.aabb);
            }
            else {
                ds.hitboxRect = NOT_SOLID_HITBOX_RECT;
            }

            std::array<rgba, 16> palette;
            if (ef.palette < ef.frameSet.palettes.size()) {
                auto msPalette = ef.frameSet.palettes.at(ef.palette);
                std::transform(msPalette.begin(), msPalette.end(), palette.begin(),
                               Snes::toRgb);
            }
            else {
                palette.fill(rgba(255, 0, 0));
            }

            MS::drawFrame(eg->image, ef.frameSet, palette, ef.frame,
                          node.x - node.originX, node.y - node.originY);

            if (ef.isEntity) {
                eg->entityNameMap.emplace(ds.name, ef.entityIndex);
                eg->entities.at(ef.entityIndex) = ds;
            }
            else {
                eg->players.at(ef.entityIndex) = ds;
            }
        }
    }

    entityGraphicsStore.set(std::move(eg), erdCompileId);
}

}
