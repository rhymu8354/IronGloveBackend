# CMakeLists.txt for IronGlove
#
# © 2018 by Richard Walters

cmake_minimum_required(VERSION 3.8)
set(This IronGlove)

set(Sources
    src/Components/Collider.hpp
    src/Components/Generator.hpp
    src/Components/Health.hpp
    src/Components/Input.hpp
    src/Components/Monster.hpp
    src/Components/Position.hpp
    src/Components/Tile.hpp
    src/Systems/AI.cpp
    src/Systems/AI.hpp
    src/Systems/Generation.cpp
    src/Systems/Generation.hpp
    src/Systems/PlayerMovement.cpp
    src/Systems/PlayerMovement.hpp
    src/Systems/Render.cpp
    src/Systems/Render.hpp
    src/Systems/Weapons.cpp
    src/Systems/Weapons.hpp
    src/Component.hpp
    src/Components.cpp
    src/Components.hpp
    src/game.cpp
    src/game.hpp
    src/main.cpp
    src/System.hpp
    src/Systems.cpp
    src/Systems.hpp
    src/TimeKeeper.cpp
    src/TimeKeeper.hpp
)

add_executable(${This} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Applications
)

target_link_libraries(${This} PUBLIC
    Json
    HttpNetworkTransport
    SystemAbstractions
    WebSockets
)

if(UNIX AND NOT APPLE)
    target_link_libraries(${This} PRIVATE
        -static-libstdc++
    )
endif(UNIX AND NOT APPLE)
