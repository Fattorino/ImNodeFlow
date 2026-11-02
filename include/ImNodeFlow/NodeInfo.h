#ifndef IMNODEFLOW_NODEINFO_H
#define IMNODEFLOW_NODEINFO_H
#pragma once

#include "ImNodeFlow/INodeBehaviour.h"
#include "ImNodeFlow/NodeStyle.h"

#include <memory>

namespace ImFlow {

/**
 * @brief User customizable options when creating a new Node
 */
struct NodeInfo {
    /// Node strategy for initialization and rendering
    std::unique_ptr<INodeBehaviour> behaviour;
    /// Style to apply to the node
    NodeStyle style = NodeStyles::default_style;
    /// Whether this node should allow inbound links to other nodes
    bool allow_inbound_links_to_other_nodes = false;
    /// Whether this node should allow outbound links to other nodes
    bool allow_outbound_links_to_other_nodes = false;
};

} // namespace ImFlow

#endif // IMNODEFLOW_NODEINFO_H
