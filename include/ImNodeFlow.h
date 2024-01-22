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
    class Pin; class BaseNode;
    class ImNodeFlow;

    // -----------------------------------------------------------------------------------------------------------------
    // LINK

    class Link
    {
    public:
        explicit Link(uintptr_t left, uintptr_t right) { m_left = left; m_right = right; }

        [[nodiscard]] uintptr_t left() const { return m_left; }
        [[nodiscard]] uintptr_t right() const { return m_right; }
    private:
        uintptr_t m_left;
        uintptr_t m_right;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // HANDLER

    class InfInterface
    {
    public:
        explicit InfInterface(ImNodeFlow &inf) : m_inf(inf) {}
        [[nodiscard]] bool dragAllowed() const; void dragAllowed(bool state);
        [[nodiscard]] bool isLinking() const; void isLinking(bool state);
        uintptr_t& pinTarget();
        Link* createLink(uintptr_t left, uintptr_t right);
        std::vector<Link>& links();
    private:
        ImNodeFlow& m_inf;
    };

    class ImNodeFlow
    {
    public:
        // static ImVec2 screen2canvas(const ImVec2&& pos) {return {};}
    public:
        friend class InfInterface;

        //ImNodeFlow() { m_infInterface = new InfInterface(*this); }
        explicit ImNodeFlow(const char* name) { m_name = name; m_infInterface = new InfInterface(*this); }

        void update();
        void resolve();

        template<typename T>
        void addNode(const char* name, const ImVec2&& pos)
        {
            static_assert(std::is_base_of<BaseNode, T>::value, "Pushed type is not subclass of BaseNode!");
            m_nodes.emplace_back(std::make_shared<T>(name, pos, m_infInterface));
        }
    private:
        InfInterface* m_infInterface;
        const char* m_name;
        ImVec2 m_scroll = ImVec2(0, 0);
        bool m_dragAllowed = true;
        bool m_isLinking = false;
        bool m_isLinkingNext = false;
        uintptr_t m_pinTarget = 0;

        std::vector<std::shared_ptr<BaseNode>> m_nodes;
        std::vector<Link> m_links;
    };

    inline bool InfInterface::dragAllowed() const { return m_inf.m_dragAllowed; }
    inline void InfInterface::dragAllowed(bool state) { m_inf.m_dragAllowed = state; }
    inline bool InfInterface::isLinking() const { return m_inf.m_isLinking; }
    inline void InfInterface::isLinking(bool state) { m_inf.m_isLinkingNext = state; }
    inline uintptr_t& InfInterface::pinTarget() { return m_inf.m_pinTarget; }
    inline Link* InfInterface::createLink(uintptr_t left, uintptr_t right)
    {
        m_inf.m_links.emplace_back(left, right);
        return &m_inf.m_links.back();
    }
    inline std::vector<Link>& InfInterface::links() { return m_inf.m_links; }

    // -----------------------------------------------------------------------------------------------------------------
    // BASE NODE

    class BaseNode
    {
    public:
        friend class ImNodeFlow;

        explicit BaseNode(const char* name, ImVec2 pos, InfInterface* inf) :m_inf(inf) { m_name = name, m_pos = pos; }

        virtual void draw() = 0;
        virtual void resolve(uintptr_t me) {}

        template<typename T>
        void addIN(const char* name, BaseNode* parent)
        {
            m_ins.emplace_back(std::make_shared<InPin<T>>(name, parent, m_inf));
        }

        template<typename T>
        void addOUT(const char* name, BaseNode* parent)
        {
            m_outs.emplace_back(std::make_shared<OutPin<T>>(name, parent, m_inf));
        }

        template<typename T>
        InPin<T>& ins(int i) { return *std::dynamic_pointer_cast<InPin<T>>(m_ins[i]); }
        Pin& ins(int i) { return *m_ins[i]; }
        template<typename T>
        OutPin<T>& outs(int i) { return *std::dynamic_pointer_cast<OutPin<T>>(m_outs[i]); }
        Pin& outs(int i) { return *m_outs[i]; }

        ImVec2& padding() { return m_padding; }
    private:
        void update(ImVec2& offset);
    private:
        InfInterface* m_inf;
        ImVec2 m_pos;
        ImVec2 m_size;
        std::string m_name;
        bool m_dragged = false;
        bool m_dragDeny = false;
        ImVec2 m_padding = ImVec2(5.f, 5.f);

        std::vector<std::shared_ptr<Pin>> m_ins;
        std::vector<std::shared_ptr<Pin>> m_outs;

    };

    // -----------------------------------------------------------------------------------------------------------------
    // PINS

    class Pin
    {
    public:
        explicit Pin(const char* name, BaseNode* parent, InfInterface* inf) :m_inf(inf) { m_name = name; m_parent = parent; }
        void pos(ImVec2 pos) { m_pos = pos; }
        [[nodiscard]] const ImVec2& pos() { return m_pos; }
        [[nodiscard]] const ImVec2& size() { return m_size; }
        virtual uintptr_t me() = 0;
        BaseNode* parent() { return m_parent; }

        virtual void update() = 0;
    protected:
        InfInterface* m_inf;
        ImVec2 m_pos = ImVec2(0, 0);
        ImVec2 m_size = ImVec2(10, 10);
        std::string m_name;
        BaseNode* m_parent = nullptr;
        bool m_dragging = false;
    };


    template<class T> class InPin : public Pin
    {
    public:
        explicit InPin(const char* name, BaseNode* parent, InfInterface* inf) : Pin(name, parent, inf) {}

        void setLink(Link* link) { m_link = link; }

        uintptr_t me() override { return m_me; }
        const T& val();

        void update() override;
    private:
        uintptr_t m_me = reinterpret_cast<uintptr_t>(this);
        Link* m_link = nullptr;
        const T defaultVal = 0;
    };


    template<class T> class OutPin : public Pin
    {
    public:
        explicit OutPin(const char* name, BaseNode* parent, InfInterface* inf) : Pin(name, parent, inf) {}

        uintptr_t me() override { return m_me; }
        const T& val();
        OutPin& operator<<(const T&& val) { m_val = val; return *this; }

        void update() override;
    private:
        uintptr_t m_me = reinterpret_cast<uintptr_t>(this);
        T m_val;
    };


    template<class T>
    const T &InPin<T>::val()
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
                            m_link = nullptr;
                            return;
                        }
                        if(l.right() == me())
                        {
                            m_inf->links().erase(m_inf->links().begin() + i);
                            m_link = nullptr;
                            break;
                        }
                        i++;
                    }
                }
                setLink(m_inf->createLink(m_inf->pinTarget(), me()));
            }
        }
    }


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
                draw_list->AddBezierCubic(pinDot, pinDot + ImVec2(+50, 0), ImGui::GetMousePos() + ImVec2(-50, 0), ImGui::GetMousePos(), IM_COL32(200, 200, 100, 255), 3.0f);
        }
    }
}
