# minimum version
cmake_minimum_required(VERSION 3.16)

project(chess CXX)

set(CMAKE_CXX_STANDARD 23)

# set the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(FETCHCONTENT_QUIET FALSE)
include(cmake/get_cpm.cmake)

# set flag to compile only the chessvalidator
option(CHESS_VALIDATOR_ONLY "Compile only the chess validator" OFF)

CPMAddPackage("gh:TheLartians/Format.cmake@1.8.1")

# add external chess lib to use as a validator for the tools
CPMAddPackage(
        NAME disservin
        GITHUB_REPOSITORY InfiniBrains/chess-library
        DOWNLOAD_ONLY YES
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
include_directories(${disservin_SOURCE_DIR}/include)

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

# add raylib
CPMAddPackage(
        NAME raylib
        GITHUB_REPOSITORY raysan5/raylib
        GIT_TAG 4.5.0
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        OPTIONS
        "raylib_USE_STATIC_LIBS ON"
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

# chess library
file(GLOB_RECURSE CHESS_BOT_FILES CONFIGURE_DEPENDS "chess-bot/*.cpp" "chess-bot/*.h")
add_library(chessbot STATIC ${CHESS_BOT_FILES})
set_target_properties(chessbot PROPERTIES LINKER_LANGUAGE CXX)
include_directories(chess-bot)

# chess cli
file(GLOB_RECURSE CHESS_CLI_FILES CONFIGURE_DEPENDS "chess-cli/*.cpp" "chess-cli/*.h")
add_executable(chesscli ${CHESS_CLI_FILES})
target_link_libraries(chesscli PUBLIC chessbot)

if(NOT CHESS_VALIDATOR_ONLY)
# chess gui
file(GLOB_RECURSE CHESS_GUI_FILES CONFIGURE_DEPENDS "chess-gui/*.cpp" "chess-gui/*.h")
add_executable(chessgui ${CHESS_GUI_FILES})
target_link_libraries(chessgui PUBLIC raylib rlImGui chessbot IMGUI lunasvg)
endif() # NOT CHESS_VALIDATOR_ONLY