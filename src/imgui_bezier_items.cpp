#ifndef IMGUI_BEZIER_ITEMS_INL
#define IMGUI_BEZIER_ITEMS_INL
#pragma once

#include "imgui_bezier_items.h"

#include "./imgui_bezier_math.h"

#include "imgui.h"

namespace ImFlow::detail {

cubic_bezier_controls_t smart_bezier_controls_from_two_points(
    const ImVec2& p1,
    const ImVec2& p2)
{
    float distance = sqrt(pow((p2.x - p1.x), 2.f) + pow((p2.y - p1.y), 2.f));
    float delta = distance * 0.45f;
    if (p2.x < p1.x) {
        delta += 0.2f * (p1.x - p2.x);
        delta = std::min(150.F, delta);
    }
    // float vert = (p2.x < p1.x - 20.f) ? 0.062f * distance * (p2.y - p1.y) * 0.005f : 0.f;
    float vert = 0.f;
    ImVec2 p22 = p2 - ImVec2(delta, vert);
    ImVec2 p11 = p1 + ImVec2(delta, vert);

    return cubic_bezier_controls_t {
        p1, p11, p22, p2
    };
}

void smart_bezier(
    ImDrawList* draw_list,
    cubic_bezier_controls_t const& controls,
    ImU32 color,
    float thickness)
{
    draw_list->AddBezierCubic(
        controls.p1,
        controls.p2,
        controls.p3,
        controls.p4,
        color,
        thickness);
}

void smart_bezier_gradient(
    ImDrawList* draw_list,
    cubic_bezier_controls_t const& controls,
    ImU32 color1,
    ImU32 color2,
    float thickness,
    float blend)
{
    if (color1 == color2) {
        smart_bezier(draw_list, controls, color1, thickness);
        return;
    }

    ImVec2 p1 = controls.p1;
    ImVec2 p11 = controls.p2;
    ImVec2 p22 = controls.p3;
    ImVec2 p2 = controls.p4;

    const int segments = 32;
    ImVec4 c1 = ImGui::ColorConvertU32ToFloat4(color1);
    ImVec4 c2 = ImGui::ColorConvertU32ToFloat4(color2);
    ImVec2 prev = p1;
    for (int i = 1; i <= segments; ++i) {
        float t = (float)i / (float)segments;
        float u = 1.f - t;
        ImVec2 pt(
            u * u * u * p1.x + 3 * u * u * t * p11.x + 3 * u * t * t * p22.x + t * t * t * p2.x,
            u * u * u * p1.y + 3 * u * u * t * p11.y + 3 * u * t * t * p22.y + t * t * t * p2.y);
        float tm = ((float)i - 0.5f) / (float)segments;
        if (blend != 1.f) // symmetric contrast curve around the midpoint
            tm = (tm < 0.5f) ? 0.5f * powf(2.f * tm, blend) : 1.f - 0.5f * powf(2.f * (1.f - tm), blend);
        ImU32 col = ImGui::ColorConvertFloat4ToU32(ImLerp(c1, c2, tm));
        draw_list->AddLine(prev, pt, col, thickness);
        prev = pt;
    }
}

bool smart_bezier_collider(
    const ImVec2& p,
    cubic_bezier_controls_t const& ctrl,
    float radius)
{
    return ImProjectOnCubicBezier(p, ctrl.p1, ctrl.p2, ctrl.p3, ctrl.p4).Distance < radius;
}

} // namespace ImFlow::detail

#endif // IMGUI_BEZIER_ITEMS_INL
