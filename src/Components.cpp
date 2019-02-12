#include "Components.hpp"

#include <vector>

struct Components::Impl {
    int nextEntityId = 1;
    std::vector< Collider > colliders;
    std::vector< Generator > generators;
    std::vector< Health > healths;
    std::vector< Hero > heroes;
    std::vector< Input > inputs;
    std::vector< Monster > monsters;
    std::vector< Pickup > pickups;
    std::vector< Position > positions;
    std::vector< Reward > rewards;
    std::vector< Tile > tiles;
    std::vector< Weapon > weapons;
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

        case Type::Generator: {
            list.first = impl_->generators.data();
            list.n = impl_->generators.size();
        } break;

        case Type::Health: {
            list.first = impl_->healths.data();
            list.n = impl_->healths.size();
        } break;

        case Type::Hero: {
            list.first = impl_->heroes.data();
            list.n = impl_->heroes.size();
        } break;

        case Type::Input: {
            list.first = impl_->inputs.data();
            list.n = impl_->inputs.size();
        } break;

        case Type::Monster: {
            list.first = impl_->monsters.data();
            list.n = impl_->monsters.size();
        } break;

        case Type::Pickup: {
            list.first = impl_->pickups.data();
            list.n = impl_->pickups.size();
        } break;

        case Type::Position: {
            list.first = impl_->positions.data();
            list.n = impl_->positions.size();
        } break;

        case Type::Reward: {
            list.first = impl_->rewards.data();
            list.n = impl_->rewards.size();
        } break;

        case Type::Tile: {
            list.first = impl_->tiles.data();
            list.n = impl_->tiles.size();
        } break;

        case Type::Weapon: {
            list.first = impl_->weapons.data();
            list.n = impl_->weapons.size();
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

        case Type::Generator: {
            const auto i = impl_->generators.size();
            impl_->generators.resize(i + 1);
            component = &impl_->generators[i];
        } break;

        case Type::Health: {
            const auto i = impl_->healths.size();
            impl_->healths.resize(i + 1);
            component = &impl_->healths[i];
        } break;

        case Type::Hero: {
            const auto i = impl_->heroes.size();
            impl_->heroes.resize(i + 1);
            component = &impl_->heroes[i];
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

        case Type::Pickup: {
            const auto i = impl_->pickups.size();
            impl_->pickups.resize(i + 1);
            component = &impl_->pickups[i];
        } break;

        case Type::Position: {
            const auto i = impl_->positions.size();
            impl_->positions.resize(i + 1);
            component = &impl_->positions[i];
        } break;

        case Type::Reward: {
            const auto i = impl_->rewards.size();
            impl_->rewards.resize(i + 1);
            component = &impl_->rewards[i];
        } break;

        case Type::Tile: {
            const auto i = impl_->tiles.size();
            impl_->tiles.resize(i + 1);
            component = &impl_->tiles[i];
        } break;

        case Type::Weapon: {
            const auto i = impl_->weapons.size();
            impl_->weapons.resize(i + 1);
            component = &impl_->weapons[i];
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

        case Type::Generator: {
            const auto n = impl_->generators.size();
            for (size_t i = 0; i < n; ++i) {
                if (impl_->generators[i].entityId == entityId) {
                    return &impl_->generators[i];
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

        case Type::Hero: {
            const auto n = impl_->heroes.size();
            for (size_t i = 0; i < n; ++i) {
                if (impl_->heroes[i].entityId == entityId) {
                    return &impl_->heroes[i];
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

        case Type::Pickup: {
            const auto n = impl_->pickups.size();
            for (size_t i = 0; i < n; ++i) {
                if (impl_->pickups[i].entityId == entityId) {
                    return &impl_->pickups[i];
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

        case Type::Reward: {
            const auto n = impl_->rewards.size();
            for (size_t i = 0; i < n; ++i) {
                if (impl_->rewards[i].entityId == entityId) {
                    return &impl_->rewards[i];
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

        case Type::Weapon: {
            const auto n = impl_->weapons.size();
            for (size_t i = 0; i < n; ++i) {
                if (impl_->weapons[i].entityId == entityId) {
                    return &impl_->weapons[i];
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

void Components::DestroyEntity(int entityId) {
    DestroyEntityComponentOfType(Type::Collider, entityId);
    DestroyEntityComponentOfType(Type::Generator, entityId);
    DestroyEntityComponentOfType(Type::Input, entityId);
    DestroyEntityComponentOfType(Type::Monster, entityId);
    DestroyEntityComponentOfType(Type::Pickup, entityId);
    DestroyEntityComponentOfType(Type::Position, entityId);
    DestroyEntityComponentOfType(Type::Reward, entityId);
    DestroyEntityComponentOfType(Type::Weapon, entityId);
    const auto tile = (Tile*)GetEntityComponentOfType(Components::Type::Tile, entityId);
    if (tile != nullptr) {
        tile->destroyed = true;
    }
}

void Components::DestroyEntityComponentOfType(Type type, int entityId) {
    switch (type) {
        case Type::Collider: {
            auto collidersEntry = impl_->colliders.begin();
            while (collidersEntry != impl_->colliders.end()) {
                if (collidersEntry->entityId == entityId) {
                    collidersEntry = impl_->colliders.erase(collidersEntry);
                    break;
                } else {
                    ++collidersEntry;
                }
            }
        } break;

        case Type::Generator: {
            auto generatorsEntry = impl_->generators.begin();
            while (generatorsEntry != impl_->generators.end()) {
                if (generatorsEntry->entityId == entityId) {
                    generatorsEntry = impl_->generators.erase(generatorsEntry);
                    break;
                } else {
                    ++generatorsEntry;
                }
            }
        } break;

        case Type::Health: {
            auto healthsEntry = impl_->healths.begin();
            while (healthsEntry != impl_->healths.end()) {
                if (healthsEntry->entityId == entityId) {
                    healthsEntry = impl_->healths.erase(healthsEntry);
                    break;
                } else {
                    ++healthsEntry;
                }
            }
        } break;

        case Type::Hero: {
            auto heroesEntry = impl_->heroes.begin();
            while (heroesEntry != impl_->heroes.end()) {
                if (heroesEntry->entityId == entityId) {
                    heroesEntry = impl_->heroes.erase(heroesEntry);
                    break;
                } else {
                    ++heroesEntry;
                }
            }
        } break;

        case Type::Input: {
            auto inputsEntry = impl_->inputs.begin();
            while (inputsEntry != impl_->inputs.end()) {
                if (inputsEntry->entityId == entityId) {
                    inputsEntry = impl_->inputs.erase(inputsEntry);
                    break;
                } else {
                    ++inputsEntry;
                }
            }
        } break;

        case Type::Monster: {
            auto monstersEntry = impl_->monsters.begin();
            while (monstersEntry != impl_->monsters.end()) {
                if (monstersEntry->entityId == entityId) {
                    monstersEntry = impl_->monsters.erase(monstersEntry);
                    break;
                } else {
                    ++monstersEntry;
                }
            }
        } break;

        case Type::Pickup: {
            auto pickupsEntry = impl_->pickups.begin();
            while (pickupsEntry != impl_->pickups.end()) {
                if (pickupsEntry->entityId == entityId) {
                    pickupsEntry = impl_->pickups.erase(pickupsEntry);
                    break;
                } else {
                    ++pickupsEntry;
                }
            }
        } break;

        case Type::Position: {
            auto positionsEntry = impl_->positions.begin();
            while (positionsEntry != impl_->positions.end()) {
                if (positionsEntry->entityId == entityId) {
                    positionsEntry = impl_->positions.erase(positionsEntry);
                    break;
                } else {
                    ++positionsEntry;
                }
            }
        } break;

        case Type::Reward: {
            auto rewardsEntry = impl_->rewards.begin();
            while (rewardsEntry != impl_->rewards.end()) {
                if (rewardsEntry->entityId == entityId) {
                    rewardsEntry = impl_->rewards.erase(rewardsEntry);
                    break;
                } else {
                    ++rewardsEntry;
                }
            }
        } break;

        case Type::Tile: {
            auto tilesEntry = impl_->tiles.begin();
            while (tilesEntry != impl_->tiles.end()) {
                if (tilesEntry->entityId == entityId) {
                    tilesEntry = impl_->tiles.erase(tilesEntry);
                    break;
                } else {
                    ++tilesEntry;
                }
            }
        } break;

        case Type::Weapon: {
            auto weaponsEntry = impl_->weapons.begin();
            while (weaponsEntry != impl_->weapons.end()) {
                if (weaponsEntry->entityId == entityId) {
                    weaponsEntry = impl_->weapons.erase(weaponsEntry);
                    break;
                } else {
                    ++weaponsEntry;
                }
            }
        } break;

        default: {
        }
    }
}

bool Components::IsObstacleInTheWay(int x, int y, int mask) {
    for (size_t i = 0; i < impl_->colliders.size(); ++i) {
        const auto& collider = impl_->colliders[i];
        const auto position = (Position*)GetEntityComponentOfType(Components::Type::Position, collider.entityId);
        if (position == nullptr) {
            continue;
        }
        if (
            (position->x == x)
            && (position->y == y)
            && ((mask & collider.mask) != 0)
        ) {
            return true;
        }
    }
    return false;
}

Collider* Components::GetColliderAt(int x, int y) {
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
            return &impl_->colliders[i];
        }
    }
    return nullptr;
}
