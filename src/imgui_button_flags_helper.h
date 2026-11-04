#pragma once

#include "imgui.h"

namespace ImFlow::detail {

inline ImGuiButtonFlags getButtonFlagsForMouseButton(ImGuiMouseButton button)
{
    switch (button) {
    case ImGuiMouseButton_Left:
        return ImGuiButtonFlags_MouseButtonLeft;
    case ImGuiMouseButton_Right:
        return ImGuiButtonFlags_MouseButtonRight;
    case ImGuiMouseButton_Middle:
        return ImGuiButtonFlags_MouseButtonMiddle;
    default:
        return 0;
    }
}

} // namespace ImFlow::detail
