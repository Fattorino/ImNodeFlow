#include "ImNodeFlow.h"

namespace ImFlow
{
    void BaseNode::update(ImVec2 offset, int i)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImGui::PushID(this);

        draw_list->ChannelsSetCurrent(i + 1); // Foreground
        ImGui::SetCursorScreenPos(offset + m_pos);

        ImGui::BeginGroup();

        ImGui::BeginGroup();
        ImGui::Text(m_name.c_str());
        ImGui::Spacing();
        ImGui::EndGroup();
        float headerH = ImGui::GetItemRectSize().y;

        ImGui::BeginGroup();
        for(auto& p : m_ins)
        {
            p->setPos(ImGui::GetCursorPos() + ImGui::GetWindowPos());
            p->update();
        }
        ImGui::EndGroup();
        ImGui::SameLine();

        ImGui::BeginGroup();
        draw();
        ImGui::EndGroup();
        ImGui::SameLine();

        ImGui::BeginGroup();
        for (auto& p : m_outs)
        {
            p->setPos(ImGui::GetCursorPos() + ImGui::GetWindowPos());
            p->update();
        }
        ImGui::EndGroup();
        ImGui::SameLine();

        ImGui::EndGroup();

        m_size = ImGui::GetItemRectSize();
        ImVec2 headerSize = ImVec2(m_size.x + m_padding.x, headerH);
        draw_list->ChannelsSetCurrent(i); // Background
        if (!ImGui::IsMouseHoveringRect(offset + m_pos - m_padding, offset + m_pos + headerSize) && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            m_dragDeny = true;
            m_inf->dragAllowed(false);
        }
        if (ImGui::IsMouseHoveringRect(offset + m_pos - m_padding, offset + m_pos + headerSize) && ImGui::IsMouseDown(ImGuiMouseButton_Left) && !m_dragDeny)
            m_dragged = true;
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            m_dragDeny = false;
            m_dragged = false;
            m_inf->dragAllowed(true);
        }
        if(m_dragged)
            m_pos += ImGui::GetIO().MouseDelta;

        draw_list->AddRectFilled(offset + m_pos - m_padding, offset + m_pos + m_size + m_padding, IM_COL32(60, 60, 60, 255), 4.0f);
        draw_list->AddRectFilled(offset + m_pos - m_padding, offset + m_pos + headerSize, IM_COL32(40, 40, 40, 255), 4.0f);
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

        ImVec2 offset = ImGui::GetCursorScreenPos() + m_scroll;
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
        draw_list->ChannelsSplit(2 * m_nodes.size() + 1);
        int i = 1;
        for (auto& node : m_nodes)
        {
            node->update(offset, i);
            i += 2;
        }
        draw_list->ChannelsMerge();

        // Scrolling
        if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
            m_scroll = m_scroll + ImGui::GetIO().MouseDelta;

        ImGui::EndChild();

        m_isLinking = m_isLinkingNext;
    }
}
