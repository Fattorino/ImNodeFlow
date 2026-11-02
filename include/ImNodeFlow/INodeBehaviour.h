#ifndef IMNODEFLOW_INODEBEHAVIOUR_H
#define IMNODEFLOW_INODEBEHAVIOUR_H
#pragma once

#include "ImNodeFlow/detail/fwd.h"
#include "ImNodeFlow/fwd.h"

#include <cstdint>
#include <memory>

namespace ImFlow {

/**
 * @brief Node behaviour strategy.
 *
 * This is what allows the user to customize the node's appearance and
 * behaviour
 */
class INodeBehaviour {
public:
    virtual ~INodeBehaviour();

    /**
     * @brief Called immediately after a node is constructed.
     */
    virtual void setup(NodeRef node);

    /**
     * @brief Called each frame. Responsible for the appearance of the
     * node's inner content
     */
    virtual void draw(NodeRef node);

    /**
     * @brief Fetches the underlying node's user data. (can be anything)
     */
    virtual uintptr_t getUserData();

    /**
     * @brief Final arbiter on whether node links are accepted or not.
     * @details This is to allow the user to customize the behaviour when a
     * new node link is created, and whether such link should be accepted.
     *
     * @param self Reference to the node with this behavior
     * @param inbound Reference to the node requesting an inbound connection
     */
    virtual bool acceptInboundNodeConnection(NodeRef self, NodeRef inbound);

    /**
     * @brief Called during a node cloning event. If not implemented or
     * nullptr is returned, the cloning event fails for the corresponding
     * node.
     */
    [[nodiscard]] virtual std::unique_ptr<INodeBehaviour> clone();
};

} // namespace ImFlow

#endif // IMNODEFLOW_INODEBEHAVIOUR_H
