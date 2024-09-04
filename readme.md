# ImNodeFlow
**Node-based editor/blueprints for ImGui**

Create your custom nodes and their logic... ImNodeFlow will handle connections, editor logic, and rendering.

![image](https://github.com/Fattorino/ImNodeFlow/assets/90210751/605f8cc5-794f-45bd-b4dd-2d6ffdb706e7)

## Features
- Support for Zoom
- Built-in Input and Output logic
- Built-in links handling
- Customizable filters for different connections
- Built-in customizable pop-up events
- Appearance 100% customizable

## Example using SDL2 + OpenGL3 (CMake project)
A simple example using SDL2 + OpenGL3 backend is provided in the /example folder. The CMakeLists.txt file downloads the necessary sources automatically and populates. Simply copy the contents of example folder, configure and build. You can use this example as your starting point to build your code.
```
    > cd example
    > mkdir build
    > cd build
    > cmake ..
    > ./nd
```

![image](https://github.com/Fattorino/ImNodeFlow/assets/90210751/0ef78533-23f6-4cda-96aa-dabb121d1503)

## CMake for custom targets
Shown below is a simple CMake script to setup your own program to compile using Imgui and ImNodeFlow. You can adapt this to your needs.
```
set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/includes/imgui)
set(IMNODEFLOW_DIR ${CMAKE_CURRENT_LIST_DIR}/includes/ImNodeFlow)

include(FetchContent)

FetchContent_Declare(ImNodeFlow
     GIT_REPOSITORY "https://github.com/Fattorino/ImNodeFlow.git"
     GIT_TAG "master"
     SOURCE_DIR ${IMNODEFLOW_DIR}
)
FetchContent_GetProperties(ImNodeFlow)
if(NOT imnodeflow_POPULATED)
  FetchContent_Populate(ImNodeFlow)
endif()

FetchContent_Declare(imgui
     GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
     GIT_TAG "origin/master"
     SOURCE_DIR ${IMGUI_DIR}
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
endif()

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

add_executable(custom_exe custom_exe.cpp ${imgui_sources} ${imnode_flow_sources})
set_property(TARGET custom_exe PROPERTY CXX_STANDARD 17)
target_include_directories(custom_exe PRIVATE ${IMGUI_DIR} ${IMNODEFLOW_DIR}/include ${IMGUI_DIR}/backends)
target_compile_definitions(custom_exe PRIVATE IMGUI_DEFINE_MATH_OPERATORS)
```
Depending on the backend you choose for Imgui you can set `target_link_libraries` to the correct sets. For example SDL2+OpenGL requires the following defintitions,

```
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
if (UNIX)
    if (NOT APPLE)
        find_package(Threads REQUIRED)
        find_package(X11 REQUIRED)
        target_link_libraries(custom_exe PRIVATE
                ${CMAKE_THREAD_LIBS_INIT} ${X11_LIBRARIES} ${CMAKE_DL_LIBS})
    endif()
endif()

target_link_libraries(custom_exe PUBLIC OpenGL::GL SDL2::SDL2)
```

## Full documentation
For a more detailed explanation please refer to the [documentation](documentation.md)

***
### Special credits
- [ocornut](https://github.com/ocornut) for Dear ImGui
- [thedmd](https://github.com/thedmd) for _imgui_bezier_math.h_
- [nem0](https://github.com/nem0) for helping with Zoom support
