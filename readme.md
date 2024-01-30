# ImNodeFlow
**Node based editor/blueprints for ImGui**

Create your custom nodes, and their logic.
ImNodeFLow will handle connections, editor logic and rendering.

https://github.com/Fattorino/ImNodeFlow/assets/90210751/c2c1e7a6-8f83-42df-8a26-037de8835f9d

## Features
- Backed-in Input and Output logic
- Backed-in links handling
- Customizable filters for different connections
- Backed-in customizable pop-ups
- Appearance 100% customizable

## Implementation (CMake project)
1. Download and copy, or clone the repo inside your project
2. Add the following lines to your CMakeLists.txt:
   ```
   add_subdirectory(path/to/ImNodeFlow)
   target_link_libraries(YourProject ImNodeFlow)
   ```
   
### Alternative
Download the latest ImNodeFlow.zip containing only the necessary files and add them manually.

## Simple Node example
```c++
class SimpleSum : public BaseNode
{
public:
    explicit Somma(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf)
    {
        addIN<int>("IN_VAL", 0, ConnectionFilter_Int);
        addOUT<int>("OUT_VAL", ConnectionFilter_Int)
                ->behaviour([this](){ return ins<int>(0) + m_valB; });
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
- [thedmd](https://github.com/thedmd) for imgui_bezier_math.h
