# ImNodeFlow
**Node based editor/blueprints for ImGui**

Create your custom nodes, and their logic.
ImNodeFlow will handle connections, editor logic and rendering.

![image](https://github.com/Fattorino/ImNodeFlow/assets/90210751/605f8cc5-794f-45bd-b4dd-2d6ffdb706e7)

## Features
- Support for Zoom
- Backed-in Input and Output logic
- Backed-in links handling
- Customizable filters for different connections
- Backed-in customizable pop-ups
- Appearance 100% customizable

## Implementation (CMake project)
### CMake `FetchContent`
1. Add the following lines to your CMakeLists.txt:
   ```
   include(FetchContent)
   FetchContent_Declare(ImNodeFlow
        GIT_REPOSITORY "https://github.com/Fattorino/ImNodeFlow.git"
        GIT_TAG "v1.1.1"
        SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/includes/ImNodeFlow"
   )
   FetchContent_MakeAvailable(ImNodeFlow)
   ```
   ```
   add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)
   target_link_libraries(YourProject ImNodeFlow)
   ```
2. Make sure you have the following dependencies available for `find_package()`:
   - [Dear ImGui](https://github.com/ocornut/imgui)

### Manually
1. Download and copy, or clone the repo (or the latest release) inside your project
2. Add the following lines to your CMakeLists.txt:
   ```
   add_subdirectory(path/to/ImNodeFlow)
   . . .
   add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)
   target_link_libraries(YourProject ImNodeFlow)
   ```
3. Make sure you have the following dependencies available for `find_package()`:
   - [Dear ImGui](https://github.com/ocornut/imgui)

## Simple Node example
```c++
class SimpleSum : public BaseNode
{
public:
    explicit SimpleSum(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf)
    {
        addIN<int>("IN_VAL", 0, ConnectionFilter_Int);
        addOUT<int>("OUT_VAL", ConnectionFilter_Int)
                ->behaviour([this](){ return getInVal<int>("IN_VAL") + m_valB; });
    }

    void draw() override
    {
        ImGui::SetNextItemWidth(100.f);
        ImGui::InputInt("##ValB", &m_valB);
    }
private:
    int m_valB = 0;
};
```

![image](https://github.com/Fattorino/ImNodeFlow/assets/90210751/4722b1e8-a52c-4ae2-b3f7-babfc713d8db)


## Full documentation
For a more detailed explanation please refer to the [full documentation](documentation.md)
***
### Special credits
- [ocornut](https://github.com/ocornut) for Dear ImGui
- [thedmd](https://github.com/thedmd) for _imgui_bezier_math.h_
- [nem0](https://github.com/nem0) for helping with Zoom support
