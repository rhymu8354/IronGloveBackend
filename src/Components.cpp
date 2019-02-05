#include "Components.hpp"

#include <vector>

struct Components::Impl {
    int nextEntityId = 1;
    std::vector< Position > positions;
    std::vector< Tile > tiles;
};

Components::~Components() = default;

Components::Components()
    : impl_(new Impl())
{
}

auto Components::GetComponentsOfType(Type type) -> ComponentList {
    ComponentList list;
    switch (type) {
        case Type::Position: {
            list.first = impl_->positions.data();
            list.n = impl_->positions.size();
        } break;

        case Type::Tile: {
            list.first = impl_->tiles.data();
            list.n = impl_->tiles.size();
        } break;

        default: {
        }
    }
    return list;
}

Component* Components::CreateComponentOfType(Type type, int entityId) {
    Component* component;
    switch (type) {
        case Type::Position: {
            const auto i = impl_->positions.size();
            impl_->positions.resize(i + 1);
            component = &impl_->positions[i];
        } break;

        case Type::Tile: {
            const auto i = impl_->tiles.size();
            impl_->tiles.resize(i + 1);
            component = &impl_->tiles[i];
        } break;

        default: {
            return nullptr;
        }
    }
    component->entityId = entityId;
    return component;
}

Component* Components::GetEntityComponentOfType(Type type, int entityId) {
    switch (type) {
        case Type::Position: {
            const auto n = impl_->positions.size();
            for (size_t i = 0; i < n; ++i) {
                if (impl_->positions[i].entityId == entityId) {
                    return &impl_->positions[i];
                }
            }
        } break;

        case Type::Tile: {
            const auto n = impl_->tiles.size();
            for (size_t i = 0; i < n; ++i) {
                if (impl_->tiles[i].entityId == entityId) {
                    return &impl_->tiles[i];
                }
            }
        } break;

        default: {
        }
    }
    return nullptr;
}

int Components::CreateEntity() {
    return impl_->nextEntityId++;
}
