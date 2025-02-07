cmake_minimum_required(VERSION 3.27)
project(Editor CXX)

# build_application(
#     SOURCES
#     application.cpp
#     EditorWorld.hpp
#     EditorWorld.cpp

#     LevelScene.hpp
#     LevelScene.cpp

#     PACKAGES
#     imguidocking
#     stb
#     flecs
#     tinyobjloader
#     engine3d-nfd
#     engine3d

#     LINK_PACKAGES

#     LINK_PACKAGES
#     imguidocking::imguidocking
#     stb::stb
#     flecs::flecs_static
#     tinyobjloader::tinyobjloader
#     engine3d-nfd::engine3d-nfd


#     engine3d::engine3d
# )

build_demos(
    SOURCES
    Application.cpp

    EditorWorld.hpp
    EditorWorld.cpp

    LevelScene.hpp
    LevelScene.cpp
    
    LINK_PACKAGES
    engine3d
)