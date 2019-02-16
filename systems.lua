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

function GetColliderAt(components, x, y)
    for collider in components.colliders do
        local position = components:GetEntityComponentOfType("position", collider.entityId)
        if position and position.x == x and position.y == y then
            return collider
        end
    end
    return nil
end

function IsObstacleInTheWay(components, x, y, mask)
    for collider in components.colliders do
        local position = components:GetEntityComponentOfType("position", collider.entityId)
        if position and (position.x == x) and (position.y == y) and (mask & collider.mask) ~= 0 then
            return true
        end
    end
    return false
end

function OnStrike(components, weapon, victimCollider, entitiesDestroyed)
    local ownerInput = components:GetEntityComponentOfType("input", weapon.ownerId)
    local ownerHero = components:GetEntityComponentOfType("hero", weapon.ownerId)
    local health = components:GetEntityComponentOfType("health", victimCollider.entityId)
    local reward = components:GetEntityComponentOfType("reward", victimCollider.entityId)
    if health then
        health.hp = health.hp - 1
        if health.hp <= 0 then
            entitiesDestroyed[#entitiesDestroyed + 1] = victimCollider.entityId
            if ownerHero and reward then
                ownerHero.score = ownerHero.score + reward.score
            end
        end
    end
    entitiesDestroyed[#entitiesDestroyed + 1] = weapon.entityId
    if ownerInput then
        ownerInput.weaponInFlight = false
    end
end

-- Systems

function Weapons(components, ws, tick)
    local entitiesDestroyed = {}
    for weapon in components.weapons do
        local position = components:GetEntityComponentOfType("position", weapon.entityId)
        if position then
            local tile = components:GetEntityComponentOfType("tile", weapon.entityId)
            if tile then
                tile.phase = ((tile.phase + 1) % 4)
            end
            local collider = GetColliderAt(components, position.x, position.y)
            if collider then
                OnStrike(components, weapon, collider, entitiesDestroyed)
            else
                local x = position.x + weapon.dx
                local y = position.y + weapon.dy
                collider = GetColliderAt(components, x, y)
                if collider then
                    OnStrike(components, weapon, collider, entitiesDestroyed)
                else
                    position.x = x
                    position.y = y
                    if tile then
                        tile.dirty = true
                    end
                end
            end
        end
    end
    for i,entityId in ipairs(entitiesDestroyed) do
        components:KillEntity(entityId)
    end
end

function PlayerFiring(components, ws, tick)
    for input in components.inputs do
        local hero = components:GetEntityComponentOfType("hero", input.entityId)
        local playerPosition = components:GetEntityComponentOfType("position", input.entityId)
        if hero and playerPosition then
            if input.usePotion and hero.potions > 0 then
                hero.potions = hero.potions - 1
                entitiesDestroyed = {}
                for monster in components.monsters do
                    local monsterPosition = components:GetEntityComponentOfType("position", monster.entityId)
                    if monsterPosition then
                        local dx = monsterPosition.x - playerPosition.x
                        local dy = monsterPosition.y - playerPosition.y
                        if math.sqrt((dx * dx) + (dy * dy)) <= 5 then
                            entitiesDestroyed[#entitiesDestroyed + 1] = monster.entityId
                            local reward = components:GetEntityComponentOfType("reward", monster.entityId)
                            if reward then
                                hero.score = hero.score + reward.score
                            end
                        end
                    end
                end
                for i,entityId in ipairs(entitiesDestroyed) do
                    components:KillEntity(entityId)
                end
            end
            input.usePotion = false
            if input.fire ~= "" and not input.weaponInFlight then
                local dx = 0
                local dy = 0
                local fireDelegates = {
                    ["a"] = function()
                        dx = -1
                    end,
                    ["d"] = function()
                        dx = 1
                    end,
                    ["w"] = function()
                        dy = -1
                    end,
                    ["s"] = function()
                        dy = 1
                    end,
                }
                local fireDelegate = fireDelegates[input.fire]
                if fireDelegate then
                    fireDelegate()
                end
                input.fireThisTick = false
                if input.fireReleased then
                    input.fire = ""
                end
                local id = components:CreateEntity()
                local weapon = components:CreateComponentOfType("weapon", id)
                local weaponPosition = components:CreateComponentOfType("position", id)
                local tile = components:CreateComponentOfType("tile", id)
                weapon.dx = dx
                weapon.dy = dy
                tile.name = "axe"
                tile.z = 2
                tile.spinning = true
                weaponPosition.x = playerPosition.x + dx
                weaponPosition.y = playerPosition.y + dy
                weapon.ownerId = input.entityId
                input.weaponInFlight = true
            end
        end
    end
end

function PlayerMovement(components, ws, tick)
    for input in components.inputs do
        if input.moveCooldown > 0 then
            input.moveCooldown = input.moveCooldown - 1
        elseif input.fire == "" then
            local position = components:GetEntityComponentOfType("position", input.entityId)
            if position then
                local collider = components:GetEntityComponentOfType("collider", input.entityId)
                local mask = collider and collider.mask or 0
                local moveDelegates = {
                    ["j"] = function()
                        if not IsObstacleInTheWay(components, position.x - 1, position.y, mask) then
                            position.x = position.x - 1
                        end
                    end,
                    ["l"] = function()
                        if not IsObstacleInTheWay(components, position.x + 1, position.y, mask) then
                            position.x = position.x + 1
                        end
                    end,
                    ["i"] = function()
                        if not IsObstacleInTheWay(components, position.x, position.y - 1, mask) then
                            position.y = position.y - 1
                        end
                    end,
                    ["k"] = function()
                        if not IsObstacleInTheWay(components, position.x, position.y + 1, mask) then
                            position.y = position.y + 1
                        end
                    end,
                }
                local moveDelegate = moveDelegates[input.move]
                if moveDelegate then
                    moveDelegate()
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
end

function AI(components, ws, tick)
    if tick % 5 ~= 0 then return end
    local heroes = components.heroes
    if #heroes ~= 1 then return end
    local hero = heroes[1]
    local playerPosition = components:GetEntityComponentOfType("position", hero.entityId)
    if not playerPosition then return end
    local playerHealth = components:GetEntityComponentOfType("health", hero.entityId)
    local colliders = components.colliders
    entitiesDestroyed = {}
    local playerDestroyed = false
    for monster in components.monsters do
        local position = components:GetEntityComponentOfType("position", monster.entityId)
        local tile = components:GetEntityComponentOfType("tile", monster.entityId)
        if position then
            local collider = components:GetEntityComponentOfType("collider", monster.entityId)
            local mask = collider and collider.mask or 0
            local dx = math.abs(position.x - playerPosition.x)
            local dy = math.abs(position.y - playerPosition.y)
            local mx = 0
            local my = 0
            if position.x < playerPosition.x then
                mx = 1
            elseif position.x > playerPosition.x then
                mx = -1
            end
            if position.y < playerPosition.y then
                my = 1
            elseif position.y > playerPosition.y then
                my = -1
            end
            if (
                (
                    (position.x + mx == playerPosition.x)
                    and (position.y == playerPosition.y)
                )
                or (
                    (position.x == playerPosition.x)
                    and (position.y + my == playerPosition.y)
                )
            ) then
                if playerHealth ~= nullptr and not playerDestroyed then
                    playerHealth.hp = playerHealth.hp - 10
                    if playerHealth.hp <= 0 then
                        playerHealth.hp = 0
                        playerDestroyed = true
                    end
                end
                local monsterHealth = components:GetEntityComponentOfType("health", monster.entityId)
                if monsterHealth then
                    monsterHealth.hp = 0
                    entitiesDestroyed[#entitiesDestroyed + 1] = monster.entityId
                end
            else
                if (
                    (dx > dy)
                    and not IsObstacleInTheWay(
                        components,
                        position.x + mx,
                        position.y,
                        mask
                    )
                ) then
                    position.x = position.x + mx
                elseif (
                    not IsObstacleInTheWay(
                        components,
                        position.x,
                        position.y + my,
                        mask
                    )
                ) then
                    position.y = position.y + my
                elseif (
                    not IsObstacleInTheWay(
                        components,
                        position.x + mx,
                        position.y,
                        mask
                    )
                ) then
                    position.x = position.x + mx
                end
                if tile then
                    tile.dirty = true
                end
            end
        end
    end
    for i,entityId in ipairs(entitiesDestroyed) do
        components:KillEntity(entityId)
    end
    if playerDestroyed then
        components:KillEntity(hero.entityId)
    end
end

function Generation(components, ws, tick)
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

function Pickup(components, ws, tick)
    local heroes = components.heroes
    if #heroes ~= 1 then return end
    local hero = heroes[1]
    local playerPosition = components:GetEntityComponentOfType("position", hero.entityId)
    local playerHealth = components:GetEntityComponentOfType("health", hero.entityId)
    if not playerPosition or not playerHealth then
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

function Hunger(components, ws, tick)
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

local previousRender = json(nil)
function Render(components, ws, tick)
    local message = json.Parse('{"type": "render"}')
    local sprites = json.Parse('[]')
    local entitiesDestroyed = {}
    for tile in components.tiles do
        local sprite = json.Parse('{"id": ' .. tile.entityId .. '}')
        if tile.destroyed then
            sprite.destroyed = true
            entitiesDestroyed[#entitiesDestroyed + 1] = tile.entityId
        else
            if not tile.dirty then goto continue end
            tile.dirty = false
            local position = components:GetEntityComponentOfType("position", tile.entityId)
            if not position then goto continue end
            sprite.texture = tile.name
            sprite.x = position.x
            sprite.y = position.y
            sprite.z = tile.z
            sprite.phase = tile.phase
            sprite.spinning = tile.spinning
            local weapon = components:GetEntityComponentOfType("weapon", tile.entityId)
            if weapon then
                local motion = json.Parse('{}')
                motion.dx = weapon.dx
                motion.dy = weapon.dy
                sprite.motion = motion
            end
        end
        json.Add(sprites, sprite)
        ::continue::
    end
    message.sprites = sprites
    local heroes = components.heroes
    if #heroes == 1 then
        local hero = heroes[1]
        local playerHealth = components:GetEntityComponentOfType("health", hero.entityId)
        message.health = playerHealth.hp
        message.score = hero.score
        message.potions = hero.potions
    end
    if message ~= previousRender then
        ws:SendText(message)
        previousRender = message
    end
    for i,entityId in ipairs(entitiesDestroyed) do
        components:DestroyEntityComponentOfType("tile", entityId)
    end
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

function update(components, ws, tick)
    for i,system in ipairs(systems) do
        system(components, ws, tick)
    end
end
