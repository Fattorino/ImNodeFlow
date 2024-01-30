# ImNodeFlow Documentation
***

## Index
- [Getting started](#getting-started)
- [Custom Nodes 101](#creating-a-custom-node)
  - [Inputs](#adding-input-pins)
  - [Outputs](#adding-output-pins)
  - [Body](#nodes-body)
  - [Example](#all-together)
  - [Adding it](#adding-nodes-to-the-grid)
- [Connection filters 101](#custom-filters)
  - [Basic filters](#basic-filters)
  - [Creating filters](#creating-more-filters)
- [Pop-ups 101](#custom-pop-ups)
  - [Right click](#right-click-pop-up)
  - [Dropped link](#dropped-link-pop-up)

## Getting started
After having included the necessary files into the project. A few simple steps are necessary.
```c++
#include <ImNodeFLow.h> // Include dependencies
```
```c++
ImNodeFlow INF; // Create an editor with default name
ImNodeFlow INF("Name"); // Create an editor with given name
```
```c++
// Inside Dear ImGui loop
INF.update(); // Update logic and render
// . . .
```

## Creating a custom node
Custom nodes **must** be derived from the class BaseNode.
```c++
class CustomNode : public BaseNode
{
public:
    . . .
private:
    . . .
};
```

### The constructor
```c++
explicit CustomNode(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf);
```
The constructor is standard and must **not** be changed.

### Adding input pins
```c++
explicit CustomNode(. . .)
{
    addIN<int>("Pin name", 0, Connection filter);
}
```
`addIN<T>` will add an input pin to the node. Usually called in the node's constructor.
#### Getting the value
```c++
int value = ins<int>(n);
```
Returns a read only reference to the value connected to the nth input pin.

### Adding output pins
```c++
explicit CustomNode(. . .)
{
    addOUT<int>("Pin name", Connection filter);
}
```
`addOUT<T>` will add an output pin to the node. Usually called in the node's constructor.
#### Defining logic
```c++
behaviour([this](){ return . . .; });
```
Node's logic for a specific pin.
<BR> Takes either a function or lambda expression. The return value is the output of the pin.
#### All together
```c++
addOUT<int>("Pin name", Connection filter)
                ->behaviour([this](){ return 0; });
```
Creates a pin with given name and filter and sets its logic.
<BR> This pin will be rather useless since it always returns 0 (also known as the author's IQ).

### Node's body
```c++
void draw() override { . . . }
```
Called each frame to draw ImGui widgets inside the node's body.
<BR> Can be left empty if the nodes only needs inputs and outputs.

### What have we learned?
To create a custom node, all it's needed is to define input pins, output pins + custom logic, and an optional body.
<BR> Everything else will be handled internally.
```c++
class SimpleSum : public BaseNode
{
public:
    explicit SimpleSum(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf)
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
The example presented in the readme.
<BR> SimpleSum has one input pin of type int called `IN_VAL` and one output pin called `OUT_VAL`.
Both have their filter set to `int`.
<BR> The output pin returns the input + the slider's value.
<BR> In the body the slider is rendered.

### Adding nodes to the grid
It's now time to add our beautifully useless node to the grid.
```c++
INF.addNode<CustomNode>("Node's name", ImVec2(0, 0)); // Add node at canvas coordinates
INF.dropNode<CustomNode>("Node's name", ImVec2(0, 0)); // Add node at screen coordinates
```

***

## Custom filters
Filters can be used to block unwanted links between pins.
### Basic filters
- `ConnectionFilter_None`: The pin will allow any connection
- `ConnectionFilter_Int`: The pin will only allow other `int` connections
- `ConnectionFilter_Float`: The pin will only allow other `float` connections
- `ConnectionFilter_Double`: The pin will only allow other `double` connections
- `ConnectionFilter_String`: The pin will only allow other `string` connections
- `ConnectionFilter_Numbers`: The pin will only allow `int`, `float` and `double` connections

### Creating more filters
It is possible to create more filters with the help of `ConnectionFilter_MakeCustom`.
```c++
ConnectionFilter myFilter = ConnectionFilter_MakeCustom << 0;

enum MyFilters
{
    FilterA = ConnectionFilter_MakeCustom << 1,
    FilterB = ConnectionFilter_MakeCustom << 2,
    FilterC = FilterA | FilterB
};
```
Demonstrates different approaches to creating filters.
<BR> Note that `FilterC` will allow `FilterA`, `FilterB` and `FilterC` connections.

***

## Custom pop-ups
ImNodeFlow supports two pop-up events.
<BR> The pop-up state is handled internally, so we only need to handle the content and out custom logic.

### Right-click pop-up
Triggered when right-clicking on an empty point on the grid.
```c++
INF.rightClickPopUpContent([]() {
    if (ImGui::Selectable("Dummy example"))
    {
        // Very smart logic
    }
    // . . .
});
```
`rightClickPopUpContent` takes either a function or lambda expression.
<BR> Said function must contain pop-up contents to be displayed and the logic.

### Dropped link pop-up
Triggered when a link id _dropped_ in an empty point on the grid. And, if specified, the correct key is pressed.
<BR> In this example the pop-up will only opened if the _Shift_ key is being pressed while _dropping_ the link.
```c++
INF.droppedLinkPopUpContent([](Pin* dragged) {
    if (ImGui::Selectable("Dummy example"))
    {
        // Very smart logic
    }
    // . . .
}, ImGuiKey_LeftShift);
```
`droppedLinkPopUpContent` takes either a function or lambda expression.
<BR> Said function must contain pop-up contents to be displayed and the logic.
<BR> An optional key to press can also be specified.

***

_Please refer to the doxygen documentation for a list of public methods and their details._

_In case of problems or questions, consider opening an issue._