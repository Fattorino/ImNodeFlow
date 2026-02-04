cmake_minimum_required(VERSION 3.10)

project(ImNodeFlow CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(
    include
)

set(SOURCE_FILES
    src/ImNodeFlow.cpp
)

add_library(ImNodeFlow SHARED ${SOURCE_FILES})
target_compile_definitions(ImNodeFlow PRIVATE IMGUI_DEFINE_MATH_OPERATORS)

target_include_directories(ImNodeFlow
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

install(TARGETS ImNodeFlow
    EXPORT ImNodeFlowTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT ImNodeFlowTargets
    FILE ImNodeFlowConfig.cmake
    NAMESPACE ImNodeFlow::
    DESTINATION lib/cmake/ImNodeFlow
)