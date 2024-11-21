# ImNodeFlow Documentation
***

## Index
- [NODES](#nodes)
    - [Definition](#definition)
    - [Body content](#body-content)
    - [Static pins](#static-pins)
    - [Dynamic pins](#dynamic-pins)
    - [Styling system](#styling-system)
- [PINS](#pins)
    - [UID system](#uid-system)
    - [Connection filters](#connection-filters)
    - [Output pins](#output-pins)
    - [Input pins](#input-pins)
    - [Styling system](#styling-system-1)
    - [Custom rendering](#custom-rendering)
- [HANDLER](#handler)
  - [Creation](#creation)
  - [Main loop](#main-loop)
  - [Adding nodes](#adding-nodes)
  - [Pop-ups](#pop-ups)
  - [Customization](#customization)

***

## NODES
### Definition
Custom nodes **must** derive from the class BaseNode. This shows at a glance the requirements for the class and serves as a good starting point.
```c++
class CustomNode : public ImFlow::BaseNode
{
    explicit CustomNode() { /* omitted */ }
};
```

You are free to add additional arguments to support custom behavior. 
Any additional arguments will be forwarded during the node creation (see [Adding nodes](#adding-nodes)).
```c++
explicit MultiSumNode(int numInputs) { /* omitted */ }
```

Inside the constructor is then possible to define the node's structure and appearance.
<BR>_NB: every method used inside the constructor can perfectly be used anywhere else._
```c++
class CustomNode : public ImFlow::BaseNode
{
    explicit CustomNode()
    {
        setTitle("I'm custom");
        setStyle(NodeStyle::brown());
        addIN<int>("I'm input", 0, 0, PinStyle::red());
    }
};
```
_This is just an example and every method used will be explained in full details._

### Body content
In addition to input and output pins, each node can have a body that supports all the ImGui widgets.
```c++
class CustomNode : public ImFlow::BaseNode
{
    explicit CustomNode() { /* omitted */ }
    void draw() override 
    {
        /* Node's body with ImGui */
    }
};
```

### Static pins
Static pins are Input or Output pins that can be added to the node and be deleted manually.
```c++
addIN<T>(pin_name, default_value, filter, style);
addOUT<T>(pin_name, filter)->behaviour( /* omitted */ );
```
_The **necessary** `behaviour()` method for Output pins is explained at [Output pins](#output-pins)._
<BR><BR>The following example adds to the node an input pin named `"Input A"` for `int` values,
with a default return value of 0 (value returned when the pin is not connected), a filter, 
and the default pin style (cyan).
```c++
addIN<int>("Input A", 0, ConnectionFilter::SameType());
```
_For a detailed explanation of pins see [PINS](#pins)._

### Dynamic pins
Dynamic pins are Input or Output pins that, just like any other ImGui widget, exist only as long as they are called each frame.
<BR>Dynamic inputs take the same parameters as static inputs, And they return each frame either the default or the connected value.
```c++
T val = showIN<T>(pin_name, default_value, filter, style);
showOUT<T>(pin_name, behaviour, filter, style);
```
_As mentioned in Static pins, `behaviour` is explained at [Output pins](#output-pins)._

### Styling system
The node's style can be fully customized. Use `setStyle()` to change the style at any time.
The default style is cyan, and the available pre-built styles are: cyan, green, red, and brown .
<BR>It is alo possible to create custom styles either from scratch or starting prom a pre-built one.
```c++
// Most common
auto custom1 = NodeStyle::brown();
custom1->radius = 10.f;

// Less used
auto custom2 std::make_shared<NodeStyle>(IM_COL32(71,142,173,255), ImColor(233,241,244,255), 6.5f);
```
Other than the visual appearance of the node (colors and sizes), it is also possible to set and/or change the node's title at any time using `setTitle()`.

***
## PINS
### UID system
Each pin can be identified by a UID, in some cases the UID can coincide with the display name, or it can be a custom UID of any type.
```c++
addIN<int>(pin_name, 0, filter);
addIN_uid<int>(0, pin_name, 0, filter);
```
The UID can be used to get a reference to the pin, and in case of an input pin, its value.
<BR>Searching for an UID that doesn't exist will throw an error.

### Connection filters
Filters are useful to avoid unwanted connection between pins.

_TODO: Update this section_

### Output pins
Output pins are in charge of processing the output and, as per the name, outputting it to the connected link.
<BR>What is outputted is defined by the pin behaviour. _(See [Static pins](#static-pins) and/or [Dynamic pins](#dynamic-pins) to set the behaviour)._
<BR>In particular, the behaviour is a function or a lambda expression that returns the same type of the pin.
```c++
addOUT<int>(pin_name)
                ->behaviour([this](){ return 0; });
```
In this simple example, a static pin is added, such pin will always output a value of 0 to the connected link.
```c++
addOUT_uid<float>(uid, pin_name)
                ->behaviour([this](){ /* omitted */ });
```
In this other example, another static pi is added, a custom UID is used and the behaviour is some custom, more complex, logic.
<BR><BR>_Dynamic pins also exist, see [Dynamic pins](#dynamic-pins)._

### Input pins
Input pins are in charge of getting the value from the connected link.
If no link is connected to the pin, the default value is returned. (See)
<BR>The method `getInVal(uid)` can be used to retrieve an input value.
```c++
addIN<T>(pin_name, default_value, filter, style);
addIN_uid<T>(uid, pin_name, default_value, filter, style);
```
_The method `addIN` adds a static pin where the name is also used as its UID._

### Styling system
When creating either a Static or Dynamic pin, it is possible to pass as an argument a style setting.
<BR>The default style is cyan, and the available pre-built styles are: cyan, green, blue, brown, red, and white.
<BR>It is alo possible to create custom styles either from scratch or starting prom a pre-built one.
```c++
// Most common
auto custom1 = PinStyle::green();
custom1->socket_radius = 10.f;

// Less used
auto custom2 std::make_shared<PinStyle>(PinStyle(IM_COL32(87,155,185,255), 0, 4.f, 4.67f, 3.7f, 1.f));
```
_When creating a style from scratch, keep in mind that it must be a `smart_pointer` and not a simple instance._

### Custom rendering
Pin rendering is handled internally. But for extra customization, a custom renderer can be assigned at each pin.
<BR>The custom renderer is a function or a lambda expression containing the new logic to draw the pin.
<BR>Some helpers are provided: `drawSocket()` and `drawDecoration()`.
```c++
addIN<float>("Custom", 0, 0)->renderer([](Pin* p) {
    auto pp = dynamic_cast<InPin<float>*>(p);
    ImGui::Text("%s: %.3f", pp->getName().c_str(), pp->val());

    p->drawSocket();
    p->drawDecoration();
});
```
In this example the pin is rendered with the same socket and hover background, thanks to the two helpers.
The content of the pin is a custom `ImGui::Text` with the name and the value of the pin.
<BR>All the logic related to links is still handled internally as well as hover events.

***
## HANDLER
### Creation
The handler, as the name suggests, handles the grid. It's responsible for all the evens and the rendering.
<BR>It is possible to create an unlimited number of grid editors. 
<BR>The default constructor creates a new editor named `"FlowGrid{i}"` where `{i}` is an increment counter.
<BR>Otherwise it is possible to specify a custom name.

### Main loop
Each frame the handler must be updated. On each update the events will be processed and nodes and links are drawn.
```c++
// Inside Dear ImGui window
myGrid.update(); // Update logic and render
// . . .
```
_This will only render the node editor, so it must be called inside a Dear ImGui window. The editor will auto-fit the available space by default.
(See [Customization](#customization) for more options)._

### Adding nodes
The handler has ownership over the nodes. ALl the nodes are stored in a list.
Three methods are provided to add nodes.
```c++
myGrid.addNode<T>(pos, ...);
```
Adds a node at the given grid coordinates. 
<BR>The `...` represents the extra optional parameters that may be required by the custom node.
```c++
myGrid.placeNode<CustomNode>(...);
```
Adds a node at the mouse position _(screen coordinates)_.
<BR>The `...` represents the extra optional parameters that may be required by the custom node.
```c++
myGrid.placeNodeAt<CustomNode>(pos, ...);
```
Adds a node at the given screen coordinates.
<BR>The `...` represents the extra optional parameters that may be required by the custom node.

### Pop-ups
The handler also provides pop-up events for right-click and dropped-link events.
<BR>The dropped-link even is triggered when the user is dragging a link and _drops it_ on an empty point on the grid.
<BR><BR>**Right-click pup-up:**
```c++
myGrid.rightClickPopUpContent([this](BaseNode* node){
    /* omitted */
});
```
Takes a function or a lambda expression (like in the example) with the content of the pop-up and the subsequent logic.
<BR>The pointer `node` points to the right-clicked node. Can be `nullptr` if the right-click happened on an empty point.

**Dropped-link pup-up:**
```c++
myGrid.droppedLinkPopUpContent([this](Pin* dragged){
    /* omitted */
}, key);
```
The first parameter is a function or a lambda expression (like in the example) with the content of the pop-up and the subsequent logic.
<BR>Additionally, an optional key can be specified. In this case the pop-up will trigger only if the given key is being held down at the moment of the _drop_.
<BR>The pointer `dragged` points to the pin the dropped link is attached to.

### Customization
The handler is fully customizable. A custom fixed size can be specified using `.setSize()`, and the visual appearance can be accessed using `.getStyle()`.
<BR>All the remaining configuration parameters can be accessed via `.getGrid().config()`.

***
_Also consult the [examples folder]() for hands-on practical examples **(coming soon)**_.

_In case of problems or questions, consider opening an issue._

_Please refer to the doxygen documentation for a list of public methods and their details._
