function hunger(components, tick)
    if tick % 10 ~= 0 then return end
    entitiesStarved = {}
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

systems = {
    hunger
}

function update(components, tick)
    for i,system in ipairs(systems) do
        system(components, tick)
    end
end
