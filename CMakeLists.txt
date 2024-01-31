# minimum version
cmake_minimum_required(VERSION 3.16)

project(chess CXX)

set(CMAKE_CXX_STANDARD 20)

set(FETCHCONTENT_QUIET FALSE)
include(cmake/get_cpm.cmake)

# set flag to compile only the chessvalidator
option(CHESS_VALIDATOR_ONLY "Compile only the chess validator" OFF)

CPMAddPackage("gh:TheLartians/Format.cmake@1.8.1")

# add external chess lib to use as a validator for the tools
CPMAddPackage(
        NAME disservin
        GITHUB_REPOSITORY Disservin/chess-library
        DOWNLOAD_ONLY YES
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
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
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
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
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)
include_directories(${magic_enum_SOURCE_DIR}/include)

IF(NOT CHESS_VALIDATOR_ONLY)
# add cpr
CPMAddPackage(
        NAME cpr
        GITHUB_REPOSITORY libcpr/cpr
        GIT_TAG master
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)
include_directories(${cpr_SOURCE_DIR}/include)

# add raylib
CPMAddPackage(
        NAME raylib
        GITHUB_REPOSITORY raysan5/raylib
        GIT_TAG 5.0
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)

# add imgui
CPMAddPackage(
        NAME IMGUI
        GITHUB_REPOSITORY ocornut/imgui
        GIT_TAG docking
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
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
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)
file(GLOB rlImGui_SOURCES
        ${rlImGui_SOURCE_DIR}/*.cpp
        ${rlImGui_SOURCE_DIR}/*.c
        ${rlImGui_SOURCE_DIR}/imgui/*.cpp
)
include_directories(${rlImGui_SOURCE_DIR})
add_library (rlImGui STATIC ${rlImGui_SOURCES})
target_link_libraries(rlImGui PUBLIC raylib IMGUI)

# fontawesome
CPMAddPackage(
        NAME fontawesome
        GITHUB_REPOSITORY juliettef/IconFontCppHeaders
        DOWNLOAD_ONLY YES
        GIT_TAG main
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)
include_directories(${fontawesome_SOURCE_DIR})

# add lunasvg
CPMAddPackage(
        NAME lunasvg
        GITHUB_REPOSITORY sammycage/lunasvg
        GIT_TAG v2.3.9
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)
include_directories(${lunasvg_SOURCE_DIR}/include)

endif() # NOT CHESS_VALIDATOR_ONLY

# Memory Tracker
file(GLOB MEMORY_TRACKER_FILES CONFIGURE_DEPENDS "memory-tracker/*.cpp" "memory-tracker/*.h")
include_directories(memory-tracker)
add_library(memorytracker STATIC ${MEMORY_TRACKER_FILES})

# chess library
file(GLOB_RECURSE CHESS_BOT_FILES CONFIGURE_DEPENDS "chess-bot/*.cpp" "chess-bot/*.h")
add_library(chessbot STATIC ${CHESS_BOT_FILES})
set_target_properties(chessbot PROPERTIES LINKER_LANGUAGE CXX)
include_directories(chess-bot)

# chess validator to be used on the backend
file(GLOB_RECURSE CHESS_VALIDATOR_FILES CONFIGURE_DEPENDS "chess-validator/*.cpp" "chess-validator/*.h")
add_executable(chessvalidator ${CHESS_VALIDATOR_FILES})

# chess cli
file(GLOB_RECURSE CHESS_CLI_FILES CONFIGURE_DEPENDS "chess-cli/*.cpp" "chess-cli/*.h")
add_executable(chesscli ${CHESS_CLI_FILES})
target_link_libraries(chesscli PUBLIC chessbot)

if(NOT CHESS_VALIDATOR_ONLY)
# chess gui
file(GLOB_RECURSE CHESS_GUI_FILES CONFIGURE_DEPENDS "chess-gui/*.cpp" "chess-gui/*.h")
add_executable(chessgui ${CHESS_GUI_FILES})
target_link_libraries(chessgui PUBLIC raylib rlImGui chessbot IMGUI lunasvg memorytracker)
endif() # NOT CHESS_VALIDATOR_ONLY