#include <iostream>
#include "ImNodeFlow.h"

namespace ImFlow
{
    // -----------------------------------------------------------------------------------------------------------------
    // LINK

    void Link::update()
    {
        ImVec2 start = m_left->pinPoint();
        ImVec2 end  = m_right->pinPoint();
        float thickness = m_inf->style().link_thickness;
        bool mouseClickState = m_inf->getSingleUseClick();

        if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            m_selected = false;

        if (smart_bezier_collider(ImGui::GetMousePos(), start, end, 2.5))
        {
            m_hovered = true;
            thickness = m_inf->style().link_hovered_thickness;
            if (mouseClickState)
            {
                m_inf->consumeSingleUseClick();
                m_selected = true;
            }
        }
        else { m_hovered = false; }

        if (m_selected)
            smart_bezier(start, end, m_inf->style().colors.link_selected_outline, thickness + m_inf->style().link_selected_outline_thickness);
        smart_bezier(start, end, m_inf->style().colors.link, thickness);

        if (m_selected && ImGui::IsKeyPressed(ImGuiKey_Delete, false))
            m_right->deleteLink();
    }

    Link::~Link()
    {
        m_left->deleteLink();
    }

    // -----------------------------------------------------------------------------------------------------------------
    // BASE NODE

    bool BaseNode::hovered()
    {
        return ImGui::IsMouseHoveringRect(m_inf->content2canvas(m_pos - m_paddingTL), m_inf->content2canvas(m_pos + m_size + m_paddingBR));
    }

    void BaseNode::update(ImVec2& offset)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImGui::PushID(this);
        bool mouseClickState = m_inf->getSingleUseClick();

        draw_list->ChannelsSetCurrent(1); // Foreground
        ImGui::SetCursorScreenPos(offset + m_pos);

        ImGui::BeginGroup();

        // Header
        ImGui::BeginGroup();
        ImGui::TextColored(m_inf->style().colors.node_header_title, m_name.c_str());
        ImGui::Spacing();
        ImGui::EndGroup();
        float headerH = ImGui::GetItemRectSize().y;
        float titleW = ImGui::GetItemRectSize().x;

        // Inputs
        ImGui::BeginGroup();
        for(auto& p : m_ins)
        {
            p.second->pos(ImGui::GetCursorPos());
            p.second->update();
        }
        ImGui::EndGroup();
        ImGui::SameLine();

        // Content
        ImGui::BeginGroup();
        draw();
        ImGui::Dummy(ImVec2(0.f, 0.f));
        ImGui::EndGroup();
        ImGui::SameLine();

        // Outputs
        float maxW = 0.0f;
        for (auto& p : m_outs)
        {
            float w = p.second->calcWidth();
            if (w > maxW)
                maxW = w;
        }
        ImGui::BeginGroup();
        for (auto& p : m_outs)
        {
            // FIXME: This looks horrible
            if (m_inf->content2canvas(m_pos + ImVec2(titleW, 0)).x < ImGui::GetCursorPos().x + ImGui::GetWindowPos().x + maxW)
                p.second->pos(ImGui::GetCursorPos() + ImGui::GetWindowPos() + ImVec2(maxW - p.second->calcWidth(), 0.f));
            else
                p.second->pos(ImVec2(m_inf->content2canvas(m_pos + ImVec2(titleW - p.second->calcWidth(), 0)).x, ImGui::GetCursorPos().y + ImGui::GetWindowPos().y));
            p.second->update();
        }
        ImGui::EndGroup();

        ImGui::EndGroup();
        m_size = ImGui::GetItemRectSize();
        ImVec2 headerSize = ImVec2(m_size.x + m_paddingTL.x, headerH);

        // Background
        draw_list->ChannelsSetCurrent(0);
        draw_list->AddRectFilled(offset + m_pos - m_paddingTL, offset + m_pos + m_size + m_paddingBR, m_inf->style().colors.node_bg, m_inf->style().node_radius);
        draw_list->AddRectFilled(offset + m_pos - m_paddingTL, offset + m_pos + headerSize, m_inf->style().colors.node_header, m_inf->style().node_radius, ImDrawFlags_RoundCornersTop);
        if(m_selected)
            draw_list->AddRect(offset + m_pos - m_paddingTL, offset + m_pos + m_size + m_paddingBR, m_inf->style().colors.node_selected_border, m_inf->style().node_radius, 0, m_inf->style().node_border_selected_thickness);
        else
            draw_list->AddRect(offset + m_pos - m_paddingTL, offset + m_pos + m_size + m_paddingBR, m_inf->style().colors.node_border, m_inf->style().node_radius, 0, m_inf->style().node_border_thickness);


        if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !m_inf->on_selected_node())
            selected(false);

        if (hovered() && mouseClickState)
        {
            selected(true);
            m_inf->consumeSingleUseClick();
        }

        bool onHeader = ImGui::IsMouseHoveringRect(offset + m_pos - m_paddingTL, offset + m_pos + headerSize);
        if (onHeader && mouseClickState)
        {
            m_inf->consumeSingleUseClick();
            m_dragged = true;
            m_inf->draggingNode(true);
        }
        if(m_dragged || (m_selected && m_inf->draggingNode()))
        {
            float step = m_inf->style().grid_size / m_inf->style().grid_subdivisions;
            m_posTarget += ImGui::GetIO().MouseDelta;
            // "Slam" The position
            m_pos.x = round(m_posTarget.x / step) * step;
            m_pos.y = round(m_posTarget.y / step) * step;

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                m_dragged = false;
                m_inf->draggingNode(false);
                m_posTarget = m_pos;
            }
        }
        ImGui::PopID();
    }

    // -----------------------------------------------------------------------------------------------------------------
    // HANDLER

    int ImNodeFlow::m_instances = 0;

    bool ImNodeFlow::on_selected_node()
    {
        return std::any_of(m_nodes.begin(), m_nodes.end(),
                           [](auto& n) { return n->selected() && n->hovered();});
    }

    bool ImNodeFlow::on_free_space()
    {
        return std::all_of(m_nodes.begin(), m_nodes.end(),
                    [](auto& n) {return !n->hovered();})
               && std::all_of(m_links.begin(), m_links.end(),
                    [](auto& l) {return !l.lock()->hovered();});
    }

    ImVec2 ImNodeFlow::content2canvas(const ImVec2& p)
    {
        return p + m_context.scroll() + ImGui::GetWindowPos();
    }

    ImVec2 ImNodeFlow::canvas2screen(const ImVec2 &p)
    {
        return (p + m_context.scroll()) * m_context.scale() + m_context.origin();
    }

    ImVec2 ImNodeFlow::screen2content(const ImVec2 &p)
    {
        return p - m_context.scroll();
    }

    ImVec2 ImNodeFlow::screen2canvas(const ImVec2 &p)
    {
        return p - pos() - m_context.scroll();
    }

    void ImNodeFlow::addLink(std::shared_ptr<Link>& link)
    {
        m_links.push_back(link);
    }

    void ImNodeFlow::update()
    {
        // Updating looping stuff
        m_hovering = nullptr;
        m_draggingNode = m_draggingNodeNext;
        m_singleUseClick = ImGui::IsMouseClicked(ImGuiMouseButton_Left);

        // Create child canvas
        m_context.begin();

        ImVec2 offset = ImGui::GetCursorScreenPos() + m_context.scroll();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Display grid
        ImVec2 win_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetWindowSize();
        for (float x = fmodf(m_context.scroll().x, m_style.grid_size); x < canvas_sz.x; x += m_style.grid_size)
            draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, m_style.colors.grid);
        for (float y = fmodf(m_context.scroll().y, m_style.grid_size); y < canvas_sz.y; y += m_style.grid_size)
            draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, m_style.colors.grid);
        for (float x = fmodf(m_context.scroll().x, m_style.grid_size / m_style.grid_subdivisions); x < canvas_sz.x; x += m_style.grid_size / m_style.grid_subdivisions)
            draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, m_style.colors.subGrid);
        for (float y = fmodf(m_context.scroll().y, m_style.grid_size / m_style.grid_subdivisions); y < canvas_sz.y; y += m_style.grid_size / m_style.grid_subdivisions)
            draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, m_style.colors.subGrid);

        // Update and draw nodes
        draw_list->ChannelsSplit(2);
        for (auto& node : m_nodes) { node->update(offset); }
        draw_list->ChannelsMerge();
        for (auto& node : m_nodes) { node->updatePublicStatus(); }

        // Update and draw links
        for (auto& l : m_links) { if(!l.expired()) l.lock()->update(); }

        // Links drop-off
        if(m_dragOut && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            if(!m_hovering)
            {
                if(on_free_space() && m_droppedLinkPopUp)
                {
                    if (m_droppedLinkPupUpComboKey == ImGuiKey_None || ImGui::IsKeyDown(m_droppedLinkPupUpComboKey))
                    {
                        m_droppedLinkLeft = m_dragOut;
                        ImGui::OpenPopup("DroppedLinkPopUp");
                    }
                }
            }
            else
                m_dragOut->createLink(m_hovering);
        }

        // Links drag-out
        if (!m_draggingNode && m_hovering && !m_dragOut && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            m_dragOut = m_hovering;
        if (m_dragOut)
        {
            if (m_dragOut->type() == PinType_Output)
                smart_bezier(m_dragOut->pinPoint(), ImGui::GetMousePos(), m_style.colors.drag_out_link, m_style.drag_out_link_thickness);
            else
                smart_bezier(ImGui::GetMousePos(), m_dragOut->pinPoint(), m_style.colors.drag_out_link, m_style.drag_out_link_thickness);

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                m_dragOut = nullptr;
        }

        // Deletion of selected stuff
        if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
        {
            m_nodes.erase(std::remove_if(m_nodes.begin(), m_nodes.end(),
                           [](const std::shared_ptr<BaseNode>& n) { return n->selected(); }), m_nodes.end());
        }

        // Right-click PopUp
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered() && on_free_space())
        {
            if (m_rightClickPopUp)
                ImGui::OpenPopup("RightClickPopUp");
        }
        if (ImGui::BeginPopup("RightClickPopUp"))
        {
            m_rightClickPopUp();
            ImGui::EndPopup();
        }

        // Dropped Link PopUp
        if (ImGui::BeginPopup("DroppedLinkPopUp"))
        {
            m_droppedLinkPopUp(m_droppedLinkLeft);
            ImGui::EndPopup();
        }

        // Removing dead Links
        m_links.erase(std::remove_if(m_links.begin(), m_links.end(),
                                     [](const std::weak_ptr<Link>& l) { return l.expired(); }), m_links.end());

        m_context.end();
    }
}
