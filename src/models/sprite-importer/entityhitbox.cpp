#include "entityhitbox.h"
#include "frame.h"

using namespace UnTech;
using namespace SpriteImporter;

EntityHitbox::EntityHitbox(std::shared_ptr<Frame> frame)
    : _frame(frame)
    , _aabb({ 0, 0, Frame::MIN_WIDTH, Frame::MIN_HEIGHT })
    , _parameter(0)
{
    const auto frameLocation = frame->location();

    _aabb.x = (frameLocation.width - Frame::MIN_WIDTH) / 2;
    _aabb.y = (frameLocation.height - Frame::MIN_HEIGHT) / 2;
}

EntityHitbox::EntityHitbox(const EntityHitbox& hitbox, std::shared_ptr<Frame> frame)
    : _frame(frame)
    , _aabb(hitbox._aabb)
    , _parameter(hitbox._parameter)
{
}

std::shared_ptr<EntityHitbox> EntityHitbox::clone(std::shared_ptr<Frame> frame)
{
    return std::make_shared<EntityHitbox>(*this, frame);
}

void EntityHitbox::setAabb(const urect& aabb)
{
    auto frame = _frame.lock();

    if (frame) {
        urect newAabb = frame->location().clipInside(aabb, _aabb);

        if (_aabb != newAabb) {
            _aabb = newAabb;
        }
    }
}

void EntityHitbox::setParameter(parameter_t parameter)
{
    auto frame = _frame.lock();
    if (frame) {
        if (_parameter != parameter) {
            _parameter = parameter;
        }
    }
}
