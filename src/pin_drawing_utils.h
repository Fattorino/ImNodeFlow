#pragma once

#include "ImNodeFlow/fwd.h"

#include "imgui.h"

namespace ImFlow::detail {

void draw_bezier_from_pin_to_pos(
    ImDrawList* draw_list,
    ConstPinRef pin,
    ImVec2 pos);

} // namespace ImFlow::detail
