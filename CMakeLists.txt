# minimum version
cmake_minimum_required(VERSION 3.16)

project(chess CXX)

set(CMAKE_CXX_STANDARD 20)

include(cmake/get_cpm.cmake)

CPMAddPackage("gh:TheLartians/Format.cmake@1.8.1")

# add external chess lib to use as a validator for the tools
CPMAddPackage(
        NAME disservin
        GITHUB_REPOSITORY Disservin/chess-library
        DOWNLOAD_ONLY YES
        GIT_TAG master
)
include_directories(${disservin_SOURCE_DIR}/include)

# add json lib
CPMAddPackage(
        NAME nlohmann_json
        VERSION 3.11.3
        GITHUB_REPOSITORY nlohmann/json
        DOWNLOAD_ONLY YES
        OPTIONS
        "JSON_BuildTests OFF"
)
include_directories(${nlohmann_json_SOURCE_DIR}/single_include)

# add magic_enum lib
CPMAddPackage(
        NAME magic_enum
        VERSION 0.9.5
        GITHUB_REPOSITORY Neargye/magic_enum
        DOWNLOAD_ONLY YES
        OPTIONS
        "MAGIC_ENUM_BUILD_EXAMPLES OFF"
        "MAGIC_ENUM_BUILD_TESTS OFF"
        "MAGIC_ENUM_BUILD_INSTALL OFF"
)
include_directories(${magic_enum_SOURCE_DIR}/include)

# add cpr
CPMAddPackage(
        NAME cpr
        GITHUB_REPOSITORY libcpr/cpr
        GIT_TAG master
)
include_directories(${cpr_SOURCE_DIR}/include)

# add raylib
CPMAddPackage(
        NAME raylib
        GITHUB_REPOSITORY raysan5/raylib
        GIT_TAG 5.0
)

# add imgui
CPMAddPackage(
        NAME IMGUI
        GITHUB_REPOSITORY ocornut/imgui
        GIT_TAG docking
)
include_directories(${IMGUI_SOURCE_DIR})
add_library(IMGUI STATIC)
target_sources(
        IMGUI
        PUBLIC
        ${IMGUI_SOURCE_DIR}/imgui_demo.cpp
        ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
        ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
        ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
        ${IMGUI_SOURCE_DIR}/imgui.cpp
)

# add rlImGui
CPMAddPackage(
        NAME rlImGui
        GITHUB_REPOSITORY raylib-extras/rlImGui
        GIT_TAG main
)
file(GLOB rlImGui_SOURCES
        ${rlImGui_SOURCE_DIR}/*.cpp
        ${rlImGui_SOURCE_DIR}/*.c
        ${rlImGui_SOURCE_DIR}/imgui/*.cpp
)
include_directories(${rlImGui_SOURCE_DIR})
add_library (rlImGui STATIC ${rlImGui_SOURCES})
target_link_libraries(rlImGui PUBLIC raylib IMGUI)

# chess library
file(GLOB_RECURSE CHESS_LIB_FILES CONFIGURE_DEPENDS "chess-library/*.cpp" "chess-library/*.h")
add_library(chesslibrary ${CHESS_LIB_FILES})
set_target_properties(chesslibrary PROPERTIES LINKER_LANGUAGE CXX)
include_directories(chess-library)

# chess validator to be used on the backend
file(GLOB_RECURSE CHESS_VALIDATOR_FILES CONFIGURE_DEPENDS "chess-validator/*.cpp" "chess-validator/*.h")
add_executable(chessvalidator ${CHESS_VALIDATOR_FILES})

# chess gui
file(GLOB_RECURSE CHESS_GUI_FILES CONFIGURE_DEPENDS "chess-gui/*.cpp" "chess-gui/*.h")
add_executable(chessgui ${CHESS_GUI_FILES})
target_link_libraries(chessgui PUBLIC raylib rlImGui chesslibrary IMGUI)