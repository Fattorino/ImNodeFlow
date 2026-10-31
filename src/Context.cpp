#include "ImNodeFlow/Context.h"

#include "ImNodeFlow/NodeGroupArenaRef.h"
#include "ImNodeFlow/NodeRef.h"
#include "ImNodeFlow/PinRef.h"
#include "ImNodeFlow/fwd.h"

#include "./NodeLinkRef.h"
#include "./NodeWrapper.h"
#include "./PinLinkRef.h"
#include "./editor_context.h"
#include "./imgui_button_flags_helper.h"
#include "./node.h"
#include "./node_link.h"
#include "./pin_drawing_utils.h"
#include "./pin_link.h"
#include "./uid_helpers.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

namespace ImFlow {

int ImNodeFlow::m_instances = 0;

NodeRef ImNodeFlow::addNode(const ImVec2& grid_coords, NodeInfo node_info)
{
    auto uid = detail::generate_node_uid();

    auto node_pos = this->slamPosition(grid_coords);
    auto new_node = std::make_unique<detail::node_t>(
        detail::node_t {
            .uid = uid,
            .pos = node_pos,
            .posTarget = node_pos,
            .style = std::move(node_info.style),
            .behaviour = std::move(node_info.behaviour),
        });

    auto& container = m_impl->is_drawing
        ? m_impl->newNodesQueue
        : m_impl->root_arena.nodes;
    auto& node = container.emplace_back(std::move(new_node));
    if (node->behaviour) {
        node->behaviour->setup(NodeRef { *this, *node });
    }

    return NodeRef { *this, *node };
}

NodeRef ImNodeFlow::placeNodeAt(const ImVec2& screen_coords, NodeInfo node_info)
{
    return addNode(screen2grid(screen_coords), std::move(node_info));
}

NodeRef ImNodeFlow::placeNode(NodeInfo node_info)
{
    return placeNodeAt(ImGui::GetMousePos(), std::move(node_info));
}

NodeRef ImNodeFlow::addNodeLambda(const ImVec2& grid_coords, lambda_draw_func_t draw_func)
{
    return addNode(
        grid_coords,
        NodeInfo {
            .behaviour = std::make_unique<detail::NodeWrapper>(std::move(draw_func)),
        });
}

ImNodeFlow::ImNodeFlow()
    : ImNodeFlow("FlowGrid" + std::to_string(m_instances))
{
}

ImNodeFlow::ImNodeFlow(std::string name)
    : m_impl(std::make_unique<detail::editor_context_t>(
          detail::editor_context_t {
              .name { std::move(name) },
          }))
{
    m_instances++;
    m_impl->context.config().extra_window_wrapper = true;
    m_impl->context.config().color = m_impl->style.colors.background;
}

ImNodeFlow::ImNodeFlow(ImNodeFlow&&) noexcept = default;
ImNodeFlow& ImNodeFlow::operator=(ImNodeFlow&&) = default;

ImNodeFlow::~ImNodeFlow() = default;

bool ImNodeFlow::mouse_on_selected_node() const
{
    return m_impl->mouseOnSelectedNode;
}

ImVec2 ImNodeFlow::slamPosition(ImVec2 grid_position) const
{
    auto const& style = this->getStyle();
    float step = style.grid_size / style.grid_subdivisions;
    return ImVec2 {
        round(grid_position.x / step) * step,
        round(grid_position.y / step) * step,
    };
}

detail::editor_context_t& ImNodeFlow::getDetail()
{
    return *m_impl;
}

ImVec2
ImNodeFlow::screen2grid(const ImVec2& p) const
{
    if (ImGui::GetCurrentContext() == m_impl->context.getRawContext())
        return p - m_impl->context.scroll();
    return (p - m_impl->context.origin()) / m_impl->context.scale() - m_impl->context.scroll();
}

ImVec2
ImNodeFlow::grid2screen(const ImVec2& p) const
{
    if (ImGui::GetCurrentContext() == m_impl->context.getRawContext())
        return p + m_impl->context.scroll();
    return (p + m_impl->context.scroll()) * m_impl->context.scale() + m_impl->context.origin();
}

bool ImNodeFlow::canLink(ConstPinRef source_pin, ConstPinRef target_pin) const
{
    if (source_pin == target_pin) {
        return false;
    }

    if (source_pin.getType() == target_pin.getType()) {
        // Cannot link inputs to inputs nor outputs to outputs
        return false;
    }

    bool same_node_allowed = source_pin.allowsSameNodeConnections()
        && target_pin.allowsSameNodeConnections();
    if (source_pin.getParent() == target_pin.getParent() && !same_node_allowed) {
        return false;
    }

    for (const auto& pin_ref : source_pin.getConnections()) {
        if (pin_ref == target_pin) {
            // Connection already exists
            return false;
        }
    }

    // Only pins with the same type can be linked. Casts between
    // compatible types like (float and double), or (short, int, and long)
    // should be handled explicitly by user types.
    //
    // On a technical note, supporting these casts is not necessarily hard,
    // but introduces a layer of complexity which either requires RTTI (so
    // we know how to type-cast), or manual RTTI (using a compile-time
    // type_id<T>, which is possible, but we are now responsible for
    // maintaining a list of which type conversions should be possible and
    // which aren't), or it might introduce subtle programming errors
    // given the nature that different types might naturally have different
    // sizes in memory, and getting a cast wrong may result in writing
    // into out-of-bounds memory.
    //
    // Supporting connections only between same types avoids all these
    // issues, and it's not something that's prohibitibly hard to work
    // around on the client side.
    return source_pin.getTypeID() == target_pin.getTypeID();
}

bool ImNodeFlow::canLink(ConstNodeRef source_node, ConstNodeRef target_node) const
{
    if (source_node == target_node) {
        return false;
    }

    for (const auto& node_ref : source_node.getConnections()) {
        if (node_ref == target_node) {
            // Connection already exists
            return false;
        }
    }

    return true;
}

void ImNodeFlow::createLink(PinRef source_pin, PinRef target_pin)
{
    if (!canLink(source_pin, target_pin)) {
        return;
    }

    auto new_link = std::make_unique<detail::pin_link_t>();
    detail::PinLinkRef<> { *this, *new_link }.connect(source_pin, target_pin);
    m_impl->pin_links.emplace_back(std::move(new_link));
}

void ImNodeFlow::createLink(NodeRef source_node, NodeRef target_node)
{
    if (!canLink(source_node, target_node)) {
        return;
    }

    auto new_link = std::make_unique<detail::node_link_t>();
    detail::NodeLinkRef<> { *this, *new_link }.connect(source_node, target_node);
    m_impl->node_links.emplace_back(std::move(new_link));
}

void ImNodeFlow::droppedLinkPopUpContent(
    dropped_link_popup_handler_t handler,
    ImGuiKey key)
{
    m_impl->droppedLinkPopUp = std::move(handler);
    m_impl->droppedLinkPopUpComboKey = key;
}

void ImNodeFlow::rightClickPopUpContent(right_click_popup_handler_t content)
{
    m_impl->rightClickPopUp = std::move(content);
}

void ImNodeFlow::dragDropTarget(drag_drop_target_handler_t content)
{
    m_impl->dragdropTarget = std::move(content);
}

const std::string& ImNodeFlow::getName() const
{
    return m_impl->name;
}

const ImVec2& ImNodeFlow::getPos() const
{
    return m_impl->context.origin();
}

const ImVec2& ImNodeFlow::getScroll() const
{
    return m_impl->context.scroll();
}

void ImNodeFlow::setScroll(ImVec2 scroll)
{
    m_impl->context.setScroll(scroll);
}

void ImNodeFlow::setScale(float scale)
{
    m_impl->context.setScale(scale);
}

ImVec2 ImNodeFlow::getScreenSpaceDelta() const
{
    return m_impl->context.getScreenDelta();
}

uint64_t ImNodeFlow::getNodesCount() const
{
    return NodeGroupArenaRef<true> { *this, m_impl->root_arena }.getNodesCount();
}

uint64_t ImNodeFlow::getSelectedNodesCount() const
{
    return NodeGroupArenaRef<true> { *this, m_impl->root_arena }.getSelectedNodesCount();
}

std::vector<NodeRef> ImNodeFlow::getNodes()
{
    std::vector<NodeRef> result;
    result.reserve(this->getNodesCount());
    NodeGroupArenaRef { *this, m_impl->root_arena }.getNodes(result);
    return result;
}

std::vector<NodeRef> ImNodeFlow::getSelectedNodes()
{
    std::vector<NodeRef> result;
    result.reserve(this->getSelectedNodesCount());
    NodeGroupArenaRef { *this, m_impl->root_arena }.getSelectedNodes(result);
    return result;
}

IContainedContext& ImNodeFlow::getGrid()
{
    return m_impl->context;
}

const InfStyler& ImNodeFlow::getStyle() const
{
    return m_impl->style;
}

void ImNodeFlow::setStyle(InfStyler style)
{
    m_impl->style = std::move(style);
}

void ImNodeFlow::setSize(const ImVec2& size)
{
    m_impl->context.config().size = size;
}

void ImNodeFlow::startNodeDrag()
{
    m_impl->nodeDragStart = this->screen2grid(ImGui::GetMousePos());
    m_impl->nodeDragEnd = this->screen2grid(ImGui::GetMousePos());
    m_impl->nodeDragActive = true;
    m_impl->nodeDragActiveNext = true;
    m_impl->nodeDragActiveNext2 = true;
}

void ImNodeFlow::stopNodeDrag()
{
    m_impl->nodeDragActiveNext2 = false;
}

bool ImNodeFlow::wantsNodeDrag()
{
    return m_impl->nodeDragActive;
}

bool ImNodeFlow::getNodeDragAmmount(ImVec2* out_vec_amount)
{
    bool should_apply_drag = m_impl->nodeDragActive && !m_impl->nodeDragActiveNext;
    if (m_impl->nodeDragActive) {
        if (out_vec_amount != nullptr) {
            *out_vec_amount = m_impl->nodeDragEnd - m_impl->nodeDragStart;
        }
    } else {
        if (out_vec_amount != nullptr) {
            *out_vec_amount = ImVec2 { 0.F, 0.F };
        }
    }
    return should_apply_drag;
}

void ImNodeFlow::hoveredNode(detail::node_t* hovering)
{
    m_impl->hoveredNode = hovering;
}

void ImNodeFlow::update()
{
    m_impl->is_drawing = true;

    m_impl->mouseOnSelectedNode
        = NodeGroupArenaRef<true> { *this, m_impl->root_arena }
              .mouse_over_any_selected_node();

    // Updating looping stuff
    m_impl->hoveredNode = nullptr;
    m_impl->nodeDragActive = m_impl->nodeDragActiveNext;
    m_impl->nodeDragActiveNext = m_impl->nodeDragActiveNext2;

    if (m_impl->nodeDragActive) {
        m_impl->nodeDragEnd = this->screen2grid(ImGui::GetMousePos());
    }

    // Create child canvas
    m_impl->context.begin();
    ImGui::GetIO().IniFilename = nullptr;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Display grid
    ImVec2 gridSize = ImGui::GetWindowSize();
    float subGridStep = m_impl->style.grid_size / m_impl->style.grid_subdivisions;
    for (float x = fmodf(m_impl->context.scroll().x, m_impl->style.grid_size);
        x < gridSize.x;
        x += m_impl->style.grid_size)
        draw_list->AddLine(
            ImVec2(x, 0.0f),
            ImVec2(x, gridSize.y),
            m_impl->style.colors.grid);
    for (float y = fmodf(m_impl->context.scroll().y, m_impl->style.grid_size);
        y < gridSize.y;
        y += m_impl->style.grid_size)
        draw_list->AddLine(
            ImVec2(0.0f, y),
            ImVec2(gridSize.x, y),
            m_impl->style.colors.grid);
    if (m_impl->context.scale() > 0.7f) {
        for (float x = fmodf(m_impl->context.scroll().x, subGridStep);
            x < gridSize.x;
            x += subGridStep)
            draw_list->AddLine(
                ImVec2(x, 0.0f),
                ImVec2(x, gridSize.y),
                m_impl->style.colors.subGrid);
        for (float y = fmodf(m_impl->context.scroll().y, subGridStep);
            y < gridSize.y;
            y += subGridStep)
            draw_list->AddLine(
                ImVec2(0.0f, y),
                ImVec2(gridSize.x, y),
                m_impl->style.colors.subGrid);
    }

    // Propagate pin values
    NodeGroupArenaRef<> root_arena { *this, m_impl->root_arena };
    root_arena.calculate_inputs();
    root_arena.prime_inputs();
    root_arena.calculate_outputs();
    root_arena.prime_outputs();

    // Update and draw nodes
    root_arena.update();

    root_arena.prepareNextFrame();

    // Update and draw links
    for (auto iter = m_impl->pin_links.begin(); iter != m_impl->pin_links.end();) {
        detail::PinLinkRef<> link { *this, **iter };

        // Clean up deleted links along the way
        if (!link.isAlive()) {
            link.destroy();
            iter = m_impl->pin_links.erase(iter);
            continue;
        }

        link.update();
        ++iter;
    }

    for (auto& l : m_impl->node_links) {
        detail::NodeLinkRef<> link { *this, *l };

        if (!link.isAlive()) {
            link.destroy();
            continue;
        }

        link.update();
    }

    ImRect always_valid_bb {
        ImGui::GetMousePos() - ImVec2(1.F, 1.F),
        ImGui::GetMousePos() + ImVec2(1.F, 1.F),
    };

    auto const& grid_cfg = getGrid().config();

    int mouse_context_button_flags = ImGuiButtonFlags_PressedOnClick
        | detail::getButtonFlagsForMouseButton(grid_cfg.selection_box_button)
        | detail::getButtonFlagsForMouseButton(grid_cfg.scroll_button);

    bool mouse_context_hovered = false;
    bool mouse_context_held = false;
    bool mouse_context_pressed = false;
    ImGuiID mouse_context_id = ImGui::GetID("##mouse_context");

    ImGui::ItemAdd(always_valid_bb, mouse_context_id);
    mouse_context_pressed = ImGui::ButtonBehavior(
        always_valid_bb,
        mouse_context_id,
        &mouse_context_hovered,
        &mouse_context_held,
        mouse_context_button_flags);

    auto active_id_mouse_button = ImGui::GetCurrentContext()->ActiveIdMouseButton;
    if (mouse_context_pressed) {
        if (active_id_mouse_button == grid_cfg.selection_box_button) {
            m_impl->box.active = true;
            m_impl->box.selectionStart = this->screen2grid(ImGui::GetMousePos());

            // Deselect all nodes if not holding down CTRL
            if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && !ImGui::IsKeyDown(ImGuiKey_RightCtrl)) {
                root_arena.deselectAllNodes();
            }
        }
    }

    if (mouse_context_hovered && ImGui::BeginDragDropTarget()) {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PIN_REF_LOCAL_DTO", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
        if (payload != nullptr) {
            PinRefLocalDTO<> dto { };
            IM_ASSERT(payload->DataSize == sizeof(dto));
            std::memcpy(&dto, payload->Data, sizeof(dto));
            PinRef source_pin { dto };
            // Links drop-off
            if (m_impl->droppedLinkPopUp
                && (m_impl->droppedLinkPopUpComboKey == ImGuiKey_None
                    || ImGui::IsKeyDown(m_impl->droppedLinkPopUpComboKey))) {
                m_impl->droppedLinkLeft = &static_cast<detail::pin_base_t&>(source_pin);
                ImGui::OpenPopup("DroppedLinkPopUp");
            }
        }
        ImGui::EndDragDropTarget();
    }

    // Dropped Link PopUp
    if (ImGui::IsPopupOpen("DroppedLinkPopUp")) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImDrawListSplitter splitter;
        splitter.Split(draw_list, 2);
        splitter.SetCurrentChannel(draw_list, 1);

        ImVec2 mouse_pos_on_popup;
        if (ImGui::BeginPopup("DroppedLinkPopUp")) {
            mouse_pos_on_popup = ImGui::GetMousePosOnOpeningCurrentPopup();
            m_impl->droppedLinkPopUp(PinRef { *this, *m_impl->droppedLinkLeft });
            ImGui::EndPopup();
        }

        splitter.SetCurrentChannel(draw_list, 0);
        detail::draw_bezier_from_pin_to_pos(
            ImGui::GetWindowDrawList(),
            ConstPinRef { *this, *m_impl->droppedLinkLeft },
            mouse_pos_on_popup);

        splitter.Merge(draw_list);
    }

    if (mouse_context_held
        && grid_cfg.selection_box_button != grid_cfg.scroll_button
        && active_id_mouse_button == grid_cfg.scroll_button) {
        m_impl->context.updateScroll();
    }

    if (m_impl->box.active) {
        ImRect boxClip { m_impl->box.selectionStart, m_impl->box.selectionStart };
        boxClip.Add(this->screen2grid(ImGui::GetMousePos()));

        draw_list->AddRectFilled(
            this->grid2screen(boxClip.GetBL()),
            this->grid2screen(boxClip.GetTR()),
            m_impl->box.selectionColor);
        draw_list->AddRect(
            this->grid2screen(boxClip.GetBL()),
            this->grid2screen(boxClip.GetTR()),
            m_impl->box.borderColor,
            0.0F,
            0,
            1.5F);

        bool shouldMarkSelected = false;

        if (!mouse_context_held) {
            m_impl->box.active = false;
            shouldMarkSelected = true;
        } else if (
            grid_cfg.selection_box_cancel_button != grid_cfg.selection_box_button && ImGui::IsMouseClicked(grid_cfg.selection_box_cancel_button)) {
            // cancel selection
            m_impl->box.active = false;
        }

        if (shouldMarkSelected) {
            root_arena.selectArea(boxClip);
        } else {
            root_arena.provisionallySelectArea(boxClip);
        }
    }

    // Right-click PopUp
    if (m_impl->rightClickPopUp && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered()) {
        m_impl->hoveredNodeAux = m_impl->hoveredNode;
        ImGui::OpenPopup("RightClickPopUp");
    }
    if (ImGui::BeginPopup("RightClickPopUp")) {
        std::optional<NodeRef> arg;
        if (m_impl->hoveredNodeAux != nullptr) {
            arg.emplace(*this, *m_impl->hoveredNodeAux);
        }
        m_impl->rightClickPopUp(std::move(arg));
        ImGui::EndPopup();
    }

    m_impl->context.end();

    if (ImGui::BeginDragDropTarget()) {
        if (m_impl->dragdropTarget) {
            m_impl->dragdropTarget(screen2grid(ImGui::GetMousePos()));
        }

        ImGui::EndDragDropTarget();
    }

    m_impl->is_drawing = false;

    if (!m_impl->newNodesQueue.empty()) {
        std::move(
            std::begin(m_impl->newNodesQueue),
            std::end(m_impl->newNodesQueue),
            std::back_inserter(m_impl->root_arena.nodes));
        m_impl->newNodesQueue.clear();
    }
}

} // namespace ImFlow
