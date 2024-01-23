#pragma once

#include "ImNodeFlow.h"

namespace ImFlow
{
    // -----------------------------------------------------------------------------------------------------------------
    // HANDLER

    template<typename T>
    void ImNodeFlow::addNode(const std::string& name, const ImVec2&& pos)
    {
        static_assert(std::is_base_of<BaseNode, T>::value, "Pushed type is not subclass of BaseNode!");
        m_nodes.emplace_back(std::make_shared<T>(name, pos, this));
    }

    // -----------------------------------------------------------------------------------------------------------------
    // BASE NODE

    template<typename T>
    void BaseNode::addIN(const char* name, BaseNode* parent)
    {
        m_ins.emplace_back(std::make_shared<InPin<T>>(name, parent, m_inf));
    }

    template<typename T>
    void BaseNode::addOUT(const char* name, BaseNode* parent)
    {
        m_outs.emplace_back(std::make_shared<OutPin<T>>(name, parent, m_inf));
    }

    template<typename T>
    InPin<T>& BaseNode::ins(int i) { return *std::dynamic_pointer_cast<InPin<T>>(m_ins[i]); }
    template<typename T>
    OutPin<T>& BaseNode::outs(int i) { return *std::dynamic_pointer_cast<OutPin<T>>(m_outs[i]); }

    // -----------------------------------------------------------------------------------------------------------------
    // IN PIN

    template<class T>
    const T& InPin<T>::val()
    {
        if(m_link)
        {
            auto* leftPin = reinterpret_cast<OutPin<T>*>(m_link->left());
            return leftPin->val();
        }
        else
        {
            return defaultVal;
        }
    }

    template<class T>
    void InPin<T>::update()
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImGui::Text(m_name.c_str());
        m_size = ImGui::GetItemRectSize();
        if (ImGui::IsItemHovered())
            draw_list->AddRectFilled(m_pos - ImVec2(3,1), m_pos + m_size + ImVec2(3,2), IM_COL32(100, 100, 255, 70));
        ImVec2 pinDot = m_pos + ImVec2(-1 * m_parent->padding().x, m_size.y / 2);
        draw_list->AddRect(m_pos - ImVec2(3,1), m_pos + m_size + ImVec2(3,2), IM_COL32(255, 255, 255, 100));
        draw_list->AddCircleFilled(pinDot, 5, IM_COL32(50, 40, 40, 255));

        // new link drop-off
        if(ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
        {
            if(m_inf->isLinking())
            {
                m_inf->isLinking(false);
                auto* leftPin = reinterpret_cast<Pin*>(m_inf->pinTarget());
                if ((void *)leftPin->parent() == (void *)m_parent)
                    return;
                if (m_link)
                {
                    int i = 0;
                    for (auto& l : m_inf->links())
                    {
                        if(l.right() == me() && l.left() == m_inf->pinTarget())
                        {
                            m_inf->links().erase(m_inf->links().begin() + i);
                            return;
                        }
                        if(l.right() == me())
                        {
                            m_inf->links().erase(m_inf->links().begin() + i);
                            break;
                        }
                        i++;
                    }
                }
                setLink(m_inf->createLink(m_inf->pinTarget(), me()));
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    // OUT PIN

    template<class T>
    const T &OutPin<T>::val() { m_parent->resolve(m_me); return m_val; } // TODO: Resolve ME somewhere else so it's not done every frame

    template<class T>
    void OutPin<T>::update()
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImGui::Text(m_name.c_str());
        m_size = ImGui::GetItemRectSize();
        if (ImGui::IsItemHovered())
            draw_list->AddRectFilled(m_pos - ImVec2(3,1), m_pos + m_size + ImVec2(3,2), IM_COL32(100, 100, 255, 70));
        ImVec2 pinDot = m_pos + ImVec2(m_size.x + m_parent->padding().x, m_size.y / 2);
        draw_list->AddRect(m_pos - ImVec2(3,1), m_pos + m_size + ImVec2(3,2), IM_COL32(255, 255, 255, 100));
        draw_list->AddCircleFilled(pinDot, 5, IM_COL32(50, 40, 40, 255));

        if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_inf->dragAllowed())
        {
            m_dragging = true;
            m_inf->dragAllowed(false);
            m_inf->isLinking(true);
            m_inf->pinTarget() = me();
        }
        if (m_dragging)
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                m_dragging = false;
                m_inf->dragAllowed(true);
                m_inf->isLinking(false);
            }
            if(!ImGui::IsItemHovered())
                draw_list->AddBezierCubic(pinDot, pinDot + ImVec2(30, 0), ImGui::GetMousePos() - ImVec2(30, 0), ImGui::GetMousePos(), IM_COL32(200, 200, 100, 255), 3.0f);
        }
    }
}
