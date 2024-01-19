#include "ImNodeFlow.h"

namespace ImFlow
{
    void BaseNode::update(ImVec2 offset)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImGui::PushID(this);

        draw_list->ChannelsSetCurrent(1); // Foreground
        ImGui::SetCursorScreenPos(offset + m_pos);
        ImGui::BeginGroup();
        draw();
        ImGui::EndGroup();

        m_size = ImGui::GetItemRectSize();
        draw_list->ChannelsSetCurrent(0); // Background
        if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            m_dragged = true;
        if(m_dragged)
        {
            m_pos += ImGui::GetIO().MouseDelta;
            if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                m_dragged = false;
        }
        draw_list->AddRectFilled(offset + m_pos - m_padding, offset + m_pos + m_size + m_padding, IM_COL32(60, 60, 60, 255), 4.0f);
        draw_list->AddRect(offset + m_pos - m_padding, offset + m_pos + m_size + m_padding, IM_COL32(100, 100, 100, 255), 4.0f);

        ImGui::PopID();
    }

    // -----------------------------------------------------------------------------------------------------------------

    void ImNodeFlow::update()
    {
        // Create our child canvas
        ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)", m_scroll.x, m_scroll.y);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(60, 60, 70, 200));
        ImGui::BeginChild(m_name.c_str(), ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
        ImGui::PopStyleVar(2); // WindowPadding
        ImGui::PopStyleColor();
        ImGui::PushItemWidth(120.0f);

        m_offset = ImGui::GetCursorScreenPos() + m_scroll;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Display grid
        ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
        float GRID_SZ = 64.0f;
        ImVec2 win_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetWindowSize();
        for (float x = fmodf(m_scroll.x, GRID_SZ); x < canvas_sz.x; x += 64)
            draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
        for (float y = fmodf(m_scroll.y, GRID_SZ); y < canvas_sz.y; y += 64)
            draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);

        // Display nodes
        draw_list->ChannelsSplit(2);
        for (auto& node : m_nodes)
        {
            node->update(m_offset);
        }
        draw_list->ChannelsMerge();

        // Scrolling
        if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
            m_scroll = m_scroll + ImGui::GetIO().MouseDelta;

        ImGui::EndChild();
    }
}
