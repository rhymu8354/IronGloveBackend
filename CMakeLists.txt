# CMakeLists.txt for IronGlove
#
# © 2018 by Richard Walters

cmake_minimum_required(VERSION 3.8)
set(This IronGlove)

set(Sources
    src/Components/Collider.hpp
    src/Components/Generator.hpp
    src/Components/Health.hpp
    src/Components/Hero.hpp
    src/Components/Input.hpp
    src/Components/Monster.hpp
    src/Components/Pickup.hpp
    src/Components/Position.hpp
    src/Components/Reward.hpp
    src/Components/Tile.hpp
    src/Components/Weapon.hpp
    src/Component.hpp
    src/Components.cpp
    src/Components.hpp
    src/game.cpp
    src/game.hpp
    src/JsonWrapper.cpp
    src/JsonWrapper.hpp
    src/main.cpp
    src/ScriptHost.cpp
    src/ScriptHost.hpp
    src/TimeKeeper.cpp
    src/TimeKeeper.hpp
    src/WebSocketWrapper.cpp
    src/WebSocketWrapper.hpp
)

add_executable(${This} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Applications
)

target_link_libraries(${This} PUBLIC
    Json
    HttpNetworkTransport
    LuaLibrary
    StringExtensions
    SystemAbstractions
    WebSockets
)

if(UNIX AND NOT APPLE)
    target_link_libraries(${This} PRIVATE
        -static-libstdc++
    )
endif(UNIX AND NOT APPLE)
