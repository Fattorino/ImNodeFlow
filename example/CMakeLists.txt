cmake_minimum_required(VERSION 3.14)
project(example VERSION 1.0 LANGUAGES CXX)

set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/includes/imgui)
set(IMNODEFLOW_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
    GIT_TAG "v1.91.6"  # Update with future minimum compatibility
    SOURCE_DIR ${IMGUI_DIR}
    GIT_SHALLOW TRUE  # Limit history to download
)
FetchContent_MakeAvailable(imgui)

list(APPEND imgui_sources
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)

list(APPEND imnode_flow_sources
  ${IMNODEFLOW_DIR}/src/ImNodeFlow.cpp)

add_executable(example example.cpp ${imgui_sources} ${imnode_flow_sources})
set_property(TARGET example PROPERTY CXX_STANDARD 17)
target_include_directories(example PRIVATE ${IMGUI_DIR} ${IMNODEFLOW_DIR}/include ${IMGUI_DIR}/backends)
target_compile_definitions(example PRIVATE IMGUI_DEFINE_MATH_OPERATORS)

if(CMAKE_SYSTEM_NAME MATCHES Emscripten)
  include(cmake/emscripten.cmake)
else()
  include(cmake/desktop.cmake)
endif()

