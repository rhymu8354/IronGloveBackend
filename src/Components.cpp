#include "Components.hpp"

#include <functional>
#include <map>
#include <vector>

struct ComponentType {
    std::function< Components::ComponentList() > list;
    std::function< Component*(int entityId) > create;
    std::function< void(int entityId) > destroy;
    std::function< void(int entityId) > kill;
    std::function< Component*(int entityId) > get;
};

struct Components::Impl {
    int nextEntityId = 1;
    std::map< Type, ComponentType > componentTypes;
};

template< typename T > ComponentType MakeComponentType(
    Components::Type type,
    std::function<
        void(
            std::vector< T >& components,
            int entityId
        )
    > kill = nullptr
) {
    ComponentType componentType;
    const auto components = std::make_shared< std::vector< T > >();
    componentType.list = [components]{
        Components::ComponentList list;
        list.first = components->data();
        list.n = components->size();
        return list;
    };
    componentType.create = [components](int entityId){
        Component* component;
        const auto i = components->size();
        components->resize(i + 1);
        component = &(*components)[i];
        component->entityId = entityId;
        return component;
    };
    componentType.destroy = [components](int entityId){
        auto componentsEntry = components->begin();
        while (componentsEntry != components->end()) {
            if (componentsEntry->entityId == entityId) {
                componentsEntry = components->erase(componentsEntry);
                break;
            } else {
                ++componentsEntry;
            }
        }
    };
    if (kill == nullptr) {
        componentType.kill = componentType.destroy;
    } else {
        componentType.kill = [components, kill](int entityId) {
            kill(*components, entityId);
        };
    }
    componentType.get = [components](int entityId){
        const auto n = components->size();
        for (size_t i = 0; i < n; ++i) {
            if ((*components)[i].entityId == entityId) {
                return &(*components)[i];
            }
        }
        return (T*)nullptr;
    };
    return componentType;
}

Components::~Components() = default;

Components::Components()
    : impl_(new Impl())
{
    impl_->componentTypes[Type::Collider] = MakeComponentType< Collider >(Type::Collider);
    impl_->componentTypes[Type::Generator] = MakeComponentType< Generator >(Type::Generator);
    impl_->componentTypes[Type::Health] = MakeComponentType< Health >(
        Type::Health,
        [](
            std::vector< Health >& components,
            int entityId
        ){
        }
    );
    impl_->componentTypes[Type::Hero] = MakeComponentType< Hero >(
        Type::Hero,
        [](
            std::vector< Hero >& components,
            int entityId
        ){
        }
    );
    impl_->componentTypes[Type::Input] = MakeComponentType< Input >(Type::Input);
    impl_->componentTypes[Type::Monster] = MakeComponentType< Monster >(Type::Monster);
    impl_->componentTypes[Type::Pickup] = MakeComponentType< Pickup >(Type::Pickup);
    impl_->componentTypes[Type::Position] = MakeComponentType< Position >(Type::Position);
    impl_->componentTypes[Type::Reward] = MakeComponentType< Reward >(Type::Reward);
    impl_->componentTypes[Type::Tile] = MakeComponentType< Tile >(
        Type::Tile,
        [](
            std::vector< Tile >& components,
            int entityId
        ){
            for (auto& component: components) {
                if (component.entityId == entityId) {
                    component.destroyed = true;
                    break;
                }
            }
        }
    );
    impl_->componentTypes[Type::Weapon] = MakeComponentType< Weapon >(Type::Weapon);
}

auto Components::GetComponentsOfType(Type type) -> ComponentList {
    return impl_->componentTypes[type].list();
}

Component* Components::CreateComponentOfType(Type type, int entityId) {
    return impl_->componentTypes[type].create(entityId);
}

Component* Components::GetEntityComponentOfType(Type type, int entityId) {
    return impl_->componentTypes[type].get(entityId);
}

int Components::CreateEntity() {
    return impl_->nextEntityId++;
}

void Components::KillEntity(int entityId) {
    for (const auto& componentType: impl_->componentTypes) {
        componentType.second.kill(entityId);
    }
}

void Components::DestroyEntityComponentOfType(Type type, int entityId) {
    return impl_->componentTypes[type].destroy(entityId);
}

bool Components::IsObstacleInTheWay(int x, int y, int mask) {
    const auto collidersInfo = GetComponentsOfType(Type::Collider);
    for (size_t i = 0; i < collidersInfo.n; ++i) {
        const auto& collider = ((Collider*)collidersInfo.first)[i];
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
    const auto collidersInfo = GetComponentsOfType(Type::Collider);
    for (size_t i = 0; i < collidersInfo.n; ++i) {
        auto& collider = ((Collider*)collidersInfo.first)[i];
        const auto position = (Position*)GetEntityComponentOfType(Components::Type::Position, collider.entityId);
        if (position == nullptr) {
            continue;
        }
        if (
            (position->x == x)
            && (position->y == y)
        ) {
            return &collider;
        }
    }
    return nullptr;
}
