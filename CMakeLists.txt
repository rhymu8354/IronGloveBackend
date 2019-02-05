# CMakeLists.txt for IronGlove
#
# © 2018 by Richard Walters

cmake_minimum_required(VERSION 3.8)
set(This IronGlove)

set(Sources
    src/game.cpp
    src/game.hpp
    src/main.cpp
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
