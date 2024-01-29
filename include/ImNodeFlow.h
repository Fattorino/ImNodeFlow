#ifndef IM_NODE_FLOW
#define IM_NODE_FLOW
#pragma once

#include <string>
#include <utility>
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>
#include <functional>
#include <imgui.h>
#include "../src/imgui_bezier_math.h"

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
    inline void smart_bezier(const ImVec2& p1, const ImVec2& p2, ImU32 color, float thickness);

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
    inline bool smart_bezier_collider(const ImVec2& p, const ImVec2& p1, const ImVec2& p2, float radius);

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
     * @details List of, ready to use,basic filters. It's possible to create more filters with the help of "ConnectionFilter_MakeCustom".
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
        ImU32 pin_bg = IM_COL32(23, 16, 16, 0); // Color of the background of the Pin
        ImU32 pin_hovered = IM_COL32(100, 100, 255, 70); // Color of the overlay to be displayed when Pin is hovered
        ImU32 pin_border = IM_COL32(255, 255, 255, 0); // Color of the border to be displayed around the Pin
        ImU32 pin_point = IM_COL32(255, 255, 240, 230); // Color of the border to be displayed around the Pin

        ImU32 link = IM_COL32(230, 230, 200, 230); // Color of the link
        ImU32 drag_out_link = IM_COL32(230, 230, 200, 230); // Color of the link while dragging
        ImU32 link_selected_outline = IM_COL32(80, 20, 255, 200); // Color of the outline of a selected link

        ImU32 node_bg = IM_COL32(97, 103, 122, 100); // Color of the background of the node's body
        ImU32 node_header = IM_COL32(23, 16, 16, 150); // Background of the node's header
        ImColor node_header_title = ImColor(255, 246, 240, 255); // Text in the node's header
        ImU32 node_border = IM_COL32(100, 100, 100, 255); // Node border
        ImU32 node_selected_border = IM_COL32(170, 190, 205, 230); // Node border when it's selected

        ImU32 background = IM_COL32(44, 51, 51, 255); // Background of the grid
        ImU32 grid = IM_COL32(200, 200, 200, 40); // Color of the main lines of the grid
        ImU32 subGrid = IM_COL32(200, 200, 200, 10); // Color of the secondary lines
    };

    /**
     * @brief ALl the Styling parameters. Sizes + Colors
     */
    struct InfStyler
    {
        ImVec2 pin_padding = ImVec2(3.f, 1.f); // Padding between Pin border and content
        float pin_radius = 8.f; // Pin's edges rounding
        float pin_border_thickness = 1.f; // Thickness of the border drawn around the Pin
        float pin_point_radius = 3.5f; // Radius of the circle in front of the Pin

        float link_thickness = 2.6f; // Thickness of the drawn link
        float link_hovered_thickness = 3.5f; // Thickness of the drawn link when hovered
        float link_selected_outline_thickness = 0.5f; // Thickness of the outline of a selected Link
        float drag_out_link_thickness = 2.f; // Thickness of the dummy link while dragging

        ImVec4 node_padding = ImVec4(9.f, 6.f, 9.f, 2.f); // Padding of Node's content (Left Top Right Bottom)
        float node_radius = 8.f; // Node's edges rounding
        float node_border_thickness = 1.f; // Node's border thickness
        float node_border_selected_thickness = 2.f; // Node's border thickness when selected

        float grid_size = 50.f; // Size of main grid
        float grid_subdivisions = 5.f; // Sub-grid divisions for Node snapping

        InfColors colors; // ImNodeFlow colors
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
         * @brief Instantiate a new editor
         * @details Empty constructor, creates a new Node Editor. Editor name will be "FlowGrid + the number of editors".
         */
        ImNodeFlow() { m_name = "FlowGrid" + std::to_string(m_instances); m_instances++; }

        /**
         * @brief Instantiate a new editor
         * @details Creates a new Node Editor with the given name.
         * @param name Name of the editor
         */
        explicit ImNodeFlow(std::string name) :m_name(std::move(name)) { m_instances++; }

        /**
         * @brief Instantiate a new editor
         * @details Creates a new Node Editor with the given name.
         * @param name Name of the editor
         */
        explicit ImNodeFlow(const char* name) :m_name(name) { m_instances++; }

        /**
         * @brief Handler loop
         * @details Main update function. Refreshes all the logic and draws everything. Must be called every frame.
         */
        void update();

        /**
         * @brief Adds a node to the editor
         * @tparam T Derived class of <BaseNode> to be added
         * @param name Name to be given to the Node
         * @param pos Position of the Node in canvas coordinates
         * @return Pointer of the pushed type to the newly added Node
         *
         * Inheritance is checked at compile time, <T> MUST be derived from BaseNode.
         */
        template<typename T>
        T* addNode(const std::string& name, const ImVec2& pos);

        /**
         * @brief Adds a node to the editor
         * @tparam T Derived class of <BaseNode> to be added
         * @param name Name to be given to the Node
         * @param pos Position of the Node in screen coordinates
         * @return Pointer of the pushed type to the newly added Node
         *
         * Inheritance is checked at compile time, <T> MUST be derived from BaseNode.
         */
        template<typename T>
        T* dropNode(const std::string& name, const ImVec2& pos);

        /**
         * @brief Get nodes count
         * @return Number of nodes present in the editor
         */
        int nodesCount() { return (int)m_nodes.size(); }

        /**
         * @brief Creates a link between two pins
         * @details Creates a link. Will check for same node connections, IN to IN or OUT to OUT connections and evaluate the filters.
         *          Then the link will be created and the input pin of the two will own the link.
         * @param start Pointer to the start pin to be connected
         * @param end Pointer to the end pin to be connected
         */
        void createLink(Pin* start, Pin* end);

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
        const ImVec2& pos() { return m_pos; }

        /**
         * @brief Get editor's grid scroll
         * @details Scroll is the offset from the origin of the grid, changes while navigating the grid with the middle mouse.
         * @return Const reference to editor's grid scroll
         */
        const ImVec2& scroll() { return m_scroll; }

        /**
         * @brief Get editor's list of nodes
         * @return Const reference to editor's internal nodes list
         */
        const std::vector<std::shared_ptr<BaseNode>>& nodes() { return m_nodes; }

        /**
         * @brief Get editor's list of links
         * @return Const reference to editor's internal links list
         */
        const std::vector<std::weak_ptr<Link>>& links() { return m_links; }

        /**
         * @brief Get dragging status
         * @return [TRUE] if a Node is being dragged around the grid
         */
        [[nodiscard]] bool draggingNode() const { return m_draggingNode; }

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

        /**
         * @brief Get current style
         * @return Reference to style variables
         */
        InfStyler& style() { return m_style; }
    private:
        std::string m_name;
        ImVec2 m_pos;
        ImVec2 m_scroll = ImVec2(0, 0);

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
        explicit BaseNode(std::string name, ImVec2 pos, ImNodeFlow* inf)
            :m_name(std::move(name)), m_pos(pos), m_inf(inf)
        {
            m_paddingTL = {m_inf->style().node_padding.x, m_inf->style().node_padding.y};
            m_paddingBR = {m_inf->style().node_padding.z, m_inf->style().node_padding.w};
        }

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
         * @tparam T Type of the data the pin will handle
         * @param name Name of the pin
         * @param defReturn Default return value when the pin is not connected
         * @param filter Connection filter
         */
        template<typename T>
        void addIN(std::string name, T defReturn, ConnectionFilter filter = ConnectionFilter_None);

        /**
         * @brief Add an Output to the node
         * @details Must be called in the node constructor. WIll add an Output pin to the node with the given name and data type.
         * @tparam T Type of the data the pin will handle
         * @param name Name of the pin
         * @param filter Connection filter
         * @return Pointer to the newly added pin. Must be used to set behaviour
         */
        template<typename T>
        [[nodiscard]] OutPin<T>* addOUT(std::string name, ConnectionFilter filter = ConnectionFilter_None);

        /**
         * @brief Get Input value
         * @details Get a reference to the value, the value is stored in the output pin at the other end of the link.
         * @tparam T Data type
         * @param i Index of the pin
         * @return Reference to the value
         */
        template<typename T>
        const T& ins(int i);

        /**
         * @brief Get generic reference to input pin
         * @param i Index of the pin
         * @return Generic pointer to the pin
         */
        Pin* ins(int i) { return m_ins[i].get(); }

        /**
         * @brief Get generic reference to output pin
         * @param i Index of the pin
         * @return Generic pointer to the pin
         */
        Pin* outs(int i) { return m_outs[i].get(); }

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
        ImVec2 m_pos, m_posOld = m_pos;
        ImVec2 m_size;
        ImNodeFlow* m_inf;
        bool m_selected = false, m_selectedNext = false;
        bool m_dragged = false;
        ImVec2 m_paddingTL;
        ImVec2 m_paddingBR;

        std::vector<std::shared_ptr<Pin>> m_ins;
        std::vector<std::shared_ptr<Pin>> m_outs;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // PINS

    /**
     * @brief Pins type identifier
     */
    enum PinKind
    {
        PinKind_Input,
        PinKind_Output
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
        explicit Pin(std::string name, ConnectionFilter filter, PinKind kind, BaseNode* parent, ImNodeFlow* inf)
            :m_name(std::move(name)), m_filter(filter), m_kind(kind), m_parent(parent), m_inf(inf) {}

        /**
         * @brief Main loop of the pin
         * @details Updates position, hovering and dragging status, and renders the pin. Must be called each frame.
         */
        virtual void update() = 0;

        /**
         * @brief Create link between pins
         * @param left Pointer to the other pin
         */
        virtual void createLink(Pin* left) {}

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
        PinKind kind() { return m_kind; }

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
        PinKind m_kind;
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
            :Pin(name, filter, PinKind_Input, parent, inf), m_emptyVal(defReturn) {}

        /**
         * @brief Main loop of the pin
         * @details Updates position, hovering and dragging status, and renders the pin. Must be called each frame.
         */
        void update() override;

        /**
         * @brief Create link between pins
         * @param left Pointer to the other pin
         */
        void createLink(Pin* left) override;

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
            :Pin(name, filter, PinKind_Output, parent, inf) {}

        /**
         * @brief When parent gets deleted, remove the link
         */
        ~OutPin() { if (!m_link.expired()) m_link.lock()->right()->deleteLink(); }

        /**
         * @brief Main loop of the pin
         * @details Updates position, hovering and dragging status, and renders the pin. Must be called each frame.
         */
        void update() override;

        /**
         * @brief Sets the reference to a link
         * @param link Pointer to the link
         */
        void setLink(std::shared_ptr<Link>& link) override { m_link = link; }

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
        std::weak_ptr<Link> m_link;
        std::function<T()> m_behaviour;
        T m_val;
    };
}

#include "../src/ImNodeFlow.inl"

#endif
