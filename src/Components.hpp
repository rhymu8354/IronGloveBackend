#pragma once

#include "Component.hpp"
#include "Components/Collider.hpp"
#include "Components/Generator.hpp"
#include "Components/Health.hpp"
#include "Components/Hero.hpp"
#include "Components/Input.hpp"
#include "Components/Monster.hpp"
#include "Components/Pickup.hpp"
#include "Components/Position.hpp"
#include "Components/Reward.hpp"
#include "Components/Tile.hpp"
#include "Components/Weapon.hpp"

#include <memory>
#include <SystemAbstractions/DiagnosticsSender.hpp>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

class Components {
    // Types
public:
    enum class Type {
        Collider,
        Generator,
        Health,
        Hero,
        Input,
        Monster,
        Pickup,
        Position,
        Reward,
        Tile,
        Weapon,
    };

    struct ComponentList {
        Component* first = nullptr;
        size_t n = 0;
    };

    // Lifecycle Methods
public:
    ~Components() noexcept;
    Components(const Components&) = delete;
    Components(Components&&) noexcept = delete;
    Components& operator=(const Components&) = delete;
    Components& operator=(Components&&) noexcept = delete;

    // Public Methods
public:
    /**
     * This is the constructor of the class.
     */
    Components();

    void SetDiagnosticsSender(
        std::shared_ptr< SystemAbstractions::DiagnosticsSender > diagnosticsSender
    );

    /**
     * This method is called to link the class with the given
     * Lua interpreter.
     *
     * @param[in] lua
     *     This points to the state of the Lua interpreter.
     */
    static void LinkLua(lua_State* lua);

    /**
     * This method is called to push the object onto
     * the Lua stack.
     *
     * @param[in] lua
     *     This points to the state of the Lua interpreter.
     */
    void PushLua(lua_State* lua);

    ComponentList GetComponentsOfType(Type type);
    Component* CreateComponentOfType(Type type, int entityId);
    Component* GetEntityComponentOfType(Type type, int entityId);
    int CreateEntity();
    void KillEntity(int entityId);
    void DestroyEntityComponentOfType(Type type, int entityId);
    bool IsObstacleInTheWay(int x, int y, int mask);
    Collider* GetColliderAt(int x, int y);

    // Private properties
private:
    /**
     * This is the type of structure that contains the private
     * properties of the instance.  It is defined in the implementation
     * and declared here to ensure that it is scoped inside the class.
     */
    struct Impl;

    /**
     * This contains the private properties of the instance.
     */
    std::shared_ptr< Impl > impl_;
};
