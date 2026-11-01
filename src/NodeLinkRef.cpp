#include "NodeLinkRef.h"

#include "ImNodeFlow/Context.h"
#include "ImNodeFlow/NodeRef.h"
#include "ImNodeFlow/detail/fwd.h"
#include "ImNodeFlow/fwd.h"

#include "./imgui_bezier_items.h"
#include "./imgui_bezier_widgets.h"
#include "./node.h"
#include "./node_link.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace ImFlow::detail {

template <bool TIsConst>
NodeLinkRef<TIsConst>::NodeLinkRef(context_type& ctx, link_type& link)
    : m_ctx(ctx)
    , m_link(link)
{
}

template <bool TIsConst>
cubic_bezier_controls_t
NodeLinkRef<TIsConst>::getCubicLineBetween(
    ImNodeFlow& ctx,
    ConstNodeRef sourceNode,
    ConstNodeRef targetNode)
{
    ImVec2 start = sourceNode.getTLPos() + sourceNode.getFullSize() * ImVec2(1.F, 0.F);
    ImVec2 end = targetNode.getTLPos();

    start.y += ImGui::GetTextLineHeight();
    end.y += ImGui::GetTextLineHeight();

    return detail::smart_bezier_controls_from_two_points(
        ctx.grid2screen(start),
        ctx.grid2screen(end));
}

template <bool TIsConst>
cubic_bezier_controls_t
NodeLinkRef<TIsConst>::getCubicLineToMouse(
    ImNodeFlow& ctx,
    ConstNodeRef sourceNode)
{
    ImVec2 start = sourceNode.getTLPos() + sourceNode.getFullSize() * ImVec2(1.F, 0.F);
    ImVec2 end = ctx.screen2grid(ImGui::GetMousePos());

    start.y += ImGui::GetTextLineHeight();

    return detail::smart_bezier_controls_from_two_points(
        ctx.grid2screen(start),
        ctx.grid2screen(end));
}

template <bool TIsConst>
bool NodeLinkRef<TIsConst>::isAlive() const
{
    auto& link = m_link.get();
    return !link.marked_for_deletion;
}

template <bool TIsConst>
NodeRefImpl<TIsConst> NodeLinkRef<TIsConst>::sourceNode() const
{
    auto& link = m_link.get();
    return NodeRefImpl<TIsConst> { m_ctx, *link.source_node };
}

template <bool TIsConst>
NodeRefImpl<TIsConst> NodeLinkRef<TIsConst>::targetNode() const
{
    auto& link = m_link.get();
    return NodeRefImpl<TIsConst> { m_ctx, *link.target_node };
}

template <bool TIsConst>
void NodeLinkRef<TIsConst>::connect(NodeRef source_node, NodeRef target_node)
    requires(!TIsConst)
{
    this->destroy();

    node_link_t& link = m_link.get();
    link.marked_for_deletion = false;
    link.source_node = &static_cast<node_t&>(source_node);
    link.target_node = &static_cast<node_t&>(target_node);

    assert(link.source_node != link.target_node
        && "Links cannot be created starting and ending in the same node");

    link.source_node->links.emplace_back(&link);
    link.target_node->links.emplace_back(&link);
}

template <bool TIsConst>
void NodeLinkRef<TIsConst>::destroy()
    requires(!TIsConst)
{
    node_link_t& link = m_link.get();
    if (link.source_node != nullptr) {
        auto iter = std::find(
            std::begin(link.source_node->links),
            std::end(link.source_node->links),
            &link);
        if (iter != std::end(link.source_node->links)) {
            link.source_node->links.erase(iter);
        }
        link.source_node = nullptr;
    }
    if (link.target_node != nullptr) {
        auto iter = std::find(
            std::begin(link.target_node->links),
            std::end(link.target_node->links),
            &link);
        if (iter != std::end(link.target_node->links)) {
            link.target_node->links.erase(iter);
        }
        link.target_node = nullptr;
    }
    link.marked_for_deletion = true;
}

template <bool TIsConst>
void NodeLinkRef<TIsConst>::update()
    requires(!TIsConst)
{
    ImNodeFlow& ctx = m_ctx.get();
    detail::node_link_t& link = m_link.get();

    if (link.marked_for_deletion) {
        return;
    }

    auto line = getCubicLineBetween(ctx, sourceNode(), targetNode());

    if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        link.selected = false;
    }

    auto start_color = sourceNode().getStyle().header_bg;
    auto end_color = targetNode().getStyle().header_bg;

    float thickness = link.hovered
        ? sourceNode().getStyle().link_hovered_thickness
        : sourceNode().getStyle().link_thickness;

    bool pressed = detail::smart_bezier_widget(
        ImGui::GetID(&link),
        line,
        start_color,
        end_color,
        link.selected,
        &link.hovered,
        thickness,
        m_ctx.get().getStyle().link_gradient_blend);

    if (link.hovered) {
        link.selected = link.selected || pressed;
    }

    if (link.selected && ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
        this->destroy();
    }
}

} // namespace ImFlow::detail
