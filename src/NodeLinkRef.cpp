#include "NodeLinkRef.h"

#include "ImNodeFlow/Context.h"
#include "ImNodeFlow/NodeRef.h"
#include "ImNodeFlow/detail/fwd.h"
#include "ImNodeFlow/fwd.h"

#include "./imgui_bezier_items.h"
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

    ImVec2 start = ctx.grid2screen(sourceNode().getCenterGridPos());
    ImVec2 end = ctx.grid2screen(targetNode().getCenterGridPos());

    float thickness = 5.F;

    if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        link.selected = false;
    }

    auto mouse_pos = ImGui::GetMousePos();
    link.hovered = detail::smart_bezier_collider(
        mouse_pos,
        detail::smart_bezier_controls_from_two_points(start, end),
        2.5);

    if (link.hovered) {
        ImRect always_valid_bb {
            mouse_pos - ImVec2(1.F, 1.F),
            mouse_pos + ImVec2(1.F, 1.F),
        };

        bool pressed = false;
        if (ImGui::ItemAdd(always_valid_bb, ImGui::GetID(this))) {
            pressed = ImGui::ButtonBehavior(
                always_valid_bb,
                ImGui::GetID(this),
                &link.hovered,
                nullptr,
                ImGuiButtonFlags_PressedOnClick);
        }

        link.selected = link.selected || pressed;
    }

    if (link.hovered) {
        thickness = 10.F;
    }

    auto start_color = sourceNode().getStyle().header_bg;
    auto end_color = targetNode().getStyle().header_bg;

    if (link.selected) {
        double t = ImGui::GetTime();

        float v0 = std::sin(t * 2.F) * 0.1F + 0.85F;
        float v1 = std::sin(t * 2.F + 2.F) * 0.1F + 0.85F;

        ImVec4 start_rgb {
            v0,
            v0,
            v0,
            1.F,
        };
        ImVec4 end_rgb {
            v1,
            v1,
            v1,
            1.F,
        };

        start_color = ImGui::ColorConvertFloat4ToU32(start_rgb);
        end_color = ImGui::ColorConvertFloat4ToU32(end_rgb);
    }

    smart_bezier_gradient(
        ImGui::GetWindowDrawList(),
        detail::smart_bezier_controls_from_two_points(start, end),
        start_color,
        end_color,
        thickness,
        m_ctx.get().getStyle().link_gradient_blend);

    if (link.selected && ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
        this->destroy();
    }
}

} // namespace ImFlow::detail
