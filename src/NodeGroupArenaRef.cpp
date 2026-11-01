#include "ImNodeFlow/NodeGroupArenaRef.h"

#include "ImNodeFlow/Context.h"
#include "ImNodeFlow/NodeRef.h"
#include "ImNodeFlow/fwd.h"

#include "./node.h"
#include "./node_group.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace ImFlow {

template <bool TIsConst>
NodeGroupArenaRef<TIsConst>::NodeGroupArenaRef(context_type& ctx, group_arena_type& arena)
    : m_ctx(ctx)
    , m_arena(arena)
{
}

template <bool TIsConst>
void NodeGroupArenaRef<TIsConst>::destroy()
    requires(!TIsConst)
{
    detail::node_group_arena_t& arena = m_arena.get();
    std::vector<std::unique_ptr<detail::node_t>> live_nodes;
    live_nodes.reserve(arena.nodes.size());
    while (!arena.nodes.empty()) {
        auto n = std::move(arena.nodes.back());
        arena.nodes.pop_back();

        NodeRef node { m_ctx, *n };
        if (!node.isIndestructable()) {
            node.destroy();
        } else {
            live_nodes.emplace_back(std::move(n));
        }
    }
    arena.nodes = std::move(live_nodes);
}

template <bool TIsConst>
bool NodeGroupArenaRef<TIsConst>::mouse_over_any_selected_node() const
{
    detail::node_group_arena_t const& arena = m_arena.get();
    return std::any_of(
        arena.nodes.begin(),
        arena.nodes.end(),
        [this](const std::unique_ptr<detail::node_t>& n) -> bool {
            ConstNodeRef node { m_ctx, *n };
            bool node_is_selected = node.isSelected() && node.isHovered();
            bool child_node_is_selected = false;
            if (n->child_arena && NodeGroupArenaRef<true> { m_ctx, *n->child_arena }.mouse_over_any_selected_node()) {
                child_node_is_selected = true;
            }
            return node_is_selected || child_node_is_selected;
        });
}

template <bool TIsConst>
uint64_t NodeGroupArenaRef<TIsConst>::getNodesCount() const
{
    return m_arena.get().nodes.size();
}

template <bool TIsConst>
uint64_t NodeGroupArenaRef<TIsConst>::getSelectedNodesCount() const
{
    uint64_t n_selected = 0;
    auto& arena = this->m_arena.get();
    for (auto& node : arena.nodes) {
        if (ConstNodeRef { m_ctx, *node }.isSelected()) {
            ++n_selected;
        }
        if (node->child_arena != nullptr) {
            n_selected += NodeGroupArenaRef<TIsConst> { m_ctx, *node->child_arena }.getSelectedNodesCount();
        }
    }
    return n_selected;
}

template <bool TIsConst>
void NodeGroupArenaRef<TIsConst>::getNodes(
    std::vector<NodeRefImpl<TIsConst>>& out_vec) const
{
    auto& arena = this->m_arena.get();
    for (auto& node : arena.nodes) {
        out_vec.emplace_back(m_ctx, *node);
    }
}

template <bool TIsConst>
void NodeGroupArenaRef<TIsConst>::getSelectedNodes(
    std::vector<NodeRefImpl<TIsConst>>& out_vec) const
{
    auto& arena = this->m_arena.get();
    for (auto& n : arena.nodes) {
        NodeRefImpl<TIsConst> node { m_ctx, *n };
        if (node.isSelected()) {
            out_vec.emplace_back(std::move(node));
        }
        if (n->child_arena) {
            NodeGroupArenaRef<TIsConst> { m_ctx, *n->child_arena }.getSelectedNodes(out_vec);
        }
    }
}

template <bool TIsConst>
void NodeGroupArenaRef<TIsConst>::deselectAllNodes()
    requires(!TIsConst)
{
    auto& arena = m_arena.get();
    for (auto& node : arena.nodes) {
        NodeRef { m_ctx, *node }.setSelected(false);
        if (node->child_arena) {
            NodeGroupArenaRef<> { m_ctx, *node->child_arena }.deselectAllNodes();
        }
    }
}

template <bool TIsConst>
void NodeGroupArenaRef<TIsConst>::provisionallySelectArea(ImRect clip_area)
    requires(!TIsConst)
{
    auto& arena = m_arena.get();
    for (auto& n : arena.nodes) {
        NodeRef node { m_ctx, *n };

        ImRect node_clip {
            node.getTLPos(),
            node.getTLPos() + node.getFullSize(),
        };

        if (clip_area.Overlaps(node_clip)) {
            node.setProvisionallySelected(true);
        }

        if (n->child_arena) {
            NodeGroupArenaRef<TIsConst> { m_ctx, *n->child_arena }.provisionallySelectArea(clip_area);
        }
    }
}

template <bool TIsConst>
void NodeGroupArenaRef<TIsConst>::selectArea(ImRect clip_area)
    requires(!TIsConst)
{
    auto& arena = m_arena.get();
    for (auto& elem : arena.nodes) {
        NodeRef node { m_ctx, *elem };

        ImRect node_clip {
            node.getTLPos(),
            node.getTLPos() + node.getFullSize(),
        };

        if (clip_area.Overlaps(node_clip)) {
            node.setSelected(true);
        }

        if (elem->child_arena) {
            NodeGroupArenaRef<> { m_ctx, *elem->child_arena }.selectArea(clip_area);
        }
    }
}

template <bool TIsConst>
void NodeGroupArenaRef<TIsConst>::calculate_inputs()
    requires(!TIsConst)
{
    auto& arena = m_arena.get();
    for (auto& node : arena.nodes) {
        NodeRef { m_ctx, *node }.calculate_inputs();
    }
}

template <bool TIsConst>
void NodeGroupArenaRef<TIsConst>::calculate_outputs()
    requires(!TIsConst)
{
    auto& arena = m_arena.get();
    for (auto& node : arena.nodes) {
        NodeRef { m_ctx, *node }.calculate_outputs();
    }
}

template <bool TIsConst>
void NodeGroupArenaRef<TIsConst>::prime_inputs()
    requires(!TIsConst)
{
    auto& arena = m_arena.get();
    for (auto& node : arena.nodes) {
        NodeRef { m_ctx, *node }.prime_inputs();
    }
}

template <bool TIsConst>
void NodeGroupArenaRef<TIsConst>::prime_outputs()
    requires(!TIsConst)
{
    auto& arena = m_arena.get();
    for (auto& node : arena.nodes) {
        NodeRef { m_ctx, *node }.prime_outputs();
    }
}

template <bool TIsConst>
ImRect NodeGroupArenaRef<TIsConst>::get_bounding_box(ConstNodeRef parentNode) const
{
    ImRect arena_outline {
        parentNode.getTLPos(),
        parentNode.getTLPos() + parentNode.getFullSize(),
    };

    detail::node_group_arena_t const& arena = m_arena.get();

    for (auto const& n : arena.nodes) {
        ConstNodeRef node { m_ctx, *n };
        arena_outline.Add(node.getTLPos());
        arena_outline.Add(node.getTLPos() + node.getFullSize());

        auto child_arena = node.getChildArena();
        if (child_arena.has_value()) {
            auto child_bb = child_arena->get_bounding_box(node);
            arena_outline.Add(child_bb.Min);
            arena_outline.Add(child_bb.Max);
        }
    }

    ImVec2 padding { 10.F, 10.F };

    arena_outline.Add(arena_outline.Min - padding);
    arena_outline.Add(arena_outline.Max + padding);

    return arena_outline;
}

template <bool TIsConst>
void NodeGroupArenaRef<TIsConst>::draw_outline(ConstNodeRef parentNode) const
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    detail::node_group_arena_t const& arena = m_arena.get();
    auto const& ctx = m_ctx.get();

    ImRect arena_outline = this->get_bounding_box(parentNode);

    auto color = parentNode.getStyle().header_bg;
    auto radius = parentNode.getStyle().radius;

    auto color_rgb = ImGui::ColorConvertU32ToFloat4(color);
    auto bg_color = ImGui::ColorConvertFloat4ToU32(ImVec4 {
        color_rgb.x,
        color_rgb.y,
        color_rgb.z,
        color_rgb.w * 0.25F,
    });

    draw_list->AddRect(
        ctx.grid2screen(arena_outline.Min),
        ctx.grid2screen(arena_outline.Max),
        color,
        radius,
        1.5F);

    draw_list->AddRectFilled(
        ctx.grid2screen(arena_outline.Min),
        ctx.grid2screen(arena_outline.Max),
        bg_color,
        radius);

    ImVec2 parent_node_center = parentNode.getTLPos() + parentNode.getFullSize() / 2.F;

    draw_list->AddLine(
        ctx.grid2screen(ImVec2(parent_node_center.x, arena_outline.Min.y)),
        ctx.grid2screen(ImVec2(parent_node_center.x, arena_outline.Max.y)),
        color,
        1.F);

    draw_list->AddLine(
        ctx.grid2screen(ImVec2(arena_outline.Min.x, parent_node_center.y)),
        ctx.grid2screen(ImVec2(arena_outline.Max.x, parent_node_center.y)),
        color,
        1.F);
}

template <bool TIsConst>
void NodeGroupArenaRef<TIsConst>::update()
    requires(!TIsConst)
{
    auto& arena = m_arena.get();

    if (!arena.nodes.empty()) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        // We need this in order to render the nodes front to back
        ImDrawListSplitter splitter;
        splitter.Split(draw_list, arena.nodes.size());
        for (size_t i = 0; i < arena.nodes.size(); i++) {
            splitter.SetCurrentChannel(draw_list, arena.nodes.size() - i - 1);
            NodeRef { m_ctx, *arena.nodes[i] }.update();
        }
        splitter.Merge(draw_list);
    }
}

template <bool TIsConst>
void NodeGroupArenaRef<TIsConst>::prepareNextFrame()
    requires(!TIsConst)
{
    auto& arena = m_arena.get();
    std::vector<std::unique_ptr<detail::node_t>> next_iteration_nodes;

    size_t count_destroyed = 0;
    size_t count_activated = 0;
    size_t count_selected = 0;
    size_t count_idle = 0;

    for (auto& elem : arena.nodes) {
        NodeRef node { m_ctx, *elem };

        if (node.toDestroy()) {
            ++count_destroyed;
            continue;
        }

        node.prepareNextFrame();

        if (node.isActivated() || node.anyChildActivated()) {
            ++count_activated;
        } else if (node.isSelected()) {
            ++count_selected;
        } else {
            ++count_idle;
        }
    }

    next_iteration_nodes.resize(count_idle + count_selected + count_activated);

    size_t next_idx_activated = 0;
    size_t next_idx_selected = next_idx_activated + count_activated;
    size_t next_idx_idle = next_idx_selected + count_selected;

    for (auto& elem : arena.nodes) {
        NodeRef node { m_ctx, *elem };
        if (node.toDestroy()) {
            node.destroy();
            continue;
        }

        if (node.isActivated() || node.anyChildActivated()) {
            next_iteration_nodes[next_idx_activated] = std::move(elem);
            ++next_idx_activated;
        } else if (node.isSelected()) {
            next_iteration_nodes[next_idx_selected] = std::move(elem);
            ++next_idx_selected;
        } else {
            next_iteration_nodes[next_idx_idle] = std::move(elem);
            ++next_idx_idle;
        }
    }

    arena.nodes = std::move(next_iteration_nodes);
}

template class NodeGroupArenaRef<false>;
template class NodeGroupArenaRef<true>;

} // namespace ImFlow
