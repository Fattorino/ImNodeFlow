#ifndef IMNODEFLOW_CONTEXT_H
#define IMNODEFLOW_CONTEXT_H
#pragma once

#include "ImNodeFlow/NodeInfo.h"
#include "ImNodeFlow/Style.h"
#include "ImNodeFlow/callback_types.h"
#include "ImNodeFlow/detail/fwd.h"
#include "ImNodeFlow/fwd.h"

#include "imgui.h"

#include <string>
#include <vector>

namespace ImFlow {

/**
 * @brief Main node editor
 * @details Handles the infinite grid, nodes and links. Also handles all the logic.
 */
class ImNodeFlow {
private:
    static int m_instances;

public:
    /**
     * @brief Instantiate a new editor with default name.
     * Editor name will be "FlowGrid + the number of editors"
     */
    ImNodeFlow();

    /**
     * @brief Instantiate a new editor with given name
     * @details Creates a new Node Editor with the given name.
     * @param name Name of the editor
     */
    explicit ImNodeFlow(std::string name);

    // delete copy-ctor
    ImNodeFlow(const ImNodeFlow&) = delete;
    // delete copy-operator
    ImNodeFlow& operator=(const ImNodeFlow&) = delete;

    // move-ctor
    ImNodeFlow(ImNodeFlow&&) noexcept;
    // move-operator
    ImNodeFlow& operator=(ImNodeFlow&&);

    ~ImNodeFlow();

    /**
     * @brief Handler loop
     * @details Main update function. Refreshes all the logic and draws everything. Must be called every frame.
     */
    void update();

    /**
     * @brief Add a node to the grid
     * @param grid_coords Position of the Node in grid coordinates
     * @param node_info Node behaviour customization options
     * @return Reference to the newly added node. Do not save this
     * reference.
     */
    NodeRef addNode(const ImVec2& grid_coords, NodeInfo node_info = { });

    /**
     * @brief Add a node to the grid
     * @param screen_coords Position of the Node in screen coordinates
     * @param node_info Node behaviour customization options
     * @return Reference to the newly added node. Do not save this
     * reference.
     */
    NodeRef placeNodeAt(const ImVec2& screen_coords, NodeInfo node_info = { });

    /**
     * @brief Add a node to the grid using mouse position
     * @param args Optional arguments to be forwarded to derived class ctor
     * @param node_info Node behaviour customization options
     * @return Reference to the newly added node. Do not save this
     * reference.
     */
    NodeRef placeNode(NodeInfo node_info = { });

    /**
     * @brief Add a node whos operation can be defined within a lambda.
     * @tparam L the type of the lambda
     * @param grid_coords the position at which to place the node
     * @param draw_func the draw_func that defines the nodes operation
     * @return Reference to the newly added node. Do not save this
     * reference.
     */
    NodeRef addNodeLambda(
        const ImVec2& grid_coords,
        lambda_draw_func_t draw_func);

    /**
     * @brief Add a node to the grid
     * @tparam T Derived class of <INodeBehaviour> to be added
     * @tparam Params types of optional args to forward to derived class ctor
     * @param grid_coords Position of the Node in grid coordinates
     * @param args Arguments to be forwarded to derived class ctor
     * @return Reference to the newly added node. Do not save this
     * reference.
     */
    template <typename T, typename... Params>
    NodeRef emplaceNode(const ImVec2& grid_coords, Params&&... args);

    /**
     * @brief Add a node to the grid
     * @tparam T Derived class of <INodeBehaviour> to be added
     * @tparam Params types of optional args to forward to derived class ctor
     * @param screen_coords Position of the Node in screen coordinates
     * @param args Arguments to be forwarded to derived class ctor
     * @return Reference to the newly added node. Do not save this
     * reference.
     */
    template <typename T, typename... Params>
    NodeRef emplaceNodeAt(const ImVec2& screen_coords, Params&&... args);

    /**
     * @brief Add a node to the grid using mouse position
     * @tparam T Derived class of <Node> to be added
     * @tparam Params types of optional args to forward to derived class ctor
     * @param args Arguments to be forwarded to derived class ctor
     * @return Reference to the newly added node. Do not save this
     * reference.
     */
    template <typename T, typename... Params>
    NodeRef emplaceNodeOnMouse(Params&&... args);

    [[nodiscard]] bool canLink(ConstPinRef source_pin, ConstPinRef target_pin) const;

    [[nodiscard]] bool canLink(ConstNodeRef source_node, ConstNodeRef target_node) const;

    /**
     * @brief Creates a link between two pins
     * @param source_pin Reference to the input pin
     * @param target_pint Reference to the output pin
     */
    void createLink(PinRef source_pin, PinRef target_pin);

    void createLink(NodeRef source_node, NodeRef target_node);

    /**
     * @brief Pop-up when link is "dropped"
     * @details Sets the content of a pop-up that can be displayed when dragging a link in the open instead of onto another pin.
     * @details If "key = ImGuiKey_None" the pop-up will always open when a link is dropped.
     * @param content Function or Lambda containing only the contents of the pop-up and the subsequent logic
     * @param key Optional key required in order to open the pop-up
     */
    void droppedLinkPopUpContent(
        dropped_link_popup_handler_t handler,
        ImGuiKey key = ImGuiKey_None);

    /**
     * @brief Pop-up when right-clicking
     * @details Sets the content of a pop-up that can be displayed when right-clicking on the grid.
     * @param content Function or Lambda containing only the contents of the pop-up and the subsequent logic
     */
    void rightClickPopUpContent(right_click_popup_handler_t content);

    /**
     * @brief <BR>Pop-up when drag_dropping
     * @details Sets the node pos when drag dropping a node to the grid
     * @param content Function or Lambda containing only the pos of the mouse when clicked
     */
    void dragDropTarget(drag_drop_target_handler_t handler);

    /**
     * @brief Get editor's name
     * @return Const reference to editor's name
     */
    [[nodiscard]] const std::string& getName() const;

    /**
     * @brief Get editor's position
     * @return Const reference to editor's position in screen coordinates
     */
    [[nodiscard]] const ImVec2& getPos() const;

    /**
     * @brief Get editor's grid scroll
     * @details Scroll is the offset from the origin of the grid, changes while navigating the grid.
     * @return Const reference to editor's grid scroll
     */
    const ImVec2& getScroll() const;

    /**
     * @brief Sets the context's scroll to a user specified value
     */
    void setScroll(ImVec2 scroll);

    /**
     * @brief Sets the context's scale to a user specified value
     */
    void setScale(float scale);

    /**
     * @brief Get the scale adjusted screen space mouse delta, needed for dragging
     *
     * @return scale adjusted mouse delta.
     */
    ImVec2 getScreenSpaceDelta() const;

    /**
     * @brief Get nodes count
     * @return Number of nodes present in the editor
     */
    uint64_t getNodesCount() const;

    uint64_t getSelectedNodesCount() const;

    /// PERF: Getting all nodes is quite slow
    std::vector<NodeRef> getNodes();

    /// PERF: Getting all selected nodes is quite slow
    std::vector<NodeRef> getSelectedNodes();

    /**
     * @brief Get zooming viewport
     * @return Const reference to editor's internal viewport for zoom support
     */
    IContainedContext& getGrid();

    /**
     * @brief Get current style
     * @return Reference to style variables
     */
    const InfStyler& getStyle() const;

    /**
     * @brief Modify current style
     */
    void setStyle(InfStyler style);

    /**
     * @brief Set editor's size
     * @param size Editor's size. Set to (0, 0) to auto-fit.
     */
    void setSize(const ImVec2& size);

    /**
     * FIXME: Remove methods for internal use from public API
     * @internal For internal use
     */
    void startNodeDrag();

    /**
     * FIXME: Remove methods for internal use from public API
     * @internal For internal use
     */
    void stopNodeDrag();

    /**
     * FIXME: Remove methods for internal use from public API
     * @internal For internal use
     */
    bool wantsNodeDrag();

    /**
     * FIXME: Remove methods for internal use from public API
     * @internal For internal use
     */
    bool getNodeDragAmmount(ImVec2* out_vec_amount);

    /**
     * @brief Draws the right click popup menu
     * @param hovered_node Pointer to the hovered node
     * @internal For internal use only
     */
    void drawRightClickPopup(detail::node_t* hovered_node);

    /**
     * @brief Convert coordinates from screen to grid
     * @param p Point in screen coordinates to be converted
     * @return Point in grid's coordinates
     */
    ImVec2 screen2grid(const ImVec2& p) const;

    /**
     * @brief Convert coordinates from grid to screen
     * @param p Point in grid's coordinates to be converted
     * @return Point in screen coordinates
     */
    ImVec2 grid2screen(const ImVec2& p) const;

    /**
     * @brief Check if mouse is on selected node
     * @return [TRUE] if the mouse is hovering a selected node
     */
    bool mouse_on_selected_node() const;

    ImVec2 slamPosition(ImVec2 grid_position) const;

    /**
     * FIXME: Remove all internal use methods from the public API
     * @internal For internal use only
     */
    detail::editor_context_t& getDetail();

private:
    std::unique_ptr<detail::editor_context_t> m_impl;
};

} // namespace ImFlow

#include "./Context.inl"

#endif // IMNODEFLOW_CONTEXT_H
