#pragma once

#include "ImNodeFlow/detail/fwd.h"
#include "ImNodeFlow/fwd.h"

#include "./imgui_bezier_items.h"

#include <functional>
#include <type_traits>

namespace ImFlow::detail {

template <bool TIsConst = false>
class NodeLinkRef {
public:
    using context_type = std::conditional_t<TIsConst, const ImNodeFlow, ImNodeFlow>;
    using link_type = std::conditional_t<TIsConst, const node_link_t, node_link_t>;

    NodeLinkRef(context_type& ctx, link_type& link);

    static cubic_bezier_controls_t
    getCubicLineBetween(
        ImNodeFlow& ctx,
        ConstNodeRef sourceNode,
        ConstNodeRef targetNode);

    static cubic_bezier_controls_t
    getCubicLineToMouse(
        ImNodeFlow& ctx,
        ConstNodeRef sourceNode);

    [[nodiscard]] bool isAlive() const;

    [[nodiscard]] NodeRefImpl<TIsConst> sourceNode() const;
    [[nodiscard]] NodeRefImpl<TIsConst> targetNode() const;

    void connect(NodeRef source_node, NodeRef target_node)
        requires(!TIsConst);

    void destroy()
        requires(!TIsConst);

    void update()
        requires(!TIsConst);

private:
    std::reference_wrapper<context_type> m_ctx;
    std::reference_wrapper<link_type> m_link;
};

} // namespace ImFlow::detail
