#pragma once

#include "imgui.h"

namespace ImFlow::compat {

inline void AddRect(
    ImDrawList* draw_list,
    const ImVec2& p_min,
    const ImVec2& p_max,
    ImU32 col,
    float rounding,
    float thickness,
    ImDrawFlags flags = ImDrawFlags_None)
{
#if IMGUI_VERSION_NUM >= 19280
    draw_list->AddRect(p_min, p_max, col, rounding, thickness, flags);
#else
    draw_list->AddRect(p_min, p_max, col, rounding, flags, thickness);
#endif
}

} // namespace ImFlow::detail
