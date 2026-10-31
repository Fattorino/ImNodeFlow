#ifndef IMNODEFLOW_CALLBACK_TYPES_H
#define IMNODEFLOW_CALLBACK_TYPES_H
#pragma once

#include "ImNodeFlow/fwd.h"

#include "imgui.h"

#include <functional>
#include <optional>

namespace ImFlow {

using right_click_popup_handler_t = std::function<void(std::optional<NodeRef> node)>;

using dropped_link_popup_handler_t = std::function<void(PinRef dragged_pin)>;

using drag_drop_target_handler_t = std::function<void(const ImVec2& grid_coords)>;

using pin_rendering_handler_t = std::function<void(PinRef pin)>;

template <typename T>
using in_pin_accumulator_t = std::function<T(bool first_pin, T init, T const& incoming_value)>;

template <typename T>
using out_pin_behaviour_t = std::function<T(NodeRef)>;

using lambda_draw_func_t = std::function<void(NodeRef)>;

} // namespace ImFlow

#endif // IMNODEFLOW_CALLBACK_TYPES_H
