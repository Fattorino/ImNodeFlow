#include "ImNodeFlow/INodeBehaviour.h"
#include "ImNodeFlow/NodeRef.h"
#include "ImNodeFlow/fwd.h"

#include <cstdint>
#include <memory>

namespace ImFlow {

INodeBehaviour::~INodeBehaviour() = default;

void INodeBehaviour::setup(NodeRef node) { }

void INodeBehaviour::draw(NodeRef node) { }

void INodeBehaviour::prepareNextFrame(NodeRef node) { }

uintptr_t INodeBehaviour::getUserData() { return 0; }

bool INodeBehaviour::acceptInboundNodeConnection(NodeRef self, NodeRef inbound)
{
    return true;
}

std::unique_ptr<INodeBehaviour> INodeBehaviour::clone()
{
    return { };
}

} // namespace ImFlow
