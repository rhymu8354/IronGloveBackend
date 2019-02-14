#include "Components.hpp"

#include <algorithm>
#include <functional>
#include <set>
#include <map>
#include <vector>

struct ComponentType {
    std::function< Components::ComponentList() > list;
    std::function< Component*(int entityId) > create;
    std::function< void(int entityId) > destroy;
    std::function< void(int entityId) > kill;
    std::function< Component*(int entityId) > get;
    std::function< size_t(int entityId) > getLuaIndex;
    std::function< void(lua_State* lua, size_t index) > push;
};

template< typename T > using LuaPropertyMap = std::map< std::string, std::function< void(lua_State* lua, T* component) > >;

struct Components::Impl {
    int nextEntityId = 1;
    std::map< Type, ComponentType > componentTypes;
    std::set< std::string > collectionTypeNames;
    std::map< std::string, Type > componentTypeNames;
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
        const auto collectionTypeNamesEntry = self->collectionTypeNames.find(fieldName);
        if (collectionTypeNamesEntry == self->collectionTypeNames.end()) {
            luaL_getmetatable(lua, "components");
            lua_insert(lua, 2);
            lua_rawget(lua, 2);
        } else {
            auto collectionWrapper = (std::shared_ptr< Impl >*)lua_newuserdata(lua, sizeof(std::shared_ptr< Impl >));
            new (collectionWrapper) std::shared_ptr< Impl >();
            *collectionWrapper = self;
            luaL_setmetatable(lua, fieldName.c_str());
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
        luaL_getmetatable(lua, "components");
        lua_insert(lua, 2);
        lua_rawset(lua, 2);
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
        const auto componentTypeNamesEntry = self->componentTypeNames.find(typeName);
        if (componentTypeNamesEntry == self->componentTypeNames.end()) {
            lua_pushnil(lua);
        } else {
            const auto type = componentTypeNamesEntry->second;
            auto index = self->componentTypes[type].getLuaIndex(entityId);
            if (index == 0) {
                lua_pushnil(lua);
            } else {
                self->componentTypes[type].push(lua, index);
            }
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

    template< typename T > void MakeComponentType(
        Components::Type type,
        lua_State* lua,
        const std::string& collectionWrapperName,
        const std::string& componentWrapperName,
        std::function< int(lua_State* lua) >& collectionIndex,
        lua_CFunction collectionIndexThunk,
        std::function< int(lua_State* lua) >& collectionLen,
        lua_CFunction collectionLenThunk,
        std::function< int(lua_State* lua) >& collectionIterate,
        lua_CFunction collectionIterateThunk,
        std::function< int(lua_State* lua) >& componentIndex,
        lua_CFunction componentIndexThunk,
        std::function< int(lua_State* lua) >& componentNewIndex,
        lua_CFunction componentNewIndexThunk,
        std::shared_ptr< LuaPropertyMap< T > > indexers = std::make_shared< LuaPropertyMap< T > >(),
        std::shared_ptr< LuaPropertyMap< T > > newIndexers = std::make_shared< LuaPropertyMap< T > >(),
        std::function<
            void(
                std::vector< T >& components,
                int entityId
            )
        > kill = nullptr
    ) {
        (void)collectionTypeNames.insert(collectionWrapperName);
        componentTypeNames[componentWrapperName] = type;
        struct ScriptComponent {
            size_t index;
            T* component;
        };
        ComponentType componentType;
        const auto components = std::make_shared< std::vector< T > >();
        const auto list = [components]{
            Components::ComponentList list;
            list.first = components->data();
            list.n = components->size();
            return list;
        };
        componentType.list = list;
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
        const auto push = [list, componentWrapperName](lua_State* lua, size_t index) {
            auto componentsInfo = list();
            auto& component = ((T*)componentsInfo.first)[index - 1];
            auto scriptComponent = (ScriptComponent*)lua_newuserdata(lua, sizeof(ScriptComponent));
            scriptComponent->index = index;
            scriptComponent->component = &component;
            luaL_setmetatable(lua, componentWrapperName.c_str());
        };
        componentType.push = push;
        collectionIndex = [list, push, collectionWrapperName, componentWrapperName](lua_State* lua){
            (void)luaL_checkudata(lua, 1, collectionWrapperName.c_str());
            const auto index = (size_t)std::max((lua_Integer)0, luaL_checkinteger(lua, 2));
            auto componentsInfo = list();
            if (
                (index == 0)
                || (index > componentsInfo.n)
            ) {
                lua_pushnil(lua);
            }
            push(lua, index);
            return 1;
        };
        collectionLen = [list, collectionWrapperName](lua_State* lua){
            (void)luaL_checkudata(lua, 1, collectionWrapperName.c_str());
            auto componentsInfo = list();
            lua_pushinteger(lua, (lua_Integer)componentsInfo.n);
            return 1;
        };
        collectionIterate = [list, push, collectionWrapperName, componentWrapperName](lua_State* lua){
            (void)luaL_checkudata(lua, 1, collectionWrapperName.c_str());
            auto componentsInfo = list();
            luaL_checkany(lua, 3);
            size_t index;
            if (lua_isnil(lua, 3)) {
                index = 1;
            } else {
                auto lastComponent = (ScriptComponent*)luaL_checkudata(lua, 3, componentWrapperName.c_str());
                index = lastComponent->index + 1;
            }
            if (index > componentsInfo.n) {
                lua_pushnil(lua);
            } else {
                push(lua, index);
            }
            return 1;
        };
        componentIndex = [componentWrapperName, indexers](lua_State* lua){
            auto self = (ScriptComponent*)luaL_checkudata(lua, 1, componentWrapperName.c_str());
            const std::string fieldName = luaL_checkstring(lua, 2);
            if (fieldName == "entityId") {
                lua_pushinteger(lua, self->component->entityId);
            } else {
                const auto indexersEntry = indexers->find(fieldName);
                if (indexersEntry == indexers->end()) {
                    lua_pushnil(lua);
                } else {
                    indexersEntry->second(lua, self->component);
                }
            }
            return 1;
        };
        componentNewIndex = [componentWrapperName, newIndexers](lua_State* lua){
            auto self = (ScriptComponent*)luaL_checkudata(lua, 1, componentWrapperName.c_str());
            const std::string fieldName = luaL_checkstring(lua, 2);
            const auto newIndexersEntry = newIndexers->find(fieldName);
            if (newIndexersEntry != newIndexers->end()) {
                newIndexersEntry->second(lua, self->component);
            }
            return 0;
        };

        // Collection
        luaL_newmetatable(lua, collectionWrapperName.c_str());
        lua_pushstring(lua, "__index");
        lua_pushcfunction(lua, collectionIndexThunk);
        lua_settable(lua, -3);
        lua_pushstring(lua, "__len");
        lua_pushcfunction(lua, collectionLenThunk);
        lua_settable(lua, -3);
        lua_pushstring(lua, "__call");
        lua_pushcfunction(lua, collectionIterateThunk);
        lua_settable(lua, -3);
        lua_pop(lua, 1);

        // Component
        luaL_newmetatable(lua, componentWrapperName.c_str());
        lua_pushstring(lua, "__index");
        lua_pushcfunction(lua, componentIndexThunk);
        lua_settable(lua, -3);
        lua_pushstring(lua, "__newindex");
        lua_pushcfunction(lua, componentNewIndexThunk);
        lua_settable(lua, -3);
        lua_pop(lua, 1);
        componentTypes[type] = std::move(componentType);
    }
};

#define MAKE_COLLECTION_INDEX_THUNK(type) \
    std::function< int(lua_State* lua) > type##CollectionIndex; \
    int type##CollectionIndexThunk(lua_State* lua) { \
        return type##CollectionIndex(lua); \
    }

#define MAKE_COLLECTION_LEN_THUNK(type) \
    std::function< int(lua_State* lua) > type##CollectionLen; \
    int type##CollectionLenThunk(lua_State* lua) { \
        return type##CollectionLen(lua); \
    }

#define MAKE_COLLECTION_ITERATE_THUNK(type) \
    std::function< int(lua_State* lua) > type##CollectionIterate; \
    int type##CollectionIterateThunk(lua_State* lua) { \
        return type##CollectionIterate(lua); \
    }

#define MAKE_COMPONENT_INDEX_THUNK(type) \
    std::function< int(lua_State* lua) > type##ComponentIndex; \
    int type##ComponentIndexThunk(lua_State* lua) { \
        return type##ComponentIndex(lua); \
    }

#define MAKE_COMPONENT_NEW_INDEX_THUNK(type) \
    std::function< int(lua_State* lua) > type##ComponentNewIndex; \
    int type##ComponentNewIndexThunk(lua_State* lua) { \
        return type##ComponentNewIndex(lua); \
    }

#define MAKE_THUNKS(type) \
    MAKE_COLLECTION_INDEX_THUNK(type) \
    MAKE_COLLECTION_LEN_THUNK(type) \
    MAKE_COLLECTION_ITERATE_THUNK(type) \
    MAKE_COMPONENT_INDEX_THUNK(type) \
    MAKE_COMPONENT_NEW_INDEX_THUNK(type)

#define PASS_THUNKS(type) \
    type##CollectionIndex, type##CollectionIndexThunk, \
    type##CollectionLen, type##CollectionLenThunk, \
    type##CollectionIterate, type##CollectionIterateThunk, \
    type##ComponentIndex, type##ComponentIndexThunk, \
    type##ComponentNewIndex, type##ComponentNewIndexThunk

Components::~Components() = default;

Components::Components()
    : impl_(std::make_shared< Impl >())
{
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
}

MAKE_THUNKS(Collider)
MAKE_THUNKS(Generator)
MAKE_THUNKS(Health)
MAKE_THUNKS(Hero)
MAKE_THUNKS(Input)
MAKE_THUNKS(Monster)
MAKE_THUNKS(Pickup)
MAKE_THUNKS(Position)
MAKE_THUNKS(Reward)
MAKE_THUNKS(Tile)
MAKE_THUNKS(Weapon)

void Components::BuildComponentTypeMap(lua_State* lua) {
    impl_->MakeComponentType< Collider >(
        Type::Collider,
        lua,
        "colliders", "collider",
        PASS_THUNKS(Collider),
        std::make_shared< LuaPropertyMap< Collider > >(
            std::initializer_list< LuaPropertyMap< Collider >::value_type >{
                {"mask", [](lua_State* lua, Collider* component){
                    lua_pushinteger(lua, (lua_Integer)component->mask);
                }},
            }
        ),
        std::make_shared< LuaPropertyMap< Collider > >(
            std::initializer_list< LuaPropertyMap< Collider >::value_type >{
                {"mask", [](lua_State* lua, Collider* component){
                    const auto mask = (int)luaL_checkinteger(lua, 3);
                    component->mask = mask;
                }},
            }
        )
    );
    impl_->MakeComponentType< Generator >(
        Type::Generator,
        lua,
        "generators", "generator",
        PASS_THUNKS(Generator),
        std::make_shared< LuaPropertyMap< Generator > >(
            std::initializer_list< LuaPropertyMap< Generator >::value_type >{
                {"spawnChance", [](lua_State* lua, Generator* component){
                    lua_pushnumber(lua, (lua_Number)component->spawnChance);
                }},
            }
        ),
        std::make_shared< LuaPropertyMap< Generator > >(
            std::initializer_list< LuaPropertyMap< Generator >::value_type >{
                {"spawnChance", [](lua_State* lua, Generator* component){
                    const auto spawnChance = (double)luaL_checknumber(lua, 3);
                    component->spawnChance = spawnChance;
                }},
            }
        )
    );
    impl_->MakeComponentType< Health >(
        Type::Health,
        lua,
        "healths", "health",
        PASS_THUNKS(Health),
        std::make_shared< LuaPropertyMap< Health > >(
            std::initializer_list< LuaPropertyMap< Health >::value_type >{
                {"hp", [](lua_State* lua, Health* component){
                    lua_pushinteger(lua, (lua_Integer)component->hp);
                }},
            }
        ),
        std::make_shared< LuaPropertyMap< Health > >(
            std::initializer_list< LuaPropertyMap< Health >::value_type >{
                {"hp", [](lua_State* lua, Health* component){
                    const auto hp = (int)luaL_checkinteger(lua, 3);
                    component->hp = hp;
                }},
            }
        ),
        [](
            std::vector< Health >& components,
            int entityId
        ){
        }
    );
    impl_->MakeComponentType< Hero >(
        Type::Hero,
        lua,
        "heroes", "hero",
        PASS_THUNKS(Hero),
        std::make_shared< LuaPropertyMap< Hero > >(
            std::initializer_list< LuaPropertyMap< Hero >::value_type >{
                {"score", [](lua_State* lua, Hero* component){
                    lua_pushinteger(lua, (lua_Integer)component->score);
                }},
                {"potions", [](lua_State* lua, Hero* component){
                    lua_pushinteger(lua, (lua_Integer)component->potions);
                }},
            }
        ),
        std::make_shared< LuaPropertyMap< Hero > >(
            std::initializer_list< LuaPropertyMap< Hero >::value_type >{
                {"score", [](lua_State* lua, Hero* component){
                    const auto score = (int)luaL_checkinteger(lua, 3);
                    component->score = score;
                }},
                {"potions", [](lua_State* lua, Hero* component){
                    const auto potions = (int)luaL_checkinteger(lua, 3);
                    component->potions = potions;
                }},
            }
        ),
        [](
            std::vector< Hero >& components,
            int entityId
        ){
        }
    );
    impl_->MakeComponentType< Input >(
        Type::Input,
        lua,
        "inputs", "input",
        PASS_THUNKS(Input),
        std::make_shared< LuaPropertyMap< Input > >(
            std::initializer_list< LuaPropertyMap< Input >::value_type >{
                {"fire", [](lua_State* lua, Input* component){
                    const std::string fireAsString(1, component->fire);
                    lua_pushstring(lua, fireAsString.c_str());
                }},
                {"fireReleased", [](lua_State* lua, Input* component){
                    lua_pushboolean(lua, component->fireReleased ? 1 : 0);
                }},
                {"fireThisTick", [](lua_State* lua, Input* component){
                    lua_pushboolean(lua, component->fireThisTick ? 1 : 0);
                }},
                {"move", [](lua_State* lua, Input* component){
                    const std::string moveAsString(1, component->move);
                    lua_pushstring(lua, moveAsString.c_str());
                }},
                {"moveReleased", [](lua_State* lua, Input* component){
                    lua_pushboolean(lua, component->moveReleased ? 1 : 0);
                }},
                {"moveThisTick", [](lua_State* lua, Input* component){
                    lua_pushboolean(lua, component->moveThisTick ? 1 : 0);
                }},
                {"weaponInFlight", [](lua_State* lua, Input* component){
                    lua_pushboolean(lua, component->weaponInFlight ? 1 : 0);
                }},
                {"moveCooldown", [](lua_State* lua, Input* component){
                    lua_pushinteger(lua, (lua_Integer)component->moveCooldown);
                }},
                {"usePotion", [](lua_State* lua, Input* component){
                    lua_pushboolean(lua, component->usePotion ? 1 : 0);
                }},
            }
        ),
        std::make_shared< LuaPropertyMap< Input > >(
            std::initializer_list< LuaPropertyMap< Input >::value_type >{
                {"fire", [](lua_State* lua, Input* component){
                    const std::string fire = luaL_checkstring(lua, 3);
                    if (fire.empty()) {
                        component->fire = 0;
                    } else {
                        component->fire = fire[0];
                    }
                }},
                {"fireReleased", [](lua_State* lua, Input* component){
                    luaL_checkany(lua, 3);
                    component->fireReleased = (lua_toboolean(lua, 3) != 0);
                }},
                {"fireThisTick", [](lua_State* lua, Input* component){
                    luaL_checkany(lua, 3);
                    component->fireThisTick = (lua_toboolean(lua, 3) != 0);
                }},
                {"move", [](lua_State* lua, Input* component){
                    const std::string move = luaL_checkstring(lua, 3);
                    if (move.empty()) {
                        component->move = 0;
                    } else {
                        component->move = move[0];
                    }
                }},
                {"moveReleased", [](lua_State* lua, Input* component){
                    luaL_checkany(lua, 3);
                    component->moveReleased = (lua_toboolean(lua, 3) != 0);
                }},
                {"moveThisTick", [](lua_State* lua, Input* component){
                    luaL_checkany(lua, 3);
                    component->moveThisTick = (lua_toboolean(lua, 3) != 0);
                }},
                {"weaponInFlight", [](lua_State* lua, Input* component){
                    luaL_checkany(lua, 3);
                    component->weaponInFlight = (lua_toboolean(lua, 3) != 0);
                }},
                {"moveCooldown", [](lua_State* lua, Input* component){
                    const auto moveCooldown = (int)luaL_checkinteger(lua, 3);
                    component->moveCooldown = moveCooldown;
                }},
                {"usePotion", [](lua_State* lua, Input* component){
                    luaL_checkany(lua, 3);
                    component->usePotion = (lua_toboolean(lua, 3) != 0);
                }},
            }
        )
    );
    impl_->MakeComponentType< Monster >(
        Type::Monster,
        lua,
        "monsters", "monster",
        PASS_THUNKS(Monster)
    );
    impl_->MakeComponentType< Pickup >(
        Type::Pickup,
        lua,
        "pickups", "pickup",
        PASS_THUNKS(Pickup),
        std::make_shared< LuaPropertyMap< Pickup > >(
            std::initializer_list< LuaPropertyMap< Pickup >::value_type >{
                {"type", [](lua_State* lua, Pickup* component){
                    std::string typeAsString;
                    switch (component->type) {
                        case Pickup::Type::Food: {
                            typeAsString = "Food";
                        } break;
                        case Pickup::Type::Potion: {
                            typeAsString = "Potion";
                        } break;
                        case Pickup::Type::Treasure: {
                            typeAsString = "Treasure";
                        } break;
                        case Pickup::Type::Exit : {
                            typeAsString = "Exit";
                        } break;
                        default: {
                            typeAsString = "???";
                        }
                    }
                    lua_pushstring(lua, typeAsString.c_str());
                }},
            }
        ),
        std::make_shared< LuaPropertyMap< Pickup > >(
            std::initializer_list< LuaPropertyMap< Pickup >::value_type >{
                {"type", [](lua_State* lua, Pickup* component){
                    const std::string type = luaL_checkstring(lua, 3);
                    if (type == "Food") {
                        component->type = Pickup::Type::Food;
                    } else if (type == "Potion") {
                        component->type = Pickup::Type::Potion;
                    } else if (type == "Treasure") {
                        component->type = Pickup::Type::Treasure;
                    } else if (type == "Exit") {
                        component->type = Pickup::Type::Exit;
                    }
                }},
            }
        )
    );
    impl_->MakeComponentType< Position >(
        Type::Position,
        lua,
        "position", "position",
        PASS_THUNKS(Position),
        std::make_shared< LuaPropertyMap< Position > >(
            std::initializer_list< LuaPropertyMap< Position >::value_type >{
                {"x", [](lua_State* lua, Position* component){
                    lua_pushinteger(lua, (lua_Integer)component->x);
                }},
                {"y", [](lua_State* lua, Position* component){
                    lua_pushinteger(lua, (lua_Integer)component->y);
                }},
            }
        ),
        std::make_shared< LuaPropertyMap< Position > >(
            std::initializer_list< LuaPropertyMap< Position >::value_type >{
                {"x", [](lua_State* lua, Position* component){
                    const auto x = (int)luaL_checkinteger(lua, 3);
                    component->x = x;
                }},
                {"y", [](lua_State* lua, Position* component){
                    const auto y = (int)luaL_checkinteger(lua, 3);
                    component->y = y;
                }},
            }
        )
    );
    impl_->MakeComponentType< Reward >(
        Type::Reward,
        lua,
        "rewards", "reward",
        PASS_THUNKS(Reward),
        std::make_shared< LuaPropertyMap< Reward > >(
            std::initializer_list< LuaPropertyMap< Reward >::value_type >{
                {"score", [](lua_State* lua, Reward* component){
                    lua_pushinteger(lua, (lua_Integer)component->score);
                }},
            }
        ),
        std::make_shared< LuaPropertyMap< Reward > >(
            std::initializer_list< LuaPropertyMap< Reward >::value_type >{
                {"score", [](lua_State* lua, Reward* component){
                    const auto score = (int)luaL_checkinteger(lua, 3);
                    component->score = score;
                }},
            }
        )
    );
    impl_->MakeComponentType< Tile >(
        Type::Tile,
        lua,
        "tiles", "tile",
        PASS_THUNKS(Tile),
        std::make_shared< LuaPropertyMap< Tile > >(
            std::initializer_list< LuaPropertyMap< Tile >::value_type >{
                {"name", [](lua_State* lua, Tile* component){
                    lua_pushstring(lua, component->name.c_str());
                }},
                {"z", [](lua_State* lua, Tile* component){
                    lua_pushinteger(lua, (lua_Integer)component->z);
                }},
                {"phase", [](lua_State* lua, Tile* component){
                    lua_pushinteger(lua, (lua_Integer)component->phase);
                }},
                {"spinning", [](lua_State* lua, Tile* component){
                    lua_pushboolean(lua, component->spinning ? 1 : 0);
                }},
                {"dirty", [](lua_State* lua, Tile* component){
                    lua_pushboolean(lua, component->dirty ? 1 : 0);
                }},
                {"destroyed", [](lua_State* lua, Tile* component){
                    lua_pushboolean(lua, component->destroyed ? 1 : 0);
                }},
            }
        ),
        std::make_shared< LuaPropertyMap< Tile > >(
            std::initializer_list< LuaPropertyMap< Tile >::value_type >{
                {"name", [](lua_State* lua, Tile* component){
                    component->name = luaL_checkstring(lua, 3);
                }},
                {"z", [](lua_State* lua, Tile* component){
                    const auto z = (int)luaL_checkinteger(lua, 3);
                    component->z = z;
                }},
                {"phase", [](lua_State* lua, Tile* component){
                    const auto phase = (int)luaL_checkinteger(lua, 3);
                    component->phase = phase;
                }},
                {"spinning", [](lua_State* lua, Tile* component){
                    luaL_checkany(lua, 3);
                    component->spinning = (lua_toboolean(lua, 3) != 0);
                }},
                {"dirty", [](lua_State* lua, Tile* component){
                    luaL_checkany(lua, 3);
                    component->dirty = (lua_toboolean(lua, 3) != 0);
                }},
                {"destroyed", [](lua_State* lua, Tile* component){
                    luaL_checkany(lua, 3);
                    component->destroyed = (lua_toboolean(lua, 3) != 0);
                }},
            }
        ),
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
    impl_->MakeComponentType< Weapon >(
        Type::Weapon,
        lua,
        "weapons", "weapon",
        PASS_THUNKS(Weapon),
        std::make_shared< LuaPropertyMap< Weapon > >(
            std::initializer_list< LuaPropertyMap< Weapon >::value_type >{
                {"dx", [](lua_State* lua, Weapon* component){
                    lua_pushinteger(lua, (lua_Integer)component->dx);
                }},
                {"dy", [](lua_State* lua, Weapon* component){
                    lua_pushinteger(lua, (lua_Integer)component->dy);
                }},
                {"ownerId", [](lua_State* lua, Weapon* component){
                    lua_pushinteger(lua, (lua_Integer)component->ownerId);
                }},
            }
        ),
        std::make_shared< LuaPropertyMap< Weapon > >(
            std::initializer_list< LuaPropertyMap< Weapon >::value_type >{
                {"dx", [](lua_State* lua, Weapon* component){
                    const auto dx = (int)luaL_checkinteger(lua, 3);
                    component->dx = dx;
                }},
                {"dy", [](lua_State* lua, Weapon* component){
                    const auto dy = (int)luaL_checkinteger(lua, 3);
                    component->dy = dy;
                }},
                {"ownerId", [](lua_State* lua, Weapon* component){
                    const auto ownerId = (int)luaL_checkinteger(lua, 3);
                    component->ownerId = ownerId;
                }},
            }
        )
    );
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
