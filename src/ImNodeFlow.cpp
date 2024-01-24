#include "ImNodeFlow.h"

namespace ImFlow
{
    // -----------------------------------------------------------------------------------------------------------------
    // LINK



    // -----------------------------------------------------------------------------------------------------------------
    // BASE NODE

    void BaseNode::update(ImVec2& offset)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImGui::PushID(this);

        draw_list->ChannelsSetCurrent(1); // Foreground
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
            p->pos(ImGui::GetCursorPos() + ImGui::GetWindowPos());
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
            p->pos(ImGui::GetCursorPos() + ImGui::GetWindowPos());
            p->update();
        }
        ImGui::EndGroup();
        ImGui::SameLine();

        ImGui::EndGroup();

        m_size = ImGui::GetItemRectSize();
        ImVec2 headerSize = ImVec2(m_size.x + m_padding.x, headerH);
        draw_list->ChannelsSetCurrent(0); // Background
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
    // HANDLER

    void ImNodeFlow::createLink(uintptr_t left, uintptr_t right)
    {
        reinterpret_cast<Pin*>(right)->setLink(m_links.emplace_back(std::make_shared<Link>(left, right)));
    }

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

        // Update and draw nodes
        draw_list->ChannelsSplit(2);
        for (auto& node : m_nodes)
        {
            node->update(offset);
        }
        draw_list->ChannelsMerge();

        //  Deselection
        if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            for (auto& l : m_links)
            {
                l->selected(false);
            }
        }

        // Draw links
        for (auto& l : m_links)
        {
            auto* leftPin = reinterpret_cast<Pin*>(l->left());
            auto* rightPin = reinterpret_cast<Pin*>(l->right());
            ImVec2 start = leftPin->pos() + ImVec2(leftPin->size().x, leftPin->size().y / 2);
            ImVec2 end = rightPin->pos() + ImVec2(0, leftPin->size().y / 2);
            if (ImProjectOnCubicBezier(ImGui::GetMousePos(), start, start + ImVec2(50, 0), end - ImVec2(50, 0), end).Distance < 2.5)
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                    l->selected(true);

            if (l->selected())
                draw_list->AddBezierCubic(start, start + ImVec2(30, 0), end - ImVec2(30, 0), end, IM_COL32(80, 20, 255, 255), 4.0f);
            draw_list->AddBezierCubic(start, start + ImVec2(30, 0), end - ImVec2(30, 0), end, IM_COL32(200, 200, 100, 255), 2.8f);
        }

        // Deletion of selected stuff
        if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
        {
            std::vector<int> deletions;

            for (int i = 0; i < m_links.size(); i++)
                if (m_links[i]->selected())
                    deletions.emplace_back(i);
            for (int& i : deletions)
                m_links.erase(m_links.begin() + i);
            deletions.clear();

            // TODO: Do the same for Nodes
        }

        // Scrolling
        if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
            m_scroll = m_scroll + ImGui::GetIO().MouseDelta;

        ImGui::EndChild();

        m_dragAllowed = m_dragAllowedNext;
        m_isLinking = m_isLinkingNext;

        ImGui::Begin("Debug");
        if (ImGui::BeginListBox("List"))
        {
            for ( auto& l : m_links)
            {
                auto* leftPin = reinterpret_cast<Pin*>(l->left());
                auto* rightPin = reinterpret_cast<Pin*>(l->right());
                ImGui::Text("Link: %s:%s -> %s:%s", leftPin->parent()->m_name.c_str(), leftPin->name().c_str(), rightPin->parent()->m_name.c_str(), rightPin->name().c_str());
            }
            ImGui::EndListBox();
        }
        ImGui::End();
    }
}
