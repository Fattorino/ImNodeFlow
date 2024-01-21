#pragma once

#include <string>
#include <utility>
#include <vector>
#include <cmath>
#include <memory>
#include <imgui.h>

namespace ImFlow
{
    template<typename T> class InPin;
    template<typename T> class OutPin;
    class Pin;

    // -----------------------------------------------------------------------------------------------------------------
    // BASE NODE

    class BaseNode
    {
    public:
        friend class ImNodeFlow;

        explicit BaseNode(const char* name, ImVec2 pos) { m_name = name, m_pos = pos; }

        virtual void draw() = 0;
        virtual void resolve(uintptr_t me) {}

        template<typename T>
        void addIN(const char* name, BaseNode* parent)
        {
            m_ins.emplace_back(std::make_shared<InPin<T>>(name, parent));
        }

        template<typename T>
        void addOUT(const char* name, BaseNode* parent)
        {
            m_outs.emplace_back(std::make_shared<OutPin<T>>(name, parent));
        }

        template<typename T>
        InPin<T>& ins(int i) { return *std::dynamic_pointer_cast<InPin<T>>(m_ins[i]); }
        Pin& ins(int i) { return *m_ins[i]; }
        template<typename T>
        OutPin<T>& outs(int i) { return *std::dynamic_pointer_cast<OutPin<T>>(m_outs[i]); }
        Pin& outs(int i) { return *m_outs[i]; }

        ImVec2& padding() { return m_padding; }
    private:
        ImVec2 m_pos;
        ImVec2 m_size;
        std::string m_name;
        bool m_dragged = false;
        bool m_dragDeny = false;
        ImVec2 m_padding = ImVec2(5.f, 5.f);

        std::vector<std::shared_ptr<Pin>> m_ins;
        std::vector<std::shared_ptr<Pin>> m_outs;

        void update(ImVec2 offset, int i);
    };

    // -----------------------------------------------------------------------------------------------------------------
    // LINK

    template<typename T> class Link
    {
    public:
        explicit Link(OutPin<T>* left) { m_left = left; }

        const T& val() { return m_left->val(); }
    private:
        OutPin<T>* m_left;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // PINS

    class Pin
    {
    public:
        explicit Pin(const char* name, BaseNode* parent) { m_name = name; m_parent = parent; }
        void setPos(ImVec2 pos) { m_pos = pos; }
        virtual uintptr_t me() = 0;
        virtual void update() = 0;
    protected:
        ImVec2 m_pos = ImVec2(0, 0);
        std::string m_name;
        BaseNode* m_parent = nullptr;
        bool m_dragging = false;
    };


    template<class T> class InPin : public Pin
    {
    public:
        explicit InPin(const char* name, BaseNode* parent) : Pin(name, parent) {}

        void setLink(Link<T>* link) { m_link = link; }

        uintptr_t me() override { return m_me; }
        const T& val();

        void update() override;
    private:
        uintptr_t m_me = reinterpret_cast<uintptr_t>(this);
        Link<T>* m_link = nullptr;
        const T defaultVal = 0;
    };

    template<class T>
    const T &InPin<T>::val() { if(m_link) return m_link->val(); return defaultVal; }


    template<class T> class OutPin : public Pin
    {
    public:
        explicit OutPin(const char* name, BaseNode* parent) : Pin(name, parent) {}

        uintptr_t me() override { return m_me; }
        const T& val();
        OutPin& operator<<(const T&& val) { m_val = val; return *this; }

        void update() override;
    private:
        uintptr_t m_me = reinterpret_cast<uintptr_t>(this);
        T m_val;
    };

    template<class T>
    const T &OutPin<T>::val() { m_parent->resolve(m_me); return m_val; }

    // -----------------------------------------------------------------------------------------------------------------

    class ImNodeFlow
    {
    public:
        static ImVec2 screen2canvas(const ImVec2&& pos) {return {};}
        static ImVec2 INF_scroll;
        static bool INF_dragAllowed;
        static bool INF_linking;
        static std::shared_ptr<Pin> IND_pinTarget;
    public:
        ImNodeFlow() = default;
        explicit ImNodeFlow(const std::string&& name) { m_name = name; }

        void update();
        void resolve();

        template<typename T>
        void pushNode(const char* name, const ImVec2&& pos)
        {
            static_assert(std::is_base_of<BaseNode, T>::value, "Pushed type is not subclass of BaseNode!");
            m_nodes.emplace_back(std::make_shared<T>(name, pos));
        }
    private:
        std::string m_name = "FlowGrid";
        std::vector<std::shared_ptr<BaseNode>> m_nodes;
    };

    // -----------------------------------------------------------------------------------------------------------------

    template<class T>
    void InPin<T>::update()
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImGui::Text(m_name.c_str());
        if (ImGui::IsItemHovered())
            draw_list->AddRectFilled(m_pos - ImVec2(3,1), m_pos + ImGui::GetItemRectSize() + ImVec2(3,2), IM_COL32(100, 100, 255, 70));
        ImVec2 pinDot = m_pos + ImVec2(ImGui::GetItemRectSize().x + m_parent->padding().x, ImGui::GetItemRectSize().y / 2);
        draw_list->AddRect(m_pos - ImVec2(3,1), m_pos + ImGui::GetItemRectSize() + ImVec2(3,2), IM_COL32(255, 255, 255, 100));
        draw_list->AddCircleFilled(pinDot, 5, IM_COL32(50, 40, 40, 255));

        if(ImNodeFlow::INF_linking && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            // TODO: Create link using ImNodeFlow::IND_pinTarget as "m_left" in linking constructor
        }
    }


    template<class T>
    void OutPin<T>::update()
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImGui::Text(m_name.c_str());
        if (ImGui::IsItemHovered())
            draw_list->AddRectFilled(m_pos - ImVec2(3,1), m_pos + ImGui::GetItemRectSize() + ImVec2(3,2), IM_COL32(100, 100, 255, 70));
        ImVec2 pinDot = m_pos + ImVec2(ImGui::GetItemRectSize().x + m_parent->padding().x, ImGui::GetItemRectSize().y / 2);
        draw_list->AddRect(m_pos - ImVec2(3,1), m_pos + ImGui::GetItemRectSize() + ImVec2(3,2), IM_COL32(255, 255, 255, 100));
        draw_list->AddCircleFilled(pinDot, 5, IM_COL32(50, 40, 40, 255));

        if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImNodeFlow::INF_dragAllowed)
        {
            m_dragging = true;
            ImNodeFlow::INF_dragAllowed = false;
            ImNodeFlow::INF_linking = true;
            ImNodeFlow::IND_pinTarget = this;
        }
        if (m_dragging)
        {
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                m_dragging = false;
                ImNodeFlow::INF_dragAllowed = true;
                ImNodeFlow::INF_linking = false;
            }
            if(!ImGui::IsItemHovered())
                draw_list->AddBezierCubic(pinDot, pinDot + ImVec2(+50, 0), ImGui::GetMousePos() + ImVec2(-50, 0), ImGui::GetMousePos(), IM_COL32(200, 200, 100, 255), 3.0f);
        }
    }
}
