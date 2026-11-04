#ifndef IMNODEFLOW_EDITOR_CONTEXT_H
#define IMNODEFLOW_EDITOR_CONTEXT_H
#include "ImNodeFlow/detail/fwd.h"
#include <memory>
#pragma once

#include "ImNodeFlow/NodeUID.h"
#include "ImNodeFlow/Style.h"
#include "ImNodeFlow/callback_types.h"

#include "./context_wrapper.h"
#include "./node.h"
#include "./node_group.h"
#include "./pin_link.h"

#include <string>
#include <vector>

template <>
struct std::hash<ImFlow::NodeUID> {
    constexpr std::size_t operator()(const ImFlow::NodeUID& n) const noexcept
    {
        return std::hash<uint64_t> { }(n.id);
    }
};

namespace ImFlow::detail {

struct editor_context_t {
    std::string name;
    ContainedContext context;

    bool is_drawing = false;

    std::vector<std::unique_ptr<node_t>> newNodesQueue;

    detail::node_group_arena_t root_arena;
    std::vector<std::unique_ptr<pin_link_t>> pin_links;
    std::vector<std::unique_ptr<node_link_t>> node_links;

    dropped_link_popup_handler_t droppedLinkPopUp;
    ImGuiKey droppedLinkPopUpComboKey = ImGuiKey_None;
    pin_base_t* droppedLinkLeft = nullptr;
    right_click_popup_handler_t rightClickPopUp;
    drag_drop_target_handler_t dragdropTarget;

    ImVec2 scroll_start;

    node_t* hoveredNode = nullptr;
    ImVec2 nodeDragStart;
    ImVec2 nodeDragEnd;
    bool nodeDragActive = false;
    bool nodeDragActiveNext = false;
    bool nodeDragActiveNext2 = false;

    InfStyler style;

    struct SelectionBox {
        bool active = false;
        ImVec2 selectionStart;
        ImU32 selectionColor = IM_COL32(100, 150, 255, 50);
        ImU32 borderColor = IM_COL32(100, 150, 255, 200);
    };

    SelectionBox box;

    bool draggingCanvas = false;
    bool mouseOnSelectedNode = false;
};

} // namespace ImFlow::detail

#endif // IMNODEFLOW_EDITOR_CONTEXT_H
