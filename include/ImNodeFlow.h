#ifndef IM_NODE_FLOW
#define IM_NODE_FLOW
#pragma once

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <memory>
#include <algorithm>
#include <functional>
#include <imgui.h>
#include "../src/imgui_bezier_math.h"
#include "../src/context_wrapper.h"

// TODO: [POLISH] Collision solver to bring first node on foreground to avoid clipping
// TODO: [EXTRA]  Custom renderers for Pins (with lambdas I think)

namespace ImFlow
{
    // -----------------------------------------------------------------------------------------------------------------
    // HELPERS

    /**
     * @brief Draw a sensible bezier between two points
     * @param p1 Starting point
     * @param p2 Ending point
     * @param color Color of the curve
     * @param thickness Thickness of the curve
     */
    inline static void smart_bezier(const ImVec2& p1, const ImVec2& p2, ImU32 color, float thickness);

    /**
     * @brief Collider checker for smart_bezier
     * @details Projects the point "p" orthogonally onto the bezier curve and
     *          checks if the distance is less than the given radius.
     * @param p Point to be tested
     * @param p1 Starting point of smart_bezier
     * @param p2 Ending point of smart_bezier
     * @param radius Lateral width of the hit box
     * @return [TRUE] if "p" is inside the collider
     *
     * Intended to be used in union with smart_bezier();
     */
    inline static bool smart_bezier_collider(const ImVec2& p, const ImVec2& p1, const ImVec2& p2, float radius);

    // -----------------------------------------------------------------------------------------------------------------
    // CLASSES PRE-DEFINITIONS

    template<typename T> class InPin;
    template<typename T> class OutPin;
    class Pin; class BaseNode;
    class ImNodeFlow;

    // -----------------------------------------------------------------------------------------------------------------
    // FILTERS

    /**
     * @brief Basic filters
     * @details List of, ready to use, basic filters. It's possible to create more filters with the help of "ConnectionFilter_MakeCustom".
     */
    enum ConnectionFilter_
    {
        ConnectionFilter_None   = 0,
        ConnectionFilter_Int    = 1 << 1,
        ConnectionFilter_Float  = 1 << 2,
        ConnectionFilter_Double = 1 << 3,
        ConnectionFilter_String = 1 << 4,
        ConnectionFilter_MakeCustom = 1 << 5,
        ConnectionFilter_Numbers = ConnectionFilter_Int | ConnectionFilter_Float | ConnectionFilter_Double
    };
    typedef long ConnectionFilter;

    // -----------------------------------------------------------------------------------------------------------------
    // LINK

    /**
     * @brief Link between two Pins of two different Nodes
     */
    class Link
    {
    public:
        /**
         * @brief Construct a link
         * @param left Pointer to the output Pin of the Link
         * @param right Pointer to the input Pin of the Link
         * @param inf Pointer to the Handler that contains the Link
         */
        explicit Link(Pin* left, Pin* right, ImNodeFlow* inf) :m_left(left), m_right(right), m_inf(inf) {}

        /**
         * @brief Destruction of a link
         * @details Deletes references of this links form connected pins
         */
        ~Link();

        /**
         * @brief Looping function to update the Link
         * @details Draws the Link and updates Hovering and Selected status.
         */
        void update();

        /**
         * @brief Get Left pin of the link
         * @return Pointer to the Pin
         */
        [[nodiscard]] Pin* left() const { return m_left; }

        /**
         * @brief Get Right pin of the link
         * @return Pointer to the Pin
         */
        [[nodiscard]] Pin* right() const { return m_right; }

        /**
         * @brief Get hovering status
         * @return [TRUE] If the link is hovered in the current frame
         */
        [[nodiscard]] bool hovered() const { return m_hovered; }

        /**
         * @brief Get selected status
         * @return [TRUE] If the link is selected in the current frame
         */
        [[nodiscard]] bool selected() const { return m_selected; }
    private:
        ImNodeFlow* m_inf;
        Pin* m_left;
        Pin* m_right;
        bool m_hovered = false;
        bool m_selected = false;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // HANDLER

    /**
     * @brief All the color parameters
     */
    struct InfColors
    {
        /// @brief Background of the Pin
        ImU32 pin_bg = IM_COL32(23, 16, 16, 0);
        /// @brief Overlay to be displayed when Pin is hovered
        ImU32 pin_hovered = IM_COL32(100, 100, 255, 70);
        /// @brief Border to be displayed around the Pin
        ImU32 pin_border = IM_COL32(255, 255, 255, 0);
        /// @brief Border to be displayed around the Pin
        ImU32 pin_point = IM_COL32(255, 255, 240, 230);

        /// @brief Link
        ImU32 link = IM_COL32(230, 230, 200, 230);
        /// @brief Link while dragging
        ImU32 drag_out_link = IM_COL32(230, 230, 200, 230);
        /// @brief Outline of a selected link
        ImU32 link_selected_outline = IM_COL32(80, 20, 255, 200);

        /// @brief Background of the node's body
        ImU32 node_bg = IM_COL32(97, 103, 122, 100);
        /// @brief Background of the node's header
        ImU32 node_header = IM_COL32(23, 16, 16, 150);
        /// @brief Text in the node's header
        ImColor node_header_title = ImColor(255, 246, 240, 255);
        /// @brief Node's border
        ImU32 node_border = IM_COL32(100, 100, 100, 255);
        /// @brief Node's border when it's selected
        ImU32 node_selected_border = IM_COL32(170, 190, 205, 230);

        /// @brief Background of the grid
        ImU32 background = IM_COL32(44, 51, 51, 255);
        /// @brief Main lines of the grid
        ImU32 grid = IM_COL32(200, 200, 200, 40);
        /// @brief Secondary lines
        ImU32 subGrid = IM_COL32(200, 200, 200, 10);
    };

    /**
     * @brief ALl the Styling parameters. Sizes + Colors
     */
    struct InfStyler
    {
        /// @brief Padding between Pin border and content
        ImVec2 pin_padding = ImVec2(3.f, 1.f);
        /// @brief Pin's edges rounding
        float pin_radius = 8.f;
        /// @brief Thickness of the border drawn around the Pin
        float pin_border_thickness = 1.f;
        /// @brief Radius of the circle in front of the Pin when connected
        float pin_point_radius = 3.5f;
        /// @brief Radius of the circle in front of the Pin when not connected
        float pin_point_empty_radius = 4.f;
        /// @brief Radius of the circle in front of the Pin when not connected and hovered
        float pin_point_empty_hovered_radius = 4.67f;

        /// @brief Thickness of the drawn link
        float link_thickness = 2.6f;
        /// @brief Thickness of the drawn link when hovered
        float link_hovered_thickness = 3.5f;
        /// @brief Thickness of the outline of a selected Link
        float link_selected_outline_thickness = 0.5f;
        /// @brief Thickness of the dummy link while dragging
        float drag_out_link_thickness = 2.f;

        /// @brief Padding of Node's content (Left Top Right Bottom)
        ImVec4 node_padding = ImVec4(9.f, 6.f, 9.f, 2.f);
        /// @brief Node's edges rounding
        float node_radius = 8.f;
        /// @brief Node's border thickness
        float node_border_thickness = 1.f;
        /// @brief Node's border thickness when selected
        float node_border_selected_thickness = 2.f;

        /// @brief Size of main grid
        float grid_size = 50.f;
        /// @brief Sub-grid divisions for Node snapping
        float grid_subdivisions = 5.f;

        /// @brief ImNodeFlow colors
        InfColors colors;
    };

    /**
     * @brief Main node editor
     * @details Handles the infinite grid, nodes and links. Also handles all the logic.
     */
    class ImNodeFlow
    {
    private:
        static int m_instances;
    public:
        /**
         * @brief Instantiate a new editor with default name
         *
         * <BR> Editor name will be "FlowGrid + the number of editors".
         */
        ImNodeFlow() : ImNodeFlow("FlowGrid" + std::to_string(m_instances)) {}

        /**
         * @brief Instantiate a new editor with given name
         * @details Creates a new Node Editor with the given name.
         * @param name Name of the editor
         */
        explicit ImNodeFlow(std::string name) :m_name(std::move(name))
        {
            m_instances++;
            m_context.config().extra_window_wrapper = true;
            m_context.config().color = m_style.colors.background;
        }

        /**
         * @brief Handler loop
         * @details Main update function. Refreshes all the logic and draws everything. Must be called every frame.
         */
        void update();

        /**
         * @brief Adds a node to the editor
         * @tparam T Derived class of <BaseNode> to be added
         * @tparam Params types of optional args to forward to derived class ctor
         *
         * @param name Name to be given to the Node
         * @param pos Position of the Node in canvas coordinates
         * @param args Optional arguments to be forwarded to derived class ctor
         * @return Pointer of the pushed type to the newly added Node
         *
         * Inheritance is checked at compile time, \<T> MUST be derived from BaseNode.
         */
        template<typename T, typename... Params>
        T* addNode(const std::string& name, const ImVec2& pos, Params&&... args);

        /**
         * @brief Adds a node to the editor using mouse position
         * @tparam T Derived class of <BaseNode> to be added
         * @tparam Params types of optional args to forward to derived class ctor
         *
         * @param name Name to be given to the Node
         * @param args Optional arguments to be forwarded to derived class ctor
         * @return Pointer of the pushed type to the newly added Node
         *
         * Inheritance is checked at compile time, \<T> MUST be derived from BaseNode.
         */
        template<typename T, typename... Params>
        T* placeNode(const std::string& name, Params&&... args);

        /**
         * @brief Adds a node to the editor
         * @tparam T Derived class of <BaseNode> to be added
         * @tparam Params types of optional args to forward to derived class ctor
         * @param name Name to be given to the Node
         * @param pos Position of the Node in screen coordinates
         * @param args Optional arguments to be forwarded to derived class ctor
         * @return Pointer of the pushed type to the newly added Node
         *
         * Inheritance is checked at compile time, \<T> MUST be derived from BaseNode.
         */
        template<typename T, typename... Params>
        T* placeNode(const std::string& name, const ImVec2& pos, Params&&... args);

        /**
         * @brief Add link to the handler internal list
         * @param link Reference to the link
         */
        void addLink(std::shared_ptr<Link>& link);

        /**
         * @brief Pop-up when link is "dropped"
         * @details Sets the content of a pop-up that can be displayed when dragging a link in the open instead of onto another pin.
         * @details If "key = ImGuiKey_None" the pop-up will always open when a link is dropped.
         * @param content Function or Lambda containing only the contents of the pop-up and the subsequent logic
         * @param key Optional key required in order to open the pop-up
         */
        void droppedLinkPopUpContent(std::function<void(Pin* dragged)> content, ImGuiKey key = ImGuiKey_None) { m_droppedLinkPopUp = std::move(content); m_droppedLinkPupUpComboKey = key; }

        /**
         * @brief Pop-up when right-clicking
         * @details Sets the content of a pop-up that can be displayed when right-clicking on the grid.
         * @param content Function or Lambda containing only the contents of the pop-up and the subsequent logic
         */
        void rightClickPopUpContent(std::function<void()> content) { m_rightClickPopUp = std::move(content); }

        /**
         * @brief Get mouse clicking status
         * @return [TRUE] if mouse is clicked and click hasn't been consumed
         */
        [[nodiscard]] bool getSingleUseClick() const { return m_singleUseClick; }

        /**
         * @brief Consume the click for the given frame
         */
        void consumeSingleUseClick() { m_singleUseClick = false; }

        /**
         * @brief Get editor's name
         * @return Const reference to editor's name
         */
        const std::string& name() { return m_name; }

        /**
         * @brief Get editor's position
         * @return Const reference to editor's position in screen coordinates
         */
        const ImVec2& pos() { return m_context.origin(); }

        /**
         * @brief Get editor's grid scroll
         * @details Scroll is the offset from the origin of the grid, changes while navigating the grid with the middle mouse.
         * @return Const reference to editor's grid scroll
         */
        const ImVec2& scroll() { return m_context.scroll(); }

        /**
         * @brief Get editor's list of nodes
         * @return Const reference to editor's internal nodes list
         */
        const std::vector<std::shared_ptr<BaseNode>>& nodes() { return m_nodes; }

        /**
         * @brief Get nodes count
         * @return Number of nodes present in the editor
         */
        uint32_t nodesCount() { return (uint32_t)m_nodes.size(); }

        /**
         * @brief Get editor's list of links
         * @return Const reference to editor's internal links list
         */
        const std::vector<std::weak_ptr<Link>>& links() { return m_links; }

        /**
         * @brief Get zooming viewport
         * @return Const reference to editor's internal viewport for zoom support
         */
        const ContainedContext& context() { return m_context; }

        /**
         * @brief Get dragging status
         * @return [TRUE] if a Node is being dragged around the grid
         */
        [[nodiscard]] bool draggingNode() const { return m_draggingNode; }

        /**
         * @brief Get current style
         * @return Reference to style variables
         */
        InfStyler& style() { return m_style; }

        /**
         * @brief Set editor's size
         * @param size Editor's size. Set to (0, 0) to auto-fit.
         */
        void size(const ImVec2& size) { m_context.config().size = size; }

        /**
         * @brief Set dragging status
         * @param state New dragging state
         *
         * The new state will only be updated one at the start of each frame.
         */
        void draggingNode(bool state) { m_draggingNodeNext = state; }

        /**
         * @brief Set what pin is being hovered
         * @param hovering Pointer to the hovered pin
         */
        void hovering(Pin* hovering) { m_hovering = hovering; }

        /**
         * @brief Convert coordinates from grid to zooming viewport
         * @param p Point in canvas coordinates to be converted
         * @return Point in screen coordinates
         */
        ImVec2 content2canvas(const ImVec2& p);

        /**
         * @brief Convert coordinates from canvas to screen
         * @param p Point in canvas coordinates to be converted
         * @return Point in screen coordinates
         */
        ImVec2 canvas2screen(const ImVec2& p);

        /**
         * @brief Convert coordinates from screen to canvas
         * @param p Point in screen coordinates to be converted
         * @return Point in canvas coordinates
         */
        ImVec2 screen2content(const ImVec2 &p);

        /**
         * @brief Convert coordinates from screen to zooming viewport
         * @param p Point in screen coordinates to be converted
         * @return Point in canvas coordinates
         */
        ImVec2 screen2canvas(const ImVec2& p);

        /**
         * @brief Check if mouse is on selected node
         * @return [TRUE] if the mouse is hovering a selected node
         */
        bool on_selected_node();

        /**
         * @brief Check if mouse is on a free point on the grid
         * @return [TRUE] if the mouse is not hovering a node or a link
         */
        bool on_free_space();
    private:
        std::string m_name;
        ContainedContext m_context;

        bool m_singleUseClick = false;

        std::vector<std::shared_ptr<BaseNode>> m_nodes;
        std::vector<std::weak_ptr<Link>> m_links;

        std::function<void()> m_rightClickPopUp;
        std::function<void(Pin* dragged)> m_droppedLinkPopUp;
        ImGuiKey m_droppedLinkPupUpComboKey = ImGuiKey_None;
        Pin* m_droppedLinkLeft = nullptr;

        bool m_draggingNode = false, m_draggingNodeNext = false;
        Pin* m_hovering = nullptr;
        Pin* m_dragOut = nullptr;

        InfStyler m_style;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // BASE NODE

    typedef unsigned long long int PinUID;

    /**
     * @brief Parent class for custom nodes
     * @details Main class from which custom nodes can be created. All interactions with the main grid are handled internally.
     */
    class BaseNode
    {
    public:
        /**
         * @brief Basic constructor
         * @param name Name of the node
         * @param pos Position in grid coordinates
         * @param inf Pointer to the Grid Handler the node is in
         */
        explicit BaseNode(std::string name, ImVec2 pos, ImNodeFlow* inf);

        /**
         * @brief Main loop of the node
         * @details Updates position, hovering and selected status, and renders the node. Must be called each frame.
         * @param offset Position of the grid Origin in screen coordinates
         */
        void update(ImVec2& offset);

        /**
         * @brief Content of the node
         * @details Function to be implemented by derived custom nodes.
         *          Must contain the body of the node. If left empty the node will only have input and output pins.
         */
        virtual void draw() = 0;

        /**
         * @brief Add an Input to the node
         * @details Must be called in the node constructor. WIll add an Input pin to the node with the given name and data type.
         *          <BR> <BR> In this case the name of the pin will also be his UID.
         * @tparam T Type of the data the pin will handle
         * @param name Name of the pin
         * @param defReturn Default return value when the pin is not connected
         * @param filter Connection filter
         */
        template<typename T>
        void addIN(const std::string& name, T defReturn, ConnectionFilter filter = ConnectionFilter_None);

        /**
         * @brief Add an Input to the node
         * @details Must be called in the node constructor. WIll add an Input pin to the node with the given name and data type.
         *          <BR> <BR> The UID must be unique only in the context of the current node.
         * @tparam T Type of the data the pin will handle
         * @tparam U Type of the UID
         * @param uid Unique identifier of the pin
         * @param name Name of the pin
         * @param defReturn Default return value when the pin is not connected
         * @param filter Connection filter
         */
        template<typename T, typename U>
        void addIN_uid(U uid, const std::string& name, T defReturn, ConnectionFilter filter = ConnectionFilter_None);

        /**
         * @brief Add an Output to the node
         * @details Must be called in the node constructor. WIll add an Output pin to the node with the given name and data type.
         *          <BR> <BR> In this case the name of the pin will also be his UID.
         *          <BR> <BR> The UID must be unique only in the context of the current node.
         * @tparam T Type of the data the pin will handle
         * @param name Name of the pin
         * @param filter Connection filter
         * @return Pointer to the newly added pin. Must be used to set behaviour
         */
        template<typename T>
        [[nodiscard]] OutPin<T>* addOUT(const std::string& name, ConnectionFilter filter = ConnectionFilter_None);

        /**
         * @brief Add an Output to the node
         * @details Must be called in the node constructor. WIll add an Output pin to the node with the given name and data type.
         *          <BR> <BR> The UID must be unique only in the context of the current node.
         * @tparam T Type of the data the pin will handle
         * @tparam U Type of the UID
         * @param uid Unique identifier of the pin
         * @param name Name of the pin
         * @param filter Connection filter
         * @return Pointer to the newly added pin. Must be used to set behaviour
         */
        template<typename T, typename U>
        [[nodiscard]] OutPin<T>* addOUT_uid(U uid, const std::string& name, ConnectionFilter filter = ConnectionFilter_None);

        /**
         * @brief Get Input value from an InPin
         * @details Get a reference to the value of an input pin, the value is stored in the output pin at the other end of the link.
         * @tparam T Data type
         * @tparam U Type of the UID
         * @param uid Unique identifier of the pin
         * @return Const reference to the value
         */
        template<typename T, typename U>
        const T& getInVal(U uid);

        /**
         * @brief Get Input value from an InPin
         * @details Get a reference to the value of an input pin, the value is stored in the output pin at the other end of the link.
         * @tparam T Data type
         * @param uid Unique identifier of the pin
         * @return Const reference to the value
         */
        template<typename T>
        const T& getInVal(const char* uid);

        /**
         * @brief Get generic reference to input pin
         * @tparam U Type of the UID
         * @param uid Unique identifier of the pin
         * @return Generic pointer to the pin
         */
        template<typename U>
        Pin* inPin(U uid) { return m_ins.at(std::hash<U>{}(uid)).get(); }

        /**
         * @brief Get generic reference to input pin
         * @param uid Unique identifier of the pin
         * @return Generic pointer to the pin
         */
        Pin* inPin(const char* uid) { return m_ins.at(std::hash<std::string>{}(std::string(uid))).get(); }

        /**
         * @brief Get generic reference to output pin
         * @tparam U Type of the UID
         * @param uid Unique identifier of the pin
         * @return Generic pointer to the pin
         */
        template<typename U>
        Pin* outPin(U uid) { return m_outs.at(std::hash<U>{}(uid)).get(); }

        /**
         * @brief Get generic reference to output pin
         * @param uid Unique identifier of the pin
         * @return Generic pointer to the pin
         */
        Pin* outPin(const char* uid) { return m_outs.at(std::hash<std::string>{}(std::string(uid))).get(); }

        /**
         * @brief Get hovered status
         * @return [TRUE] if the mouse is hovering the node
         */
        bool hovered();

        /**
         * @brief Get node name
         * @return Const reference to the node's name
         */
        const std::string& name() { return m_name; }

        /**
         * @brief Get node size
         * @return Const reference to the node's size
         */
        const ImVec2& size() { return  m_size; }

        /**
         * @brief Get node position
         * @return Const reference to the node's position
         */
        const ImVec2& pos() { return  m_pos; }

        /**
         * @brief Get selected status
         * @return [TRUE] if the node is selected
         */
        [[nodiscard]] bool selected() const { return m_selected; }

        /**
         * @brief Get dragged status
         * @return [TRUE] if the node is being dragged
         */
        [[nodiscard]] bool dragged() const { return m_dragged; }

        /**
         * @brief Set node's name
         * @param name New name
         */
        void name(const std::string& name) { m_name = name; }

        /**
         * @brief Set selected status
         * @param state New selected state
         *
         * Status only updates when updatePublicStatus() is called
         */
        void selected(bool state) { m_selectedNext = state; }

        /**
         * @brief Updates the selected status of the node
         */
        void updatePublicStatus() { m_selected = m_selectedNext; }
    private:
        std::string m_name;
        ImVec2 m_pos, m_posTarget;
        ImVec2 m_size;
        ImNodeFlow* m_inf;
        bool m_selected = false, m_selectedNext = false;
        bool m_dragged = false;
        ImVec2 m_paddingTL;
        ImVec2 m_paddingBR;

        std::unordered_map<PinUID, std::shared_ptr<Pin>> m_ins;
        std::unordered_map<PinUID, std::shared_ptr<Pin>> m_outs;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // PINS

    /**
     * @brief Pins type identifier
     */
    enum PinType
    {
        PinType_Input,
        PinType_Output
    };

    /**
     * @brief Generic base class for pins
     */
    class Pin
    {
    public:
        /**
         * @brief Generic pin constructor
         * @param name Name of the pin
         * @param filter Connection filter
         * @param kind Specifies Input or Output
         * @param parent Pointer to the Node containing the pin
         * @param inf Pointer to the Grid Handler the pin is in (same as parent)
         */
        explicit Pin(std::string name, ConnectionFilter filter, PinType kind, BaseNode* parent, ImNodeFlow* inf)
            : m_name(std::move(name)), m_filter(filter), m_type(kind), m_parent(parent), m_inf(inf) {}

        /**
         * @brief Main loop of the pin
         * @details Updates position, hovering and dragging status, and renders the pin. Must be called each frame.
         */
        virtual void update() = 0;

        /**
         * @brief Create link between pins
         * @param other Pointer to the other pin
         */
        virtual void createLink(Pin* other) = 0;

        /**
         * @brief Sets the reference to a link
         * @param link Pointer to the link
         */
        virtual void setLink(std::shared_ptr<Link>& link) {}

        /**
         * @brief Deletes the link from pin
         */
        virtual void deleteLink() {}

        /**
         * @brief Get pin's link
         * @return Weak_ptr reference to the link connected to the pin
         */
        virtual std::weak_ptr<Link> getLink() { return std::weak_ptr<Link>{}; }

        /**
         * @brief Get pin's name
         * @return Const reference to pin's name
         */
        const std::string& name() { return m_name; }

        /**
         * @brief Get pin's position
         * @return Const reference to pin's position in canvas coordinates
         */
        [[nodiscard]] const ImVec2& pos() { return m_pos; }

        /**
         * @brief Get pin's hit-box size
         * @return Const reference to pin's hit-box size
         */
        [[nodiscard]] const ImVec2& size() { return m_size; }

        /**
         * @brief Get pin's parent node
         * @return Const reference to pin's parent node. Node that contains it
         */
        BaseNode* parent() { return m_parent; }

        /**
         * @brief Get pin's type
         * @return The pin type. Either Input or Output
         */
        PinType type() { return m_type; }

        /**
         * @brief Get pin's connection filter
         * @return Pin's connection filter configuration
         */
        [[nodiscard]] ConnectionFilter filter() const { return m_filter; }

        /**
         * @brief Get pin's link attachment point
         * @return Canvas coordinates to the attachment point between the link and the pin
         */
        virtual ImVec2 pinPoint() = 0;

        /**
         * @brief Calculate pin's width pre-rendering
         * @return The with of the pin once it will be rendered
         */
        float calcWidth() { return ImGui::CalcTextSize(m_name.c_str()).x; }

        /**
         * @brief Set pin's position
         * @param pos Position in screen coordinates
         */
        void pos(ImVec2 pos) { m_pos = pos; }
    protected:
        std::string m_name;
        ImVec2 m_pos = ImVec2(0.f, 0.f);
        ImVec2 m_size = ImVec2(0.f, 0.f);
        PinType m_type;
        ConnectionFilter m_filter;
        BaseNode* m_parent = nullptr;
        ImNodeFlow* m_inf;
    };

    /**
     * @brief Input specific pin
     * @details Derived from the generic class Pin. The input pin owns the link pointer.
     * @tparam T Data type handled by the pin
     */
    template<class T> class InPin : public Pin
    {
    public:
        /**
         * @brief Input pin constructor
         * @param name Name of the pin
         * @param filter Connection filter
         * @param parent Pointer to the Node containing the pin
         * @param defReturn Default return value when the pin is not connected
         * @param inf Pointer to the Grid Handler the pin is in (same as parent)
         */
        explicit InPin(const std::string& name, ConnectionFilter filter, BaseNode* parent, T defReturn, ImNodeFlow* inf)
            : Pin(name, filter, PinType_Input, parent, inf), m_emptyVal(defReturn) {}

        /**
         * @brief Main loop of the pin
         * @details Updates position, hovering and dragging status, and renders the pin. Must be called each frame.
         */
        void update() override;

        /**
         * @brief Create link between pins
         * @param other Pointer to the other pin
         */
        void createLink(Pin* other) override;

        /**
        * @brief Deletes the link from pin
        */
        void deleteLink() override { m_link.reset(); }

        /**
         * @brief Get pin's link
         * @return Weak_ptr reference to the link connected to the pin
         */
        std::weak_ptr<Link> getLink() override { return m_link; }

        /**
         * @brief Get pin's link attachment point
         * @return Canvas coordinates to the attachment point between the link and the pin
         */
        ImVec2 pinPoint() override { return m_pos + ImVec2(-m_inf->style().node_padding.z, m_size.y / 2); }

        /**
         * @brief Get value carried by the link
         * @return Reference to the value of the connected OutPin. Or the default value if not connected
         */
        const T& val();
    private:
        std::shared_ptr<Link> m_link;
        T m_emptyVal;
    };

    /**
     * @brief Output specific pin
     * @details Derived from the generic class Pin. The output pin handles the logic.
     * @tparam T Data type handled by the pin
     */
    template<class T> class OutPin : public Pin
    {
    public:
        /**
         * Output pin constructor
         * @param name Name of the pin
         * @param filter Connection filter
         * @param parent Pointer to the Node containing the pin
         * @param inf Pointer to the Grid Handler the pin is in (same as parent)
         */
        explicit OutPin(const std::string& name, ConnectionFilter filter, BaseNode* parent, ImNodeFlow* inf)
            :Pin(name, filter, PinType_Output, parent, inf) {}

        /**
         * @brief When parent gets deleted, remove the links
         */
        ~OutPin() { for (auto &l: m_links) if (!l.expired()) l.lock()->right()->deleteLink(); }

        /**
         * @brief Main loop of the pin
         * @details Updates position, hovering and dragging status, and renders the pin. Must be called each frame.
         */
        void update() override;

        /**
         * @brief Create link between pins
         * @param other Pointer to the other pin
         */
        void createLink(Pin* other) override;

        /**
         * @brief Sets the reference to a link
         * @param link Pointer to the link
         */
        void setLink(std::shared_ptr<Link>& link) override;

        /**
         * @brief Deletes any expired pointer to a (now deleted) link
         */
        void deleteLink() override;

        /**
         * @brief Get pin's link attachment point
         * @return Canvas coordinates to the attachment point between the link and the pin
         */
        ImVec2 pinPoint() override { return m_pos + ImVec2(m_size.x + m_inf->style().node_padding.z, m_size.y / 2); }

        /**
         * @brief Calculate and get pin's value
         * @return Reference to the internal value of the pin
         */
        const T& val();

        /**
         * @brief Set logic to calculate output value
         * @details Used to define the pin behaviour. This is what gets the data from the parent's inputs, and applies the needed logic.
         * @param func Function or Lambda to be called by val()
         */
        void behaviour(std::function<T()> func) { m_behaviour = std::move(func); }
    private:
        std::vector<std::weak_ptr<Link>> m_links;
        std::function<T()> m_behaviour;
        T m_val;
    };
}

#include "../src/ImNodeFlow.inl"

#endif
