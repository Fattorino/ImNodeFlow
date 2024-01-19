cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 17)

# CREATE PROJECT
project(ImNodeFlow)

# SET SOURCE FILES FOR PROJECT
file(GLOB_RECURSE _HDRS "include/*.h")
file(GLOB_RECURSE _SRCS "src/*.cpp")
#file(GLOB_RECURSE _IMGUI_NODE_EDITOR "imgui_node_editor/*.cpp" "imgui_node_editor/*.h" "imgui_node_editor/*.inl")

# PRE-PROCESSOR VARs
add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)

# CREATE LIBRARY FROM SOURCE_FILES
#add_library(ImNodeFlow ${_SRCS} ${_HDRS} ${_IMGUI_NODE_EDITOR})
add_library(ImNodeFlow ${_SRCS} ${_HDRS})

# FIND DEPENDENCIES
find_package(imgui)

# LINK CONAN LIBS
target_link_libraries(ImNodeFlow imgui::imgui)

# PREP TO USE "#include <>"
target_include_directories(ImNodeFlow PUBLIC include)

#DEVELOPMENT PROJECT
project(ImNodeFlow_dev)
add_subdirectory(ImGuiHandler)
add_executable(ImNodeFlow_dev main.cpp)
target_link_libraries(ImNodeFlow_dev ImNodeFlow)
target_link_libraries(ImNodeFlow_dev ImGuiHandler)
