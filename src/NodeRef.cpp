#include "ImNodeFlow/NodeRef.h"

#include "ImNodeFlow/Context.h"
#include "ImNodeFlow/NodeGroupArenaRef.h"
#include "ImNodeFlow/NodeStyle.h"
#include "ImNodeFlow/NodeUID.h"
#include "ImNodeFlow/PinInfo.h"
#include "ImNodeFlow/PinRef.h"
#include "ImNodeFlow/PinType.h"
#include "ImNodeFlow/PinUID.h"
#include "ImNodeFlow/detail/IPin.h"
#include "ImNodeFlow/detail/fwd.h"
#include "ImNodeFlow/fwd.h"

#include "./DynamicPinRef.h"
#include "./NodeLinkRef.h"
#include "./dynamic_pin.h"
#include "./editor_context.h"
#include "./imgui_bezier_items.h"
#include "./imgui_button_flags_helper.h"
#include "./imgui_compat.h"
#include "./node.h"
#include "./node_link.h"
#include "./pin_base.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

namespace ImFlow {

template <bool TIsConst>
NodeRefImpl<TIsConst>::NodeRefImpl(context_type& ctx, node_type& internal_node)
    : m_ctx(ctx)
    , m_node(internal_node)
{
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::destroy()
    requires(!TIsConst)
{
    detail::node_t& node = m_node.get();

    if (node.indestructable) {
        return;
    }

    if (node.behaviour) {
        node.behaviour.reset();
    }
    this->deleteAllLinks();
    for (auto& elem : node.ins) {
        PinRef { m_ctx, *elem }.destroy();
    }
    for (auto& elem : node.dynamicIns) {
        detail::DynamicPinRef<> { m_ctx, *elem }.destroy();
    }
    for (auto& elem : node.outs) {
        PinRef { m_ctx, *elem }.destroy();
    }
    for (auto& elem : node.dynamicOuts) {
        detail::DynamicPinRef<> { m_ctx, *elem }.destroy();
    }
    auto child_arena = this->getChildArena();
    if (child_arena.has_value()) {
        child_arena->destroy();
        while (!node.child_arena->nodes.empty()) {
            NodeRef { m_ctx, *node.child_arena->nodes.back() }
                .setParentNode(this->getParentNode());
        }
        node.child_arena.reset();
    }
    node.destroyed = true;
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::deleteAllLinks()
    requires(!TIsConst)
{
    auto& node = m_node.get();
    auto links = std::move(node.links);
    for (auto& l : links) {
        detail::NodeLinkRef { m_ctx, *l }.destroy();
    }
}

template <bool TIsConst>
bool NodeRefImpl<TIsConst>::toDestroy() const
{
    return m_node.get().destroyed;
}

template <bool TIsConst>
bool NodeRefImpl<TIsConst>::isHovered() const
{
    auto& node = m_node.get();
    ImVec2 paddingTL = { node.style.padding.x, node.style.padding.y };
    ImVec2 paddingBR = { node.style.padding.z, node.style.padding.w };
    return ImGui::IsMouseHoveringRect(
        m_ctx.get().grid2screen(node.pos - paddingTL),
        m_ctx.get().grid2screen(node.pos + node.size + paddingBR));
}

template <bool TIsConst>
NodeUID NodeRefImpl<TIsConst>::getUID() const
{
    return m_node.get().uid;
}

template <bool TIsConst>
const std::string& NodeRefImpl<TIsConst>::getName() const
{
    return m_node.get().title;
}

template <bool TIsConst>
const ImVec2& NodeRefImpl<TIsConst>::getSize() const
{
    return m_node.get().size;
}

template <bool TIsConst>
const ImVec2& NodeRefImpl<TIsConst>::getFullSize() const
{
    return m_node.get().fullSize;
}

template <bool TIsConst>
const ImVec2& NodeRefImpl<TIsConst>::getPos() const
{
    return m_node.get().pos;
}

template <bool TIsConst>
ImVec2 NodeRefImpl<TIsConst>::getTLPos() const
{
    auto& node = m_node.get();
    return node.pos - ImVec2 { node.style.padding.x, node.style.padding.y };
}

template <bool TIsConst>
ImVec2 NodeRefImpl<TIsConst>::getCenterGridPos() const
{
    auto tl_pos = this->getTLPos();
    auto size = this->getFullSize();
    return tl_pos + size * 0.5;
}

template <bool TIsConst>
NodeStyle& NodeRefImpl<TIsConst>::getStyle() const
    requires(!TIsConst)
{
    return m_node.get().style;
}

template <bool TIsConst>
const NodeStyle& NodeRefImpl<TIsConst>::getStyle() const
{
    return m_node.get().style;
}

template <bool TIsConst>
bool NodeRefImpl<TIsConst>::isActivated() const
{
    auto& node = m_node.get();
    return !node.wasSelected && node.selected;
}

template <bool TIsConst>
std::vector<NodeRefImpl<TIsConst>>
NodeRefImpl<TIsConst>::getChildNodes() const
{
    std::vector<NodeRefImpl<TIsConst>> result;
    auto child_arena = this->getChildArena();
    if (child_arena.has_value()) {
        result.reserve(child_arena->getNodesCount());
        child_arena->getNodes(result);
    }
    return result;
}

template <bool TIsConst>
std::optional<NodeGroupArenaRef<TIsConst>>
NodeRefImpl<TIsConst>::getChildArena() const
{
    std::optional<NodeGroupArenaRef<TIsConst>> result;
    auto& node = m_node.get();
    if (node.child_arena) {
        result.emplace(m_ctx, *node.child_arena);
    }
    return result;
}

template <bool TIsConst>
bool NodeRefImpl<TIsConst>::anyChildActivated() const
{
    detail::node_t const& node = m_node.get();
    if (!node.child_arena) {
        return false;
    }
    return std::any_of(
        std::begin(node.child_arena->nodes),
        std::end(node.child_arena->nodes),
        [this](const auto& elem) -> bool {
            ConstNodeRef child_node { m_ctx, *elem };
            return child_node.isActivated() || child_node.anyChildActivated();
        });
}

template <bool TIsConst>
bool NodeRefImpl<TIsConst>::hasChildNodes() const
{
    detail::node_t const& node = m_node.get();
    return node.child_arena && !node.child_arena->nodes.empty();
}

template <bool TIsConst>
std::vector<NodeRefImpl<TIsConst>>
NodeRefImpl<TIsConst>::getInboundConnections() const
{
    detail::node_t const& node = m_node.get();
    std::vector<NodeRefImpl<TIsConst>> result;
    result.reserve(node.links.size());
    for (auto& l : node.links) {
        if (l->target_node == &node) {
            result.emplace_back(m_ctx, *l->source_node);
        }
    }
    return result;
}

template <bool TIsConst>
std::vector<NodeRefImpl<TIsConst>>
NodeRefImpl<TIsConst>::getOutboundConnections() const
{
    detail::node_t const& node = m_node.get();
    std::vector<NodeRefImpl<TIsConst>> result;
    result.reserve(node.links.size());
    for (auto& l : node.links) {
        if (l->source_node == &node) {
            result.emplace_back(m_ctx, *l->target_node);
        }
    }
    return result;
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::linkTo(NodeRef other)
    requires(!TIsConst)
{
    auto& ctx = m_ctx.get();
    ctx.createLink(*this, other);
}

template <bool TIsConst>
bool NodeRefImpl<TIsConst>::isSelected() const
{
    auto& node = m_node.get();
    return node.selected || node.provisionallySelected;
}

template <bool TIsConst>
bool NodeRefImpl<TIsConst>::isIndestructable() const
{
    return m_node.get().indestructable;
}

template <bool TIsConst>
NodeRefImpl<TIsConst>& NodeRefImpl<TIsConst>::setUID(NodeUID uid)
    requires(!TIsConst)
{
    m_node.get().uid = uid;
    return *this;
}

template <bool TIsConst>
NodeRefImpl<TIsConst>& NodeRefImpl<TIsConst>::setTitle(const std::string& title)
    requires(!TIsConst)
{
    m_node.get().title = title;
    return *this;
}

template <bool TIsConst>
NodeRefImpl<TIsConst>& NodeRefImpl<TIsConst>::setPos(const ImVec2& pos)
    requires(!TIsConst)
{
    auto& node = m_node.get();
    node.pos = pos;
    node.posTarget = pos;
    return *this;
}

template <bool TIsConst>
NodeRefImpl<TIsConst>& NodeRefImpl<TIsConst>::setStyle(NodeStyle style)
    requires(!TIsConst)
{
    m_node.get().style = std::move(style);
    return *this;
}

template <bool TIsConst>
NodeRefImpl<TIsConst>& NodeRefImpl<TIsConst>::setAllowInboundLinks(bool value)
    requires(!TIsConst)
{
    detail::node_t& node = m_node.get();
    node.allowInboundLinksToOtherNodes = value;
    return *this;
}

template <bool TIsConst>
NodeRefImpl<TIsConst>& NodeRefImpl<TIsConst>::setAllowOutboundLinks(bool value)
    requires(!TIsConst)
{
    detail::node_t& node = m_node.get();
    node.allowOutboundLinksToOtherNodes = value;
    return *this;
}

template <bool TIsConst>
NodeRefImpl<TIsConst>& NodeRefImpl<TIsConst>::setIndestructable(bool indestruct)
    requires(!TIsConst)
{
    m_node.get().indestructable = indestruct;
    return *this;
}

template <bool TIsConst>
NodeRefImpl<TIsConst>& NodeRefImpl<TIsConst>::setSelected(bool state)
    requires(!TIsConst)
{
    auto& node = m_node.get();
    node.selectedNext = state;
    return *this;
}

template <bool TIsConst>
NodeRefImpl<TIsConst>& NodeRefImpl<TIsConst>::setProvisionallySelected(bool state)
    requires(!TIsConst)
{
    m_node.get().provisionallySelectedNext = state;
    return *this;
}

template <bool TIsConst>
ImNodeFlow& NodeRefImpl<TIsConst>::getContext() const
    requires(!TIsConst)
{
    return m_ctx.get();
}

template <bool TIsConst>
ImNodeFlow const& NodeRefImpl<TIsConst>::getContext() const
{
    return m_ctx.get();
}

template <bool TIsConst>
INodeBehaviour* NodeRefImpl<TIsConst>::getBehavior() const
    requires(!TIsConst)
{
    detail::node_t& node = m_node.get();
    if (node.behaviour) {
        return &*node.behaviour;
    }
    return nullptr;
}

template <bool TIsConst>
INodeBehaviour const* NodeRefImpl<TIsConst>::getBehavior() const
{
    detail::node_t const& node = m_node.get();
    if (node.behaviour) {
        return &*node.behaviour;
    }
    return nullptr;
}

template <bool TIsConst>
std::optional<NodeRefImpl<TIsConst>>
NodeRefImpl<TIsConst>::getParentNode() const
{
    auto& node = m_node.get();

    std::optional<NodeRefImpl<TIsConst>> result;
    if (node.parent_node) {
        result.emplace(m_ctx, *node.parent_node);
    }
    return result;
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::detachFromParent()
    requires(!TIsConst)
{
    auto grandparent_node = this->getParentNode();
    if (grandparent_node.has_value()) {
        grandparent_node = grandparent_node->getParentNode();
    }
    return this->setParentNode(grandparent_node);
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::setParentNode(std::optional<NodeRef> newParentNode)
    requires(!TIsConst)
{
    ImNodeFlow& ctx = m_ctx.get();
    detail::node_t& node = m_node.get();
    detail::node_t* parent_node = newParentNode.has_value() ? &newParentNode->m_node.get() : nullptr;

    // A node cannot be it's own parent
    if (&node == parent_node) {
        return;
    }

    for (detail::node_t* curr = parent_node;
        curr != nullptr;
        curr = curr->parent_node) {
        if (curr == &node) {
            // new parent node is a child node, so we must remove it from
            // the the child list in order to avoid catastrophic results
            newParentNode->setParentNode(getParentNode());
            break;
        }
    }

    detail::node_group_arena_t* own_arena = &ctx.getDetail().root_arena;
    if (node.parent_node != nullptr) {
        own_arena = node.parent_node->child_arena.get();
    }

    detail::node_group_arena_t* target_arena = &ctx.getDetail().root_arena;
    if (parent_node != nullptr) {
        if (!parent_node->child_arena) {
            parent_node->child_arena = std::make_unique<detail::node_group_arena_t>();
        }
        target_arena = &*parent_node->child_arena;
    }

    assert(own_arena != nullptr && "Could not find node's own arena");
    assert(target_arena != nullptr && "Could not find target arena");

    if (own_arena == target_arena) {
        return;
    }

    auto iter = std::find_if(
        std::begin(own_arena->nodes),
        std::end(own_arena->nodes),
        [&](std::unique_ptr<detail::node_t>& elem) -> bool {
            return elem.get() == &node;
        });

    assert(iter != std::end(own_arena->nodes)
        && "Sanity check failed. Node cannot be found in it's own arena.");

    // move node ownership to the new arena
    target_arena->nodes.emplace_back(std::move(*iter));
    // Remove the empty unique_ptr from the old arena
    own_arena->nodes.erase(iter);
    node.parent_node = parent_node;
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::disbandGroup()
    requires(!TIsConst)
{
    detail::node_t& node = m_node.get();
    if (!node.child_arena) {
        return;
    }

    while (!node.child_arena->nodes.empty()) {
        NodeRef child { m_ctx, **std::rbegin(node.child_arena->nodes) };
        child.setParentNode(getParentNode());
    }

    node.child_arena.reset();
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::prepareNextFrame()
    requires(!TIsConst)
{
    detail::node_t& node = m_node.get();
    node.wasSelected = node.selected;
    node.selected = node.selectedNext;
    node.provisionallySelected = node.provisionallySelectedNext;
    node.provisionallySelectedNext = false;

    if (node.behaviour) {
        node.behaviour->prepareNextFrame(*this);
    }

    auto child_arena = this->getChildArena();
    if (child_arena.has_value()) {
        child_arena->prepareNextFrame();
        if (child_arena->getNodesCount() == 0) {
            node.child_arena.reset();
        }
    }
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::dropIN(const PinUID& uid)
    requires(!TIsConst)
{
    auto& node = m_node.get();
    auto iter = std::find_if(
        node.ins.begin(),
        node.ins.end(),
        [&](auto const& pin) -> bool {
            return pin->uid == uid;
        });

    if (iter != node.ins.end()) {
        node.ins.erase(iter);
    }
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::dropIN(std::string_view name)
    requires(!TIsConst)
{
    auto& node = m_node.get();
    node.ins.erase(
        std::remove_if(
            node.ins.begin(),
            node.ins.end(),
            [&](auto const& pin) -> bool {
                return pin->name == name;
            }),
        node.ins.end());
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::dropOUT(const PinUID& uid)
    requires(!TIsConst)
{
    auto& node = m_node.get();
    auto iter = std::find_if(
        node.outs.begin(),
        node.outs.end(),
        [&uid](auto const& pin) -> bool {
            return pin->uid == uid;
        });

    if (iter != node.outs.end()) {
        node.outs.erase(iter);
    }
}

template <bool TIsConst>
inline void
NodeRefImpl<TIsConst>::dropOUT(std::string_view name)
    requires(!TIsConst)
{
    auto& node = m_node.get();
    node.outs.erase(
        std::remove_if(
            node.outs.begin(),
            node.outs.end(),
            [&name](auto const& pin) -> bool {
                return pin->name == name;
            }),
        node.outs.end());
}

template <bool TIsConst>
std::optional<PinRefImpl<TIsConst>>
NodeRefImpl<TIsConst>::inPin(const PinUID& uid)
{
    auto& node = m_node.get();
    auto it = std::find_if(
        node.ins.begin(),
        node.ins.end(),
        [&uid](const auto& p) {
            return p->uid == uid;
        });
    std::optional<PinRefImpl<TIsConst>> result;
    if (it != node.ins.end()) {
        result.emplace(m_ctx, **it);
    }
    return result;
}

template <bool TIsConst>
std::optional<PinRefImpl<TIsConst>>
NodeRefImpl<TIsConst>::inPin(std::string_view name)
{
    auto& node = m_node.get();
    auto it = std::find_if(
        node.ins.begin(),
        node.ins.end(),
        [&name](const auto& p) {
            return p->name == name;
        });
    std::optional<PinRefImpl<TIsConst>> result;
    if (it != node.ins.end()) {
        result.emplace(m_ctx, **it);
    }
    return result;
}

template <bool TIsConst>
std::optional<PinRefImpl<TIsConst>>
NodeRefImpl<TIsConst>::tempInPin(const PinUID& uid)
{
    auto& node = m_node.get();
    auto it = std::find_if(
        node.dynamicIns.begin(),
        node.dynamicIns.end(),
        [&uid](const auto& p) {
            return p->pin.uid == uid;
        });
    std::optional<PinRefImpl<TIsConst>> result;
    if (it != node.dynamicIns.end()) {
        detail::DynamicPinRef<TIsConst> dyn_pin { m_ctx, **it };
        if constexpr (!TIsConst) {
            dyn_pin.refresh();
        }
        result.emplace(dyn_pin.pin());
    }
    return result;
}

template <bool TIsConst>
std::optional<PinRefImpl<TIsConst>>
NodeRefImpl<TIsConst>::tempInPin(std::string_view name)
{
    auto& node = m_node.get();
    auto it = std::find_if(
        node.dynamicIns.begin(),
        node.dynamicIns.end(),
        [&name](const auto& p) {
            return p->pin.name == name;
        });
    std::optional<PinRefImpl<TIsConst>> result;
    if (it != node.dynamicIns.end()) {
        detail::DynamicPinRef<TIsConst> dyn_pin { m_ctx, **it };
        if constexpr (!TIsConst) {
            dyn_pin.refresh();
        }
        result.emplace(dyn_pin.pin());
    }
    return result;
}

template <bool TIsConst>
std::optional<PinRefImpl<TIsConst>>
NodeRefImpl<TIsConst>::outPin(const PinUID& uid)
{
    auto& node = m_node.get();
    auto it = std::find_if(
        node.outs.begin(),
        node.outs.end(),
        [&uid](const auto& p) {
            return p->uid == uid;
        });
    std::optional<PinRefImpl<TIsConst>> result;
    if (it != node.outs.end()) {
        result.emplace(m_ctx, **it);
    }
    return result;
}

template <bool TIsConst>
std::optional<PinRefImpl<TIsConst>>
NodeRefImpl<TIsConst>::outPin(std::string_view name)
{
    auto& node = m_node.get();
    auto it = std::find_if(
        node.outs.begin(),
        node.outs.end(),
        [&name](const auto& p) {
            return p->name == name;
        });
    std::optional<PinRefImpl<TIsConst>> result;
    if (it != node.outs.end()) {
        result.emplace(m_ctx, **it);
    }
    return result;
}

template <bool TIsConst>
std::optional<PinRefImpl<TIsConst>>
NodeRefImpl<TIsConst>::tempOutPin(const PinUID& uid)
{
    auto& node = m_node.get();
    auto it = std::find_if(
        node.dynamicOuts.begin(),
        node.dynamicOuts.end(),
        [&uid](const auto& p) {
            return p->pin.uid == uid;
        });
    std::optional<PinRefImpl<TIsConst>> result;
    if (it != node.dynamicOuts.end()) {
        detail::DynamicPinRef<TIsConst> dyn_pin { m_ctx, **it };
        if constexpr (!TIsConst) {
            dyn_pin.refresh();
        }
        result.emplace(dyn_pin.pin());
    }
    return result;
}

template <bool TIsConst>
std::optional<PinRefImpl<TIsConst>>
NodeRefImpl<TIsConst>::tempOutPin(std::string_view name)
{
    auto& node = m_node.get();
    auto it = std::find_if(
        node.dynamicOuts.begin(),
        node.dynamicOuts.end(),
        [&name](const auto& p) {
            return p->pin.name == name;
        });
    std::optional<PinRefImpl<TIsConst>> result;
    if (it != node.dynamicOuts.end()) {
        detail::DynamicPinRef<TIsConst> dyn_pin { m_ctx, **it };
        if constexpr (!TIsConst) {
            dyn_pin.refresh();
        }
        result.emplace(dyn_pin.pin());
    }
    return result;
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::calculate_inputs()
    requires(!TIsConst)
{
    detail::node_t& node = m_node.get();
    for (auto& pin : node.ins) {
        PinRef { m_ctx, *pin }.calculate();
    }
    for (auto& pin : node.dynamicIns) {
        detail::DynamicPinRef { m_ctx, *pin }.pin().calculate();
    }
    auto child_arena = this->getChildArena();
    if (child_arena.has_value()) {
        child_arena->calculate_inputs();
    }
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::calculate_outputs()
    requires(!TIsConst)
{
    auto& node = m_node.get();
    for (auto& pin : node.outs) {
        PinRef { m_ctx, *pin }.calculate();
    }
    for (auto& pin : node.dynamicOuts) {
        detail::DynamicPinRef { m_ctx, *pin }.pin().calculate();
    }
    auto child_arena = this->getChildArena();
    if (child_arena.has_value()) {
        child_arena->calculate_outputs();
    }
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::prime_inputs()
    requires(!TIsConst)
{
    auto& node = m_node.get();
    for (auto& pin : node.ins) {
        PinRef { m_ctx, *pin }.prime();
    }
    for (auto& pin : node.dynamicIns) {
        detail::DynamicPinRef { m_ctx, *pin }.pin().prime();
    }
    auto child_arena = this->getChildArena();
    if (child_arena.has_value()) {
        child_arena->prime_inputs();
    }
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::prime_outputs()
    requires(!TIsConst)
{
    auto& node = m_node.get();
    for (auto& pin : node.outs) {
        PinRef { m_ctx, *pin }.prime();
    }
    for (auto& pin : node.dynamicOuts) {
        detail::DynamicPinRef { m_ctx, *pin }.pin().prime();
    }
    auto child_arena = this->getChildArena();
    if (child_arena.has_value()) {
        child_arena->prime_outputs();
    }
}

template <bool TIsConst>
void NodeRefImpl<TIsConst>::update()
    requires(!TIsConst)
{
    detail::node_t& node = m_node.get();
    auto& ctx = m_ctx.get();

    if (node.child_arena && node.child_arena->nodes.empty()) {
        node.child_arena.reset();
    }

    {
        auto child_arena = this->getChildArena();
        if (child_arena.has_value()) {
            child_arena->draw_outline(*this);
        }
    }

    if (ImGui::IsWindowFocused() && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ctx.mouse_on_selected_node()) {
        setSelected(false);
    }

    if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete) && !ImGui::IsAnyItemActive() && isSelected() && !isIndestructable()) {
        destroy();
        return;
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImGui::PushID(&node);

    ImVec2 offset = ctx.grid2screen({ 0.f, 0.f });
    ImVec2 paddingTL = { node.style.padding.x, node.style.padding.y };
    ImVec2 paddingBR = { node.style.padding.z, node.style.padding.w };

    // Used to render each node's foreground and background out of order
    ImDrawListSplitter drawListSplitter;
    drawListSplitter.Split(draw_list, 3);
    drawListSplitter.SetCurrentChannel(draw_list, 2); // Foreground
    ImGui::SetCursorScreenPos(offset + node.pos);

    ImGui::BeginGroup();

    // Header
    ImGui::BeginGroup();
    ImGui::TextColored(node.style.header_title_color, "%s", node.title.c_str());
    ImGui::Spacing();
    ImGui::EndGroup();
    float headerH = ImGui::GetItemRectSize().y;
    float titleW = ImGui::GetItemRectSize().x;

    // Inputs
    if (!node.ins.empty() || !node.dynamicIns.empty()) {
        ImGui::BeginGroup();
        for (auto& p : node.ins) {
            PinRef { m_ctx, *p }.update();
        }
        for (auto& dyn_pin : node.dynamicIns) {
            detail::DynamicPinRef<> { m_ctx, *dyn_pin }.update();
        }
        ImGui::EndGroup();
        ImGui::SameLine();
    }

    // Content
    ImGui::BeginGroup();

    if (node.behaviour) {
        node.behaviour->draw(*this);
    } else {
        ImGui::TextUnformatted("<no renderer provided>");
    }

    // Dynamic inputs stale check can only be done after the call to
    // behaviour->draw() and before any call to DynamicPin::update()
    node.dynamicIns.erase(
        std::remove_if(
            node.dynamicIns.begin(),
            node.dynamicIns.end(),
            [this](const auto& elem) -> bool {
                return detail::DynamicPinRef { m_ctx, *elem }.isStale();
            }),
        node.dynamicIns.end());
    node.dynamicOuts.erase(
        std::remove_if(
            node.dynamicOuts.begin(),
            node.dynamicOuts.end(),
            [this](const auto& elem) -> bool {
                return detail::DynamicPinRef { m_ctx, *elem }.isStale();
            }),
        node.dynamicOuts.end());

    ImGui::Dummy(ImVec2(0.f, 0.f));
    ImGui::EndGroup();
    ImGui::SameLine();

    // Outputs
    float maxW = 0.0f;
    for (auto& p : node.outs) {
        PinRef pin { m_ctx, *p };
        float w = pin.pinPoint().x - pin.getPos().x;
        maxW = std::max(maxW, w);
    }
    for (auto& dyn_pin : node.dynamicOuts) {
        auto pin = detail::DynamicPinRef<> { m_ctx, *dyn_pin }.pin();
        float w = pin.pinPoint().x - pin.getPos().x;
        maxW = std::max(maxW, w);
    }
    ImGui::BeginGroup();
    for (auto& p : node.outs) {
        PinRef pin { m_ctx, *p };
        float w = pin.pinPoint().x - pin.getPos().x;
        if (w > 0.F) {
            float align_offset = maxW - w;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + align_offset);
        }
        pin.update();
    }
    for (auto& elem : node.dynamicOuts) {
        detail::DynamicPinRef<> dyn_pin { m_ctx, *elem };
        PinRef pin = dyn_pin.pin();
        float w = pin.pinPoint().x - pin.getPos().x;
        if (w > 0.F) {
            float align_offset = maxW - w;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + align_offset);
        }
        dyn_pin.update();
    }

    ImGui::EndGroup();

    ImGui::EndGroup();
    node.size = ImGui::GetItemRectSize();
    ImVec2 headerSize = ImVec2(node.size.x + paddingBR.x, headerH);

    ImRect header_bb {
        ctx.grid2screen(node.pos - paddingTL),
        ctx.grid2screen(node.pos + headerSize),
    };

    ImRect node_bb {
        ctx.grid2screen(node.pos - paddingTL),
        ctx.grid2screen(node.pos + node.size + paddingBR),
    };

    bool header_hovered = false;
    bool header_held = false;
    bool header_pressed = false;
    bool header_released = false;
    bool node_hovered = false;
    bool node_held = false;
    bool node_pressed = false;
    bool node_released = false;

    ImGuiID header_id = ImGui::GetID("##node_header");
    ImGuiID node_id = ImGui::GetID("##node_body");

    if (ImGui::ItemAdd(header_bb, header_id)) {
        header_pressed = ImGui::ButtonBehavior(
            header_bb,
            header_id,
            &header_hovered,
            &header_held,
            ImGuiButtonFlags_PressedOnClick
                | detail::getButtonFlagsForMouseButton(ImGuiMouseButton_Left)
                | detail::getButtonFlagsForMouseButton(ImGuiMouseButton_Right));

        header_released = ImGui::IsItemDeactivated();
    }

    if (ImGui::ItemAdd(node_bb, node_id)) {
        node_pressed = ImGui::ButtonBehavior(
            node_bb,
            node_id,
            &node_hovered,
            &node_held,
            ImGuiButtonFlags_PressedOnClick);

        node_released = ImGui::IsItemDeactivated();
    }

    if (node.allowOutboundLinksToOtherNodes
        && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
        auto dto = this->serialize();
        ImGui::SetDragDropPayload("NODE_REF_LOCAL_DTO", &dto, sizeof(dto));
        ImGui::EndDragDropSource();

        auto line = detail::NodeLinkRef<true>::getCubicLineToMouse(m_ctx, *this);

        detail::smart_bezier(
            ImGui::GetForegroundDrawList(),
            line,
            node.style.header_bg,
            node.style.link_thickness);
    }

    if (node.allowInboundLinksToOtherNodes
        && ImGui::BeginDragDropTarget()) {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NODE_REF_LOCAL_DTO");
        std::optional<NodeRef> inbound;

        if (payload != nullptr) {
            NodeRefLocalDTO<> dto { };
            IM_ASSERT(payload->DataSize == sizeof(dto));
            std::memcpy(&dto, payload->Data, sizeof(dto));
            inbound.emplace(dto);
        }
        ImGui::EndDragDropTarget();

        if (inbound.has_value()) {
            if (node.behaviour && node.behaviour->acceptInboundNodeConnection(*this, *inbound)) {
                inbound->linkTo(*this);
            }
        }
    }

    // Background
    {
        auto bg_col = node_hovered ? node.style.bg_hovered : node.style.bg;

        drawListSplitter.SetCurrentChannel(draw_list, 1);
        draw_list->AddRectFilled(
            offset + node.pos - paddingTL,
            offset + node.pos + node.size + paddingBR,
            bg_col,
            node.style.radius);
        draw_list->AddRectFilled(
            offset + node.pos - paddingTL,
            offset + node.pos + headerSize,
            node.style.header_bg,
            node.style.radius,
#if IMGUI_VERSION_NUM >= 19300
            ImDrawFlags_RoundTop);
#else
            ImDrawFlags_RoundCornersTop);
#endif
        node.fullSize = node.size + paddingTL + paddingBR;
        ImU32 col = node.style.border_color;
        float thickness = node.style.border_thickness;
        ImVec2 ptl = paddingTL;
        ImVec2 pbr = paddingBR;
        if (this->isSelected()) {
            col = node.style.border_selected_color;
            thickness = node.style.border_selected_thickness;
        }
        if (thickness < 0.f) {
            ptl.x -= thickness / 2;
            ptl.y -= thickness / 2;
            pbr.x -= thickness / 2;
            pbr.y -= thickness / 2;
            thickness *= -1.f;
        }
        compat::AddRect(
            draw_list,
            offset + node.pos - ptl,
            offset + node.pos + node.size + pbr,
            col,
            node.style.radius,
            thickness,
            ImDrawFlags_None);
    }

    if (node_pressed || header_pressed) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            node.wantsDeselect = node.selected;
            setSelected(true);
        } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("RightClickPopUp");
        }
    }
    ctx.drawRightClickPopup(&node);
    if ((node_released || header_released)
        && ImGui::IsMouseReleased(ImGuiMouseButton_Left)
        && node.wantsDeselect) {
        setSelected(false);
    }
    if (header_pressed
        && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ctx.startNodeDrag();
    }
    if (header_released
        && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        ctx.stopNodeDrag();
    }

    bool wants_drag = false;
    detail::node_t* curr = &node;
    while (!wants_drag && curr != nullptr) {
        wants_drag = curr->selected;
        curr = curr->parent_node;
    }

    if (wants_drag && ctx.wantsNodeDrag()) {
        ImVec2 drag_vec;
        bool should_apply_drag = ctx.getNodeDragAmmount(&drag_vec);

        if (!node.dragging) {
            node.posBackup = node.posTarget;
            node.dragging = true;
        }

        node.posTarget = node.posBackup + drag_vec;
        if (drag_vec != ImVec2(0.F, 0.F)) {
            node.wantsDeselect = false;
        }

        node.pos = ctx.slamPosition(node.posTarget);

        if (should_apply_drag) {
            node.dragging = false;
            node.posTarget = node.pos;
        }
    }

    drawListSplitter.SetCurrentChannel(draw_list, 0);
    {
        auto child_arena = this->getChildArena();
        if (child_arena.has_value()) {
            child_arena->update();
        }
    }

    drawListSplitter.Merge(draw_list);
    ImGui::PopID();
}

template <bool TIsConst>
PinRefImpl<TIsConst>
NodeRefImpl<TIsConst>::addIN_impl(
    BasePinInfo info,
    std::unique_ptr<detail::IPin> pin_impl)
    requires(!TIsConst)
{
    auto& node = m_node.get();
    node.ins.emplace_back(std::make_unique<detail::pin_base_t>(
        *this,
        PinType::Input,
        std::move(info),
        std::move(pin_impl)));

    return PinRefImpl<TIsConst> { m_ctx, *node.ins.back() };
}

template <bool TIsConst>
PinRefImpl<TIsConst>
NodeRefImpl<TIsConst>::showIN_impl(
    BasePinInfo info,
    std::unique_ptr<detail::IPin> pin_impl)
    requires(!TIsConst)
{
    auto& node = m_node.get();
    node.dynamicIns.emplace_back(std::make_unique<detail::dynamic_pin_t>(
        detail::pin_base_t {
            *this,
            PinType::Input,
            std::move(info),
            std::move(pin_impl),
        }));

    return detail::DynamicPinRef<TIsConst> { m_ctx, *node.dynamicIns.back() }.pin();
}

template <bool TIsConst>
PinRefImpl<TIsConst>
NodeRefImpl<TIsConst>::addOUT_impl(
    BasePinInfo info,
    std::unique_ptr<detail::IPin> pin_impl)
    requires(!TIsConst)
{
    auto& node = m_node.get();
    node.outs.emplace_back(std::make_unique<detail::pin_base_t>(
        *this,
        PinType::Output,
        std::move(info),
        std::move(pin_impl)));

    return PinRefImpl<TIsConst> { m_ctx, *node.outs.back() };
}

template <bool TIsConst>
PinRefImpl<TIsConst>
NodeRefImpl<TIsConst>::showOUT_impl(
    BasePinInfo info,
    std::unique_ptr<detail::IPin> pin_impl)
    requires(!TIsConst)
{
    auto& node = m_node.get();
    node.dynamicOuts.emplace_back(std::make_unique<detail::dynamic_pin_t>(
        detail::pin_base_t {
            *this,
            PinType::Output,
            std::move(info),
            std::move(pin_impl),
        }));

    return detail::DynamicPinRef<TIsConst> { m_ctx, *node.dynamicOuts.back() }.pin();
}

template class NodeRefImpl<false>;
template class NodeRefImpl<true>;

} // namespace ImFlow
