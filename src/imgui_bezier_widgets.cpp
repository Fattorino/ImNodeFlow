#include "imgui_bezier_widgets.h"

#include "imgui.h"
#include "imgui_bezier_items.h"
#include "imgui_internal.h"

namespace ImFlow::detail {

bool smart_bezier_widget(
    ImGuiID item_id,
    cubic_bezier_controls_t const& line,
    ImU32 color1,
    ImU32 color2,
    bool selected,
    bool* hovered,
    float thickness,
    float blend)
{
    auto mouse_pos = ImGui::GetMousePos();
    bool is_hovered = detail::smart_bezier_collider(mouse_pos, line, 2.5);
    bool is_pressed = false;

    if (is_hovered) {
        ImRect always_valid_bb {
            mouse_pos - ImVec2(1.F, 1.F),
            mouse_pos + ImVec2(1.F, 1.F),
        };

        if (ImGui::ItemAdd(always_valid_bb, item_id)) {
            is_pressed = ImGui::ButtonBehavior(
                always_valid_bb,
                item_id,
                &is_hovered,
                nullptr,
                ImGuiButtonFlags_PressedOnClick);
        }
    }

    auto start_color = color1;
    auto end_color = color2;

    if (selected) {
        double t = ImGui::GetTime();

        float v0 = std::sin(t * 2.F) * 0.1F + 0.85F;
        float v1 = std::sin(t * 2.F + 2.F) * 0.1F + 0.85F;

        ImVec4 start_rgb { v0, v0, v0, 1.F };
        ImVec4 end_rgb { v1, v1, v1, 1.F };

        start_color = ImGui::ColorConvertFloat4ToU32(start_rgb);
        end_color = ImGui::ColorConvertFloat4ToU32(end_rgb);
    }

    smart_bezier_gradient(
        ImGui::GetWindowDrawList(),
        line,
        start_color,
        end_color,
        thickness,
        blend);

    if (hovered != nullptr) {
        *hovered = is_hovered;
    }

    return is_pressed;
}

} // namespace ImFlow::detail
