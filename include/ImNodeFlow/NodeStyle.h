#ifndef IMNODEFLOW_NODESTYLE_H
#define IMNODEFLOW_NODESTYLE_H
#pragma once

#include "imgui.h"

namespace ImFlow {

/**
 * @brief Defines the visual appearance of a node
 */
class NodeStyle {
public:
    /// @brief Body's background color
    ImU32 bg = IM_COL32(55, 64, 75, 255);
    /// @brief Body's background color when hovered
    ImU32 bg_hovered = IM_COL32(80, 94, 110, 255);
    /// @brief Header's background color
    ImU32 header_bg = IM_COL32(71, 142, 173, 255);
    /// @brief Header title color
    ImColor header_title_color = ImColor(233, 241, 244, 255);
    /// @brief Border color
    ImU32 border_color = IM_COL32(30, 38, 41, 140);
    /// @brief Border color when selected
    ImU32 border_selected_color = IM_COL32(170, 190, 205, 230);

    /// @brief Body's content padding (Left Top Right Bottom)
    ImVec4 padding = ImVec4(6.0f, 6.f, 6.0f, 2.f);
    /// @brief Edges rounding
    float radius = 6.5f;
    /// @brief Border thickness
    float border_thickness = -1.35f;
    /// @brief Border thickness when selected
    float border_selected_thickness = 2.f;

    /// @brief Link thickness
    float link_thickness = 3.f;
    /// @brief Link thickness when dragged
    float link_dragged_thickness = 2.2f;
    /// @brief Link thickness when hovered
    float link_hovered_thickness = 4.f;
    /// @brief Thickness of the outline of a selected link
    float link_selected_outline_thickness = 1.f;
};

namespace NodeStyles {
    /// @brief Default cyan style
    static constexpr NodeStyle cyan {
        .header_bg = IM_COL32(71, 142, 173, 255),
        .header_title_color = ImColor(233, 241, 244, 255),
        .radius = 6.5f,
    };

    /// @brief Default green style
    static constexpr NodeStyle green {
        .header_bg = IM_COL32(90, 191, 93, 255),
        .header_title_color = ImColor(233, 241, 244, 255),
        .radius = 3.5f,
    };

    /// @brief Default red style
    static constexpr NodeStyle red {
        .header_bg = IM_COL32(191, 90, 90, 255),
        .header_title_color = ImColor(233, 241, 244, 255),
        .radius = 11.f,
    };

    /// @brief Default brown style
    static constexpr NodeStyle brown {
        .header_bg = IM_COL32(191, 134, 90, 255),
        .header_title_color = ImColor(233, 241, 244, 255),
        .radius = 6.5f,
    };

    static inline constexpr NodeStyle default_style = cyan;

}

} // namespace ImFlow

#endif // IMNODEFLOW_NODESTYLE_H
