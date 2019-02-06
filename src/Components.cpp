#include "Components.hpp"

#include <vector>

struct Components::Impl {
    int nextEntityId = 1;
    std::vector< Collider > colliders;
    std::vector< Health > healths;
    std::vector< Input > inputs;
    std::vector< Monster > monsters;
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
        case Type::Collider: {
            list.first = impl_->colliders.data();
            list.n = impl_->colliders.size();
        } break;

        case Type::Health: {
            list.first = impl_->healths.data();
            list.n = impl_->healths.size();
        } break;

        case Type::Input: {
            list.first = impl_->inputs.data();
            list.n = impl_->inputs.size();
        } break;

        case Type::Monster: {
            list.first = impl_->monsters.data();
            list.n = impl_->monsters.size();
        } break;

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
        case Type::Collider: {
            const auto i = impl_->colliders.size();
            impl_->colliders.resize(i + 1);
            component = &impl_->colliders[i];
        } break;

        case Type::Health: {
            const auto i = impl_->healths.size();
            impl_->healths.resize(i + 1);
            component = &impl_->healths[i];
        } break;

        case Type::Input: {
            const auto i = impl_->inputs.size();
            impl_->inputs.resize(i + 1);
            component = &impl_->inputs[i];
        } break;

        case Type::Monster: {
            const auto i = impl_->monsters.size();
            impl_->monsters.resize(i + 1);
            component = &impl_->monsters[i];
        } break;

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
        case Type::Collider: {
            const auto n = impl_->colliders.size();
            for (size_t i = 0; i < n; ++i) {
                if (impl_->colliders[i].entityId == entityId) {
                    return &impl_->colliders[i];
                }
            }
        } break;

        case Type::Health: {
            const auto n = impl_->healths.size();
            for (size_t i = 0; i < n; ++i) {
                if (impl_->healths[i].entityId == entityId) {
                    return &impl_->healths[i];
                }
            }
        } break;

        case Type::Input: {
            const auto n = impl_->inputs.size();
            for (size_t i = 0; i < n; ++i) {
                if (impl_->inputs[i].entityId == entityId) {
                    return &impl_->inputs[i];
                }
            }
        } break;

        case Type::Monster: {
            const auto n = impl_->monsters.size();
            for (size_t i = 0; i < n; ++i) {
                if (impl_->monsters[i].entityId == entityId) {
                    return &impl_->monsters[i];
                }
            }
        } break;

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

bool Components::IsObstacleInTheWay(int x, int y) {
    for (size_t i = 0; i < impl_->colliders.size(); ++i) {
        const auto& collider = impl_->colliders[i];
        const auto position = (Position*)GetEntityComponentOfType(Components::Type::Position, collider.entityId);
        if (position == nullptr) {
            continue;
        }
        if (
            (position->x == x)
            && (position->y == y)
        ) {
            return true;
        }
    }
    return false;
}
