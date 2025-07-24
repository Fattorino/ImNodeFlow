cmake_minimum_required(VERSION 3.10)

set(TARGET_NAME ImNodeFlow)
find_package(imgui CONFIG REQUIRED)

set(IMNODEFLOW_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/src/ImNodeFlow.cpp
)

add_library(${TARGET_NAME} STATIC ${IMNODEFLOW_SOURCES})
add_library(${TARGET_NAME}::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${TARGET_NAME} PUBLIC imgui)
target_compile_definitions(${TARGET_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
