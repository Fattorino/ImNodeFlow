#ifndef IM_NODE_FLOW_FWD
#define IM_NODE_FLOW_FWD
#pragma once

namespace ImFlow {

template <typename T, bool TIsConst = false>
class InPinRef;
template <typename T, bool TIsConst = false>
class OutPinRef;
template <bool TIsConst = false>
class PinRefImpl;
template <bool TIsConst = false>
class NodeRefImpl;
template <bool TIsConst = false>
class NodeGroupArenaRef;

using PinRef = PinRefImpl<false>;
using ConstPinRef = PinRefImpl<true>;
using NodeRef = NodeRefImpl<false>;
using ConstNodeRef = NodeRefImpl<true>;

class IContainedContext;
class INodeBehaviour;
class ImNodeFlow;

} // namespace ImFlow

#endif // IM_NODE_FLOW_FWD
