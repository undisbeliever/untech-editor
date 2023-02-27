/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project-data.h"

namespace UnTech::Project {

// DataStore
// =========

template <typename T>
void DataStore<T>::clearAllAndResize(size_t size)
{
    data.write([&](auto& d) {
        d.mapping.clear();
        d.mapping.reserve(size);

        d.data.clear();
        d.data.resize(size);
    });
}

template <typename T>
bool DataStore<T>::store(const size_t index, const idstring& name, std::shared_ptr<const T>&& res_data)
{
    return data.write_and_return_bool([&](auto& d) {
        auto& entry = d.data.at(index);

        if (entry.first != name) {
            // Name changed
            auto it = d.mapping.find(entry.first);
            if (it != d.mapping.end()) {
                // Only remove old name if it points to the correct resource
                if (it->second == index) {
                    d.mapping.erase(it);
                }
            }
        }

        entry.first = name;
        entry.second = std::move(res_data);

        // Return false if name already used by a resource that is not `index`.
        if (name.isValid()) {
            const auto [it, b] = d.mapping.try_emplace(name, index);
            return it->second == index;
        }
        else {
            return false;
        }
    });
}

template <typename T>
size_t DataStore<T>::size() const
{
    return data.read_and_return_size_t([](const auto& d) { return d.data.size(); });
}

template <typename T>
std::pair<std::optional<size_t>, std::shared_ptr<const T>> DataStore<T>::indexAndDataFor(const idstring& id) const
{
    return data.template read_and_return_index_and_const_shared_ptr<T>([&](const auto& d) -> std::pair<std::optional<size_t>, std::shared_ptr<const T>> {
        const auto it = d.mapping.find(id);
        if (it != d.mapping.end()) {
            const size_t index = it->second;
            if (index < d.data.size()) {
                return std::pair{ index, d.data.at(index).second };
            }
        }
        return std::pair{ std::nullopt, nullptr };
    });
}

template <typename T>
std::shared_ptr<const T> DataStore<T>::at(unsigned index) const
{
    return data.template read_and_return_const_shared_ptr<T>([&](const auto& d) -> std::shared_ptr<const T> {
        if (index < d.data.size()) {
            return d.data.at(index).second;
        }
        else {
            return nullptr;
        }
    });
}

template class DataStore<UnTech::MetaSprite::Compiler::FrameSetData>;
template class DataStore<Resources::PaletteData>;
template class DataStore<Resources::BackgroundImageData>;
template class DataStore<MetaTiles::MetaTileTilesetData>;
template class DataStore<Rooms::RoomData>;

// ProjectSettingsData
// ===================

void ProjectSettingsData::store(std::shared_ptr<const Scripting::GameStateData>&& gameState)
{
    return data.write([&](auto& d) { d.gameState = gameState; });
}

void ProjectSettingsData::store(std::shared_ptr<const Scripting::BytecodeMapping>&& bytecode)
{
    return data.write([&](auto& d) { d.bytecode = bytecode; });
}

void ProjectSettingsData::store(std::shared_ptr<const MetaSprite::ActionPointMapping>&& actionPointMapping)
{
    return data.write([&](auto& d) { d.actionPointMapping = actionPointMapping; });
}

void ProjectSettingsData::store(std::shared_ptr<const MetaTiles::InteractiveTilesData>&& interactiveTiles)
{
    return data.write([&](auto& d) { d.interactiveTiles = interactiveTiles; });
}

void ProjectSettingsData::store(std::shared_ptr<const Resources::CompiledScenesData>&& scenes)
{
    return data.write([&](auto& d) { d.scenes = scenes; });
}

void ProjectSettingsData::store(std::shared_ptr<const Entity::CompiledEntityRomData>&& entityRomData)
{
    return data.write([&](auto& d) { d.entityRomData = entityRomData; });
}

std::shared_ptr<const Scripting::GameStateData> ProjectSettingsData::gameState() const
{
    return data.read_and_return_const_shared_ptr<Scripting::GameStateData>([](const auto& d) { return d.gameState; });
}

std::shared_ptr<const Scripting::BytecodeMapping> ProjectSettingsData::bytecodeData() const
{
    return data.read_and_return_const_shared_ptr<Scripting::BytecodeMapping>([](const auto& d) { return d.bytecode; });
}

std::shared_ptr<const MetaSprite::ActionPointMapping> ProjectSettingsData::actionPointMapping() const
{
    return data.read_and_return_const_shared_ptr<MetaSprite::ActionPointMapping>([](const auto& d) { return d.actionPointMapping; });
}

std::shared_ptr<const MetaTiles::InteractiveTilesData> ProjectSettingsData::interactiveTiles() const
{
    return data.read_and_return_const_shared_ptr<MetaTiles::InteractiveTilesData>([](const auto& d) { return d.interactiveTiles; });
}

std::shared_ptr<const Resources::CompiledScenesData> ProjectSettingsData::scenes() const
{
    return data.read_and_return_const_shared_ptr<Resources::CompiledScenesData>([](const auto& d) { return d.scenes; });
}

std::shared_ptr<const Entity::CompiledEntityRomData> ProjectSettingsData::entityRomData() const
{
    return data.read_and_return_const_shared_ptr<Entity::CompiledEntityRomData>([](const auto& d) { return d.entityRomData; });
}

}
