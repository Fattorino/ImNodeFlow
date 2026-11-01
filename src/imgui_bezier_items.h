#ifndef IMGUI_BEZIER_ITEMS_H
#define IMGUI_BEZIER_ITEMS_H
#pragma once

#include "imgui.h"

namespace ImFlow::detail {

struct cubic_bezier_controls_t {
    ImVec2 p1;
    ImVec2 p2;
    ImVec2 p3;
    ImVec2 p4;
};

cubic_bezier_controls_t smart_bezier_controls_from_two_points(
    const ImVec2& p1,
    const ImVec2& p2);

/**
 * @brief Draw a sensible bezier between two points
 * @param draw_list The draw list to draw to
 * @param line Bezier curve description
 * @param color Color of the curve
 * @param thickness Thickness of the curve
 */
void smart_bezier(
    ImDrawList* draw_list,
    cubic_bezier_controls_t const& line,
    ImU32 color,
    float thickness);

/**
 * @brief Draw a sensible bezier whose color blends from one end to the other
 * @param draw_list The draw list to draw to
 * @param line Bezier curve description
 * @param color1 Color at the starting point
 * @param color2 Color at the ending point
 * @param thickness Thickness of the curve
 * @param blend Tightness of the color transition. 1 = linear, >1 concentrates it toward the middle, <1 spreads it
 */
void smart_bezier_gradient(
    ImDrawList* draw_list,
    cubic_bezier_controls_t const& line,
    ImU32 color1,
    ImU32 color2,
    float thickness,
    float blend = 1.f);

/**
 * @brief Collider checker for smart_bezier
 * @details Projects the point "p" orthogonally onto the bezier curve and
 *	    checks if the distance is less than the given radius.
 * @param p Point to be tested
 * @param line Bezier curve description
 * @param radius Lateral width of the hit box
 * @return [TRUE] if "p" is inside the collider
 *
 * Intended to be used in union with smart_bezier();
 */
bool smart_bezier_collider(
    const ImVec2& p,
    cubic_bezier_controls_t const& line,
    float radius);

} // namespace ImFlow::detail

#endif // IMGUI_BEZIER_ITEMS_H
