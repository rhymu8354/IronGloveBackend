-- Utilities

function AddMonster(components, x, y)
    local id = components:CreateEntity()
    local collider = components:CreateComponentOfType("collider", id)
    local health = components:CreateComponentOfType("health", id)
    local monster = components:CreateComponentOfType("monster", id)
    local position = components:CreateComponentOfType("position", id)
    local tile = components:CreateComponentOfType("tile", id)
    local reward = components:CreateComponentOfType("reward", id)
    collider.mask = 2
    tile.name = "monster"
    tile.z = 1
    position.x = x
    position.y = y
    health.hp = 1
    reward.score = 10
end

function IsObstacleInTheWay(components, x, y, mask)
    for collider in components.colliders do
        local position = components:GetEntityComponentOfType("position", collider.entityId)
        if position and (position.x == x) and (position.y == y) and (mask & collider.mask) then
            return true
        end
    end
    return false
end

-- Systems

function Weapons(components, tick)
end

function PlayerFiring(components, tick)
end

function PlayerMovement(components, tick)
    for input in components.inputs do
        if input.moveCooldown > 0 then
            input.moveCooldown = input.moveCooldown - 1
        elseif input.fire == "" then
            local position = components:GetEntityComponentOfType("position", input.entityId)
            if position then
                local collider = components:GetEntityComponentOfType("collider", input.entityId)
                local mask = collider and collider.mask or 0
                components:DiagnosticMessage(3, "mask: " .. mask)
                local moveDelegates = {
                    ['j'] = function()
                        if not IsObstacleInTheWay(components, position.x - 1, position.y, mask) then
                            position.x = position.x - 1
                        end
                    end,
                    ['l'] = function()
                        if not IsObstacleInTheWay(components, position.x + 1, position.y, mask) then
                            position.x = position.x + 1
                        end
                    end,
                    ['i'] = function()
                        if not IsObstacleInTheWay(components, position.x, position.y - 1, mask) then
                            position.y = position.y - 1
                        end
                    end,
                    ['k'] = function()
                        if not IsObstacleInTheWay(components, position.x, position.y + 1, mask) then
                            position.y = position.y + 1
                        end
                    end,
                }
                local moveDelegate = moveDelegates[input.move]
                if moveDelegate then
                    moveDelegate()
                end
                input.moveCooldown = 1
                input.moveThisTick = false
                if input.moveReleased then
                    input.move = ""
                end
                local tile = components:GetEntityComponentOfType("tile", input.entityId)
                if tile then
                    tile.dirty = true
                end
            end
        end
    end
end

function AI(components, tick)
end

function Generation(components, tick)
    for generator in components.generators do
        local position = components:GetEntityComponentOfType("position", generator.entityId)
        local roll = math.random()
        if roll < generator.spawnChance then
            local d = math.floor(math.random(0, 4) + 0.5)
            local x = position.x
            local y = position.y
            if d / 2 == 0 then
                x = x + 2 * (d % 2) - 1
            else
                y = y + 2 * (d % 2) - 1
            end
            if not IsObstacleInTheWay(components, x, y, ~0) then
                AddMonster(components, x, y)
            end
        end
    end
end

function Pickup(components, tick)
    local heroes = components.heroes
    if #heroes ~= 1 then
        components:DiagnosticMessage(3, "#heroes ~= 1")
        return
    end
    local hero = heroes[1]
    local playerPosition = components:GetEntityComponentOfType("position", hero.entityId)
    local playerHealth = components:GetEntityComponentOfType("health", hero.entityId)
    if not playerPosition or not playerHealth then
        components:DiagnosticMessage(3, "hero is missing position and/or health")
        return
    end
    local entitiesDestroyed = {}
    local exited = false
    for pickup in components.pickups do
        local position = components:GetEntityComponentOfType("position", pickup.entityId)
        if position and position.x == playerPosition.x and position.y == playerPosition.y then
            local destroyPickup = true
            if pickup.type == "Treasure" then
                hero.score = hero.score + 100
            elseif pickup.type == "Food" then
                playerHealth.hp = playerHealth.hp + 100
            elseif pickup.type == "Potion" then
                hero.potions = hero.potions + 1
            elseif pickup.type == "Exit" then
                exited = true
                destroyPickup = false
            end
            if destroyPickup then
                entitiesDestroyed[#entitiesDestroyed + 1] = pickup.entityId
            end
        end
    end
    for i,entityId in ipairs(entitiesDestroyed) do
        components:KillEntity(entityId)
    end
    if exited then
        local tile = components:GetEntityComponentOfType("tile", hero.entityId)
        if tile then
            tile.destroyed = true
        end
        components:DestroyEntityComponentOfType("position", hero.entityId)
    end
end

function Hunger(components, tick)
    if tick % 10 ~= 0 then return end
    local entitiesStarved = {}
    for hero in components.heroes do
        local health = components:GetEntityComponentOfType("health", hero.entityId)
        local position = components:GetEntityComponentOfType("position", hero.entityId)
        if health and position and health.hp > 0 then
            health.hp = health.hp - 1
            if health.hp <= 0 then
                entitiesStarved[#entitiesStarved + 1] = hero.entityId
            end
        end
    end
    for i,entityId in ipairs(entitiesStarved) do
        components:KillEntity(entityId)
    end
end

function Render(components, tick)
end

systems = {
    Weapons,
    PlayerFiring,
    PlayerMovement,
    AI,
    Generation,
    Pickup,
    Hunger,
    Render
}

function update(components, tick)
    for i,system in ipairs(systems) do
        system(components, tick)
    end
end
