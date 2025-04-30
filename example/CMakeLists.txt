cmake_minimum_required(VERSION 3.14)
project(example VERSION 1.0 LANGUAGES CXX)

option(USE_SYSTEM_IMGUI "Use system Imgui instead of automatic download" OFF)

set(IMNODEFLOW_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
list(APPEND imnode_flow_sources
  ${IMNODEFLOW_DIR}/src/ImNodeFlow.cpp)

if(USE_SYSTEM_IMGUI)
  # Make sure you have the Findimgui.cmake scripts
  # available to CMake using correct path
  find_package(imgui)
  # Make sure the target imgui::imgui is setup in your scripts
  add_executable(example example.cpp ${imnode_flow_sources})
  target_link_libraries(ImNodeFlow imgui::imgui)
else()
  # Location to download Imgui sources
  set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/includes/imgui)
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
  add_executable(example example.cpp ${imgui_sources} ${imnode_flow_sources})
  target_include_directories(example PRIVATE ${IMGUI_DIR} ${IMGUI_DIR}/backends)
endif()

# Common definitions
target_include_directories(example PRIVATE ${IMNODEFLOW_DIR}/include)
set_property(TARGET example PROPERTY CXX_STANDARD 17)
target_compile_definitions(example PRIVATE IMGUI_DEFINE_MATH_OPERATORS)

if(CMAKE_SYSTEM_NAME MATCHES Emscripten)
  include(cmake/emscripten.cmake)
else()
  include(cmake/desktop.cmake)
endif()

