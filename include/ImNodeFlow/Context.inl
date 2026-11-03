#pragma once

#include "ImNodeFlow/Context.h"

#include "ImNodeFlow/NodeRef.h"
#include "ImNodeFlow/fwd.h"

#include "imgui.h"

namespace ImFlow {

template <typename T, typename... Params>
NodeRef ImNodeFlow::emplaceNode(const ImVec2& grid_coords, Params&&... args)
{
    return addNode(
        grid_coords,
        NodeInfo {
            .behaviour = std::make_unique<T>(std::forward<Params>(args)...),
        });
}

template <typename T, typename... Params>
NodeRef ImNodeFlow::emplaceNodeAt(const ImVec2& screen_coords, Params&&... args)
{
    return placeNodeAt(
        screen_coords,
        NodeInfo {
            .behaviour = std::make_unique<T>(std::forward<Params>(args)...),
        });
}

template <typename T, typename... Params>
NodeRef ImNodeFlow::emplaceNodeOnMouse(Params&&... args)
{
    return placeNode(
        NodeInfo {
            .behaviour = std::make_unique<T>(std::forward<Params>(args)...),
        });
}

} // namespace ImFlow
