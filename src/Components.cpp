#include "Components.hpp"

#include <algorithm>
#include <functional>
#include <map>
#include <vector>

struct ComponentType {
    std::function< Components::ComponentList() > list;
    std::function< Component*(int entityId) > create;
    std::function< void(int entityId) > destroy;
    std::function< void(int entityId) > kill;
    std::function< Component*(int entityId) > get;
    std::function< size_t(int entityId) > getLuaIndex;
};

struct Components::Impl {
    int nextEntityId = 1;
    std::map< Type, ComponentType > componentTypes;
    std::shared_ptr< SystemAbstractions::DiagnosticsSender > diagnosticsSender;

    /**
     * This is a Lua function registered as the __gc
     * object metamethod of the "components" class.
     *
     * @param[in] lua
     *     This points to the Lua interpreter instance.
     */
    static int Finalizer(lua_State* lua) {
        auto self = (std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "components");
        self->~shared_ptr< Impl >();
        return 0;
    }

    /**
     * This handles Lua index metamethod calls for
     * the global components object.
     *
     * @param[in] lua
     *     This points to the state of the Lua interpreter.
     *
     * @return
     *     The number of return values that have been pushed onto the
     *     Lua stack by the function as return values of the function
     *     is returned.
     */
    static int Index(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "components");
        const std::string fieldName = luaL_checkstring(lua, 2);
        if (fieldName == "heroes") {
            auto heroes = (std::shared_ptr< Impl >*)lua_newuserdata(lua, sizeof(std::shared_ptr< Impl >));
            new (heroes) std::shared_ptr< Impl >();
            *heroes = self;
            luaL_setmetatable(lua, "heroes");
        // } else if (fieldName == "status") {
        //     (void)lua_pushstring(lua, self->status_.c_str());
        // } else if (fieldName == "progress") {
        //     (void)lua_pushnumber(lua, (lua_Number)self->progress_);
        } else {
            luaL_getmetatable(lua, "components");
            lua_insert(lua, 2);
            lua_rawget(lua, 2);
        }
        return 1;
    }

    /**
     * This handles Lua newindex metamethod calls for
     * the global components object.
     *
     * @param[in] lua
     *     This points to the state of the Lua interpreter.
     *
     * @return
     *     The number of return values that have been pushed onto the
     *     Lua stack by the function as return values of the function
     *     is returned.
     */
    static int NewIndex(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "components");
        const std::string fieldName = luaL_checkstring(lua, 2);
        // if (fieldName == "title") {
        //     size_t length;
        //     auto text = luaL_checklstring(lua, 3, &length);
        //     self->title_ = std::string(text, length);
        //     self->user_->ScriptHostUserSetTitle(self->title_);
        // } else if (fieldName == "status") {
        //     size_t length;
        //     auto text = luaL_checklstring(lua, 3, &length);
        //     self->status_ = std::string(text, length);
        //     self->user_->ScriptHostUserSetStatus(self->status_);
        // } else if (fieldName == "progress") {
        //     self->progress_ = (double)luaL_checknumber(lua, 3);
        //     self->user_->ScriptHostUserSetProgress(self->progress_);
        // } else {
            luaL_getmetatable(lua, "components");
            lua_insert(lua, 2);
            lua_rawset(lua, 2);
        // }
        return 0;
    }

    /**
     * This handles Lua tostring metamethod calls for
     * the global components object.
     *
     * @param[in] lua
     *     This points to the state of the Lua interpreter.
     *
     * @return
     *     The number of return values that have been pushed onto the
     *     Lua stack by the function as return values of the function
     *     is returned.
     */
    static int ToString(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "components");
        lua_pushstring(lua, "components()");
        return 1;
    }

    static int DiagnosticMessageFromSystems(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "components");
        const auto level = (size_t)std::max((lua_Integer)0, luaL_checkinteger(lua, 2));
        const std::string message = luaL_checkstring(lua, 3);
        self->diagnosticsSender->SendDiagnosticInformationString(
            level,
            std::string("systems: ") + message
        );
        return 0;
    }

    static int GetEntityComponentOfType(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "components");
        const std::string typeName = luaL_checkstring(lua, 2);
        const auto entityId = (int)luaL_checkinteger(lua, 3);
        if (typeName == "Health") {
            auto index = self->componentTypes[Type::Health].getLuaIndex(entityId);
            if (index == 0) {
                lua_pushnil(lua);
            } else {
                self->PushHealth(lua, index);
            }
        } else if (typeName == "Position") {
            auto index = self->componentTypes[Type::Position].getLuaIndex(entityId);
            if (index == 0) {
                lua_pushnil(lua);
            } else {
                self->PushPosition(lua, index);
            }
        } else {
            lua_pushnil(lua);
        }
        return 1;
    }

    static int KillEntity(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "components");
        const auto entityId = (int)luaL_checkinteger(lua, 2);
        for (const auto& componentType: self->componentTypes) {
            componentType.second.kill(entityId);
        }
        return 0;
    }

    // Heroes

    struct ScriptHero {
        size_t index;
        Hero* hero;
    };

    static int HeroesIndex(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "heroes");
        const auto heroIndex = (size_t)std::max((lua_Integer)0, luaL_checkinteger(lua, 2));
        auto heroesInfo = self->componentTypes[Type::Hero].list();
        if (
            (heroIndex == 0)
            || (heroIndex > heroesInfo.n)
        ) {
            lua_pushnil(lua);
        }
        self->PushHero(lua, heroIndex);
        return 1;
    }

    static int HeroesLen(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "heroes");
        auto heroesInfo = self->componentTypes[Type::Hero].list();
        lua_pushinteger(lua, (lua_Integer)heroesInfo.n);
        return 1;
    }

    static int HeroesIterate(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "heroes");
        auto heroesInfo = self->componentTypes[Type::Hero].list();
        luaL_checkany(lua, 3);
        size_t heroIndex;
        if (lua_isnil(lua, 3)) {
            heroIndex = 1;
        } else {
            auto lastHero = (ScriptHero*)luaL_checkudata(lua, 3, "hero");
            heroIndex = lastHero->index + 1;
        }
        if (heroIndex > heroesInfo.n) {
            lua_pushnil(lua);
        } else {
            self->PushHero(lua, heroIndex);
        }
        return 1;
    }

    void PushHero(lua_State* lua, size_t heroIndex) {
        auto heroesInfo = componentTypes[Type::Hero].list();
        auto& hero = ((Hero*)heroesInfo.first)[heroIndex - 1];
        auto scriptHero = (ScriptHero*)lua_newuserdata(lua, sizeof(ScriptHero));
        scriptHero->index = heroIndex;
        scriptHero->hero = &hero;
        luaL_setmetatable(lua, "hero");
    }

    static int HeroIndex(lua_State* lua) {
        auto self = (ScriptHero*)luaL_checkudata(lua, 1, "hero");
        const std::string fieldName = luaL_checkstring(lua, 2);
        if (fieldName == "score") {
            lua_pushinteger(lua, self->hero->score);
        } else if (fieldName == "entityId") {
            lua_pushinteger(lua, self->hero->entityId);
        } else {
            lua_pushnil(lua);
        }
        return 1;
    }

    // Health

    struct ScriptHealth {
        size_t index;
        Health* health;
    };

    static int HealthsIndex(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "healths");
        const auto healthIndex = (size_t)std::max((lua_Integer)0, luaL_checkinteger(lua, 2));
        auto healthsInfo = self->componentTypes[Type::Health].list();
        if (
            (healthIndex == 0)
            || (healthIndex > healthsInfo.n)
        ) {
            lua_pushnil(lua);
        }
        self->PushHealth(lua, healthIndex);
        return 1;
    }

    static int HealthsLen(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "healths");
        auto healthsInfo = self->componentTypes[Type::Health].list();
        lua_pushinteger(lua, (lua_Integer)healthsInfo.n);
        return 1;
    }

    static int HealthsIterate(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "healths");
        auto healthsInfo = self->componentTypes[Type::Health].list();
        luaL_checkany(lua, 3);
        size_t healthIndex;
        if (lua_isnil(lua, 3)) {
            healthIndex = 1;
        } else {
            auto lastHealth = (ScriptHealth*)luaL_checkudata(lua, 3, "health");
            healthIndex = lastHealth->index + 1;
        }
        if (healthIndex > healthsInfo.n) {
            lua_pushnil(lua);
        } else {
            self->PushHealth(lua, healthIndex);
        }
        return 1;
    }

    void PushHealth(lua_State* lua, size_t healthIndex) {
        auto healthsInfo = componentTypes[Type::Health].list();
        auto& health = ((Health*)healthsInfo.first)[healthIndex - 1];
        auto scriptHealth = (ScriptHealth*)lua_newuserdata(lua, sizeof(ScriptHealth));
        scriptHealth->index = healthIndex;
        scriptHealth->health = &health;
        luaL_setmetatable(lua, "health");
    }

    static int HealthIndex(lua_State* lua) {
        auto self = (ScriptHealth*)luaL_checkudata(lua, 1, "health");
        const std::string fieldName = luaL_checkstring(lua, 2);
        if (fieldName == "hp") {
            lua_pushinteger(lua, self->health->hp);
        } else if (fieldName == "entityId") {
            lua_pushinteger(lua, self->health->entityId);
        } else {
            lua_pushnil(lua);
        }
        return 1;
    }

    static int HealthNewIndex(lua_State* lua) {
        auto self = (ScriptHealth*)luaL_checkudata(lua, 1, "health");
        const std::string fieldName = luaL_checkstring(lua, 2);
        if (fieldName == "hp") {
            const auto hp = (int)luaL_checkinteger(lua, 3);
            self->health->hp = hp;
        }
        return 0;
    }

    // Position

    struct ScriptPosition {
        size_t index;
        Position* position;
    };

    static int PositionsIndex(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "positions");
        const auto positionIndex = (size_t)std::max((lua_Integer)0, luaL_checkinteger(lua, 2));
        auto positionsInfo = self->componentTypes[Type::Position].list();
        if (
            (positionIndex == 0)
            || (positionIndex > positionsInfo.n)
        ) {
            lua_pushnil(lua);
        }
        self->PushPosition(lua, positionIndex);
        return 1;
    }

    static int PositionsLen(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "positions");
        auto positionsInfo = self->componentTypes[Type::Position].list();
        lua_pushinteger(lua, (lua_Integer)positionsInfo.n);
        return 1;
    }

    static int PositionsIterate(lua_State* lua) {
        auto self = *(std::shared_ptr< Impl >*)luaL_checkudata(lua, 1, "positions");
        auto positionsInfo = self->componentTypes[Type::Position].list();
        luaL_checkany(lua, 3);
        size_t positionIndex;
        if (lua_isnil(lua, 3)) {
            positionIndex = 1;
        } else {
            auto lastPosition = (ScriptPosition*)luaL_checkudata(lua, 3, "position");
            positionIndex = lastPosition->index + 1;
        }
        if (positionIndex > positionsInfo.n) {
            lua_pushnil(lua);
        } else {
            self->PushPosition(lua, positionIndex);
        }
        return 1;
    }

    void PushPosition(lua_State* lua, size_t positionIndex) {
        auto positionsInfo = componentTypes[Type::Position].list();
        auto& position = ((Position*)positionsInfo.first)[positionIndex - 1];
        auto scriptPosition = (ScriptPosition*)lua_newuserdata(lua, sizeof(ScriptPosition));
        scriptPosition->index = positionIndex;
        scriptPosition->position = &position;
        luaL_setmetatable(lua, "position");
    }

    static int PositionIndex(lua_State* lua) {
        auto self = (ScriptPosition*)luaL_checkudata(lua, 1, "position");
        const std::string fieldName = luaL_checkstring(lua, 2);
        if (fieldName == "x") {
            lua_pushinteger(lua, self->position->x);
        } else if (fieldName == "y") {
            lua_pushinteger(lua, self->position->y);
        } else if (fieldName == "entityId") {
            lua_pushinteger(lua, self->position->entityId);
        } else {
            lua_pushnil(lua);
        }
        return 1;
    }
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
    componentType.getLuaIndex = [components](int entityId){
        const auto n = components->size();
        for (size_t i = 0; i < n; ++i) {
            if ((*components)[i].entityId == entityId) {
                return i + 1;
            }
        }
        return (size_t)0;
    };
    return componentType;
}

Components::~Components() = default;

Components::Components()
    : impl_(std::make_shared< Impl >())
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

void Components::SetDiagnosticsSender(
    std::shared_ptr< SystemAbstractions::DiagnosticsSender > diagnosticsSender
) {
    impl_->diagnosticsSender = diagnosticsSender;
}

void Components::LinkLua(lua_State* lua) {
    // Components
    luaL_newmetatable(lua, "components");
    lua_pushstring(lua, "__gc");
    lua_pushcfunction(lua, Impl::Finalizer);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__index");
    lua_pushcfunction(lua, Impl::Index);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__newindex");
    lua_pushcfunction(lua, Impl::NewIndex);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__tostring");
    lua_pushcfunction(lua, Impl::ToString);
    lua_settable(lua, -3);
    lua_pushstring(lua, "DiagnosticMessage");
    lua_pushcfunction(lua, Impl::DiagnosticMessageFromSystems);
    lua_settable(lua, -3);
    lua_pushstring(lua, "GetEntityComponentOfType");
    lua_pushcfunction(lua, Impl::GetEntityComponentOfType);
    lua_settable(lua, -3);
    lua_pushstring(lua, "KillEntity");
    lua_pushcfunction(lua, Impl::KillEntity);
    lua_settable(lua, -3);
    lua_pop(lua, 1);

    // Heroes
    luaL_newmetatable(lua, "heroes");
    lua_pushstring(lua, "__index");
    lua_pushcfunction(lua, Impl::HeroesIndex);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__len");
    lua_pushcfunction(lua, Impl::HeroesLen);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__call");
    lua_pushcfunction(lua, Impl::HeroesIterate);
    lua_settable(lua, -3);
    lua_pop(lua, 1);

    // Hero
    luaL_newmetatable(lua, "hero");
    lua_pushstring(lua, "__index");
    lua_pushcfunction(lua, Impl::HeroIndex);
    lua_settable(lua, -3);
    lua_pop(lua, 1);

    // Healths
    luaL_newmetatable(lua, "healths");
    lua_pushstring(lua, "__index");
    lua_pushcfunction(lua, Impl::HealthsIndex);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__len");
    lua_pushcfunction(lua, Impl::HealthsLen);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__call");
    lua_pushcfunction(lua, Impl::HealthsIterate);
    lua_settable(lua, -3);
    lua_pop(lua, 1);

    // Health
    luaL_newmetatable(lua, "health");
    lua_pushstring(lua, "__index");
    lua_pushcfunction(lua, Impl::HealthIndex);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__newindex");
    lua_pushcfunction(lua, Impl::HealthNewIndex);
    lua_settable(lua, -3);
    lua_pop(lua, 1);

    // Positions
    luaL_newmetatable(lua, "positions");
    lua_pushstring(lua, "__index");
    lua_pushcfunction(lua, Impl::PositionsIndex);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__len");
    lua_pushcfunction(lua, Impl::PositionsLen);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__call");
    lua_pushcfunction(lua, Impl::PositionsIterate);
    lua_settable(lua, -3);
    lua_pop(lua, 1);

    // Position
    luaL_newmetatable(lua, "position");
    lua_pushstring(lua, "__index");
    lua_pushcfunction(lua, Impl::PositionIndex);
    lua_settable(lua, -3);
    lua_pop(lua, 1);
}

void Components::PushLua(lua_State* lua) {
    auto self = (std::shared_ptr< Impl >*)lua_newuserdata(lua, sizeof(std::shared_ptr< Impl >));
    new (self) std::shared_ptr< Impl >();
    *self = impl_;
    luaL_setmetatable(lua, "components");
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
