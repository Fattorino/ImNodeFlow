#pragma once

#include "ImNodeFlow.h"

namespace ImFlow
{
    inline void smart_bezier(const ImVec2& p1, const ImVec2& p2, ImU32 color, float thickness)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        float distance = sqrt(pow((p2.x - p1.x), 2.f) + pow((p2.y - p1.y), 2.f));
        float delta = distance * 0.45f;
        if (p2.x < p1.x) delta += 0.2f * (p1.x - p2.x);
        // float vert = (p2.x < p1.x - 20.f) ? 0.062f * distance * (p2.y - p1.y) * 0.005f : 0.f;
        float vert = 0.f;
        ImVec2 p22 = p2 - ImVec2(delta, vert);
        if (p2.x < p1.x - 50.f) delta *= -1.f;
        ImVec2 p11 = p1 + ImVec2(delta, vert);
        dl->AddBezierCubic(p1, p11, p22, p2, color, thickness);
    }

    inline bool smart_bezier_collider(const ImVec2& p, const ImVec2& p1, const ImVec2& p2, float radius)
    {
        float distance = sqrt(pow((p2.x - p1.x), 2.f) + pow((p2.y - p1.y), 2.f));
        float delta = distance * 0.45f;
        if (p2.x < p1.x) delta += 0.2f * (p1.x - p2.x);
        // float vert = (p2.x < p1.x - 20.f) ? 0.062f * distance * (p2.y - p1.y) * 0.005f : 0.f;
        float vert = 0.f;
        ImVec2 p22 = p2 - ImVec2(delta, vert);
        if (p2.x < p1.x - 50.f) delta *= -1.f;
        ImVec2 p11 = p1 + ImVec2(delta, vert);
        return ImProjectOnCubicBezier(p, p1, p11, p22, p2).Distance < radius;
    }

    // -----------------------------------------------------------------------------------------------------------------
    // HANDLER

    template<typename T, typename... Params>
    T* ImNodeFlow::addNode(const std::string& name, const ImVec2& pos, std::shared_ptr<NodeStyle> style, Params&&... args)
    {
        static_assert(std::is_base_of<BaseNode, T>::value, "Pushed type is not a subclass of BaseNode!");
        m_nodes.emplace_back(std::make_shared<T>(name, pos, this, std::move(style), std::forward<Params>(args)...));
        return static_cast<T*>(m_nodes.back().get());
    }

    template<typename T, typename... Params>
    T* ImNodeFlow::placeNode(const std::string& name, std::shared_ptr<NodeStyle> style, Params&&... args)
    {
        return placeNodeAt<T>(name, ImGui::GetMousePos(), std::move(style), std::forward<Params>(args)...);
    }

    template<typename T, typename... Params>
    T* ImNodeFlow::placeNodeAt(const std::string& name, const ImVec2& pos, std::shared_ptr<NodeStyle> style, Params&&... args)
    {
        return addNode<T>(name, screen2content(pos), std::move(style), std::forward<Params>(args)...);
    }

    // -----------------------------------------------------------------------------------------------------------------
    // BASE NODE

    template<typename T>
    InPin<T>* BaseNode::addIN(const std::string& name, T defReturn, ConnectionFilter filter, std::shared_ptr<PinStyle> style)
    {
        return addIN_uid(name, name, defReturn, filter, std::move(style));
    }

    template<typename T, typename U>
    InPin<T>* BaseNode::addIN_uid(U uid, const std::string& name, T defReturn, ConnectionFilter filter, std::shared_ptr<PinStyle> style)
    {
        PinUID h = std::hash<U>{}(uid);
        m_ins.emplace_back(std::make_shared<InPin<T>>(h, name, filter, this, defReturn, m_inf, std::move(style)));
        return static_cast<InPin<T>*>(m_ins.back().get());
    }

    template<typename T>
    const T& BaseNode::showIN(const std::string& name, T defReturn, ConnectionFilter filter, std::shared_ptr<PinStyle> style)
    {
        return showIN_uid(name, name, defReturn, filter, std::move(style));
    }

    template<typename T, typename U>
    const T& BaseNode::showIN_uid(U uid, const std::string& name, T defReturn, ConnectionFilter filter, std::shared_ptr<PinStyle> style)
    {
        PinUID h = std::hash<U>{}(uid);
        for (std::pair<int, std::shared_ptr<Pin>>& p : m_dynamicIns)
        {
            if (p.second->getUid() == h)
            {
                p.first = 1;
                return static_cast<InPin<T>*>(p.second.get())->val();
            }
        }

        m_dynamicIns.emplace_back(std::make_pair(1, std::make_shared<InPin<T>>(h, name, filter, this, defReturn, m_inf, std::move(style))));
        return static_cast<InPin<T>*>(m_dynamicIns.back().second.get())->val();
    }

    template<typename T>
    OutPin<T>* BaseNode::addOUT(const std::string& name, ConnectionFilter filter, std::shared_ptr<PinStyle> style)
    {
        return addOUT_uid<T>(name, name, filter, std::move(style));
    }

    template<typename T, typename U>
    OutPin<T>* BaseNode::addOUT_uid(U uid, const std::string& name, ConnectionFilter filter, std::shared_ptr<PinStyle> style)
    {
        PinUID h = std::hash<U>{}(uid);
        m_outs.emplace_back(std::make_shared<OutPin<T>>(h, name, filter, this, m_inf, std::move(style)));
        return static_cast<OutPin<T>*>(m_outs.back().get());
    }

    template<typename T>
    void BaseNode::showOUT(const std::string& name, std::function<T()> behaviour, ConnectionFilter filter, std::shared_ptr<PinStyle> style)
    {
        showOUT_uid<T>(name, name, std::move(behaviour), filter, std::move(style));
    }

    template<typename T, typename U>
    void BaseNode::showOUT_uid(U uid, const std::string& name, std::function<T()> behaviour, ConnectionFilter filter, std::shared_ptr<PinStyle> style)
    {
        PinUID h = std::hash<U>{}(uid);
        for (std::pair<int, std::shared_ptr<Pin>>& p : m_dynamicOuts)
        {
            if (p.second->getUid() == h)
            {
                p.first = 2;
                return;
            }
        }

        m_dynamicOuts.emplace_back(std::make_pair(2, std::make_shared<OutPin<T>>(h, name, filter, this, m_inf, std::move(style))));
        static_cast<OutPin<T>*>(m_dynamicOuts.back().second.get())->behaviour(std::move(behaviour));
    }

    template<typename T, typename U>
    const T& BaseNode::getInVal(U uid)
    {
        PinUID h = std::hash<U>{}(uid);
        auto it = std::find_if(m_ins.begin(), m_ins.end(), [&h](std::shared_ptr<Pin>& p)
                            { return p->getUid() == h; });
        assert(it != m_ins.end() && "Pin UID not found!");
        return static_cast<InPin<T>*>(it->get())->val();
    }

    template<typename T>
    const T& BaseNode::getInVal(const char* uid)
    {
        PinUID h = std::hash<std::string>{}(std::string(uid));
        auto it = std::find_if(m_ins.begin(), m_ins.end(), [&h](std::shared_ptr<Pin>& p)
                            { return p->getUid() == h; });
        assert(it != m_ins.end() && "Pin UID not found!");
        return static_cast<InPin<T>*>(it->get())->val();
    }

    template<typename U>
    Pin* BaseNode::inPin(U uid)
    {
        PinUID h = std::hash<U>{}(uid);
        auto it = std::find_if(m_ins.begin(), m_ins.end(), [&h](std::shared_ptr<Pin>& p)
                            { return p->getUid() == h; });
        assert(it != m_ins.end() && "Pin UID not found!");
        return it->get();
    }

    inline Pin* BaseNode::inPin(const char* uid)
    {
        PinUID h = std::hash<std::string>{}(std::string(uid));
        auto it = std::find_if(m_ins.begin(), m_ins.end(), [&h](std::shared_ptr<Pin>& p)
                            { return p->getUid() == h; });
        assert(it != m_ins.end() && "Pin UID not found!");
        return it->get();
    }

    template<typename U>
    Pin* BaseNode::outPin(U uid)
    {
        PinUID h = std::hash<U>{}(uid);
        auto it = std::find_if(m_outs.begin(), m_outs.end(), [&h](std::shared_ptr<Pin>& p)
                            { return p->getUid() == h; });
        assert(it != m_outs.end() && "Pin UID not found!");
        return it->get();
    }

    inline Pin* BaseNode::outPin(const char* uid)
    {
        PinUID h = std::hash<std::string>{}(std::string(uid));
        auto it = std::find_if(m_outs.begin(), m_outs.end(), [&h](std::shared_ptr<Pin>& p)
                            { return p->getUid() == h; });
        assert(it != m_outs.end() && "Pin UID not found!");
        return it->get();
    }

    // -----------------------------------------------------------------------------------------------------------------
    // PIN

    inline void Pin::update()
    {
        // Custom rendering
        if (m_renderer)
        {
            ImGui::BeginGroup();
            m_renderer(this);
            ImGui::EndGroup();
            m_size = ImGui::GetItemRectSize();
            if (ImGui::IsItemHovered())
                m_inf->hovering(this);
            return;
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 tl = pinPoint() - ImVec2(m_style->socket_radius, m_style->socket_radius);
        ImVec2 br = pinPoint() + ImVec2(m_style->socket_radius, m_style->socket_radius);

        ImGui::SetCursorPos(m_pos);
        ImGui::Text(m_name.c_str());
        m_size = ImGui::GetItemRectSize();

        if (ImGui::IsItemHovered())
            draw_list->AddRectFilled(m_pos - m_style->extra.padding, m_pos + m_size + m_style->extra.padding, m_style->extra.bg_hover_color, m_style->extra.bg_radius);
        else
            draw_list->AddRectFilled(m_pos - m_style->extra.padding, m_pos + m_size + m_style->extra.padding, m_style->extra.bg_color, m_style->extra.bg_radius);
        draw_list->AddRect(m_pos - m_style->extra.padding, m_pos + m_size + m_style->extra.padding, m_style->extra.border_color, m_style->extra.bg_radius, 0, m_style->extra.border_thickness);

        if (isConnected())
            draw_list->AddCircleFilled(pinPoint(), m_style->socket_connected_radius, m_style->color, m_style->socket_shape);
        else
        {
            if (ImGui::IsItemHovered() || ImGui::IsMouseHoveringRect(tl, br))
                draw_list->AddCircle(pinPoint(), m_style->socket_hovered_radius, m_style->color, m_style->socket_shape, m_style->socket_thickness);
            else
                draw_list->AddCircle(pinPoint(), m_style->socket_radius, m_style->color, m_style->socket_shape, m_style->socket_thickness);
        }

        if (ImGui::IsItemHovered() || ImGui::IsMouseHoveringRect(tl, br))
            m_inf->hovering(this);
    }

    // -----------------------------------------------------------------------------------------------------------------
    // IN PIN

    template<class T>
    const T& InPin<T>::val()
    {
        if(!m_link)
            return m_emptyVal;

        return reinterpret_cast<OutPin<T>*>(m_link->left())->val();
    }

    template<class T>
    void InPin<T>::createLink(Pin *other)
    {
        if (other == this || other->getType() == PinType_Input || (m_parent == other->getParent() && (m_filter & ConnectionFilter_SameNode) == 0))
            return;

        if (!((m_filter & other->getFilter()) != 0 || m_filter == ConnectionFilter_None || other->getFilter() == ConnectionFilter_None)) // Check Filter
            return;

        if (m_link && m_link->left() == other)
        {
            m_link.reset();
            return;
        }

        m_link = std::make_shared<Link>(other, this, m_inf);
        other->setLink(m_link);
        m_inf->addLink(m_link);
    }

    // -----------------------------------------------------------------------------------------------------------------
    // OUT PIN

    template<class T>
    const T &OutPin<T>::val() { return m_val; }

    template<class T>
    void OutPin<T>::createLink(ImFlow::Pin *other)
    {
        if (other == this || other->getType() == PinType_Output)
            return;

        other->createLink(this);
    }

    template<class T>
    void OutPin<T>::setLink(std::shared_ptr<Link>& link)
    {
        m_links.emplace_back(link);
    }

    template<class T>
    void OutPin<T>::deleteLink()
    {
        m_links.erase(std::remove_if(m_links.begin(), m_links.end(),
                                     [](const std::weak_ptr<Link>& l) { return l.expired(); }), m_links.end());
    }
}
