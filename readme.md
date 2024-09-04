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


## Full documentation
For a more detailed explanation please refer to the [documentation](documentation.md)

***
### Special credits
- [ocornut](https://github.com/ocornut) for Dear ImGui
- [thedmd](https://github.com/thedmd) for _imgui_bezier_math.h_
- [nem0](https://github.com/nem0) for helping with Zoom support
