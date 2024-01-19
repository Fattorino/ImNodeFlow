#pragma once

#include <string>
#include <utility>
#include <vector>
#include <cmath>
#include <memory>
#include <imgui.h>

namespace ImFlow
{
    template<typename T> class Link;
    class BaseNode;

    class Pin
    {
    public:
        explicit Pin(std::string name) { m_name = std::move(name); }
        virtual void draw() = 0;
    protected:
        std::string m_name;
    };

    template<typename T> class InPin : public Pin
    {
    public:
        explicit InPin(std::string name) : Pin(name) {}

        void setLink(Link<T>* link) { m_link = link; }
        const T& val();

        void draw() override { ImGui::Text(m_name.c_str()); }
    private:
        Link<T>* m_link = nullptr;
        const T defaultVal = 0;
    };

    template<typename T> class OutPin : public Pin
    {
    public:
        explicit OutPin(std::string name, BaseNode* parent) : Pin(name) {m_parent = parent;}

        uintptr_t me() { return reinterpret_cast<uintptr_t>(this); }
        const T& val();
        OutPin& operator<<(const T&& val) { m_val = val; return *this; }

        void draw() override { ImGui::Text(m_name.c_str()); }
    private:
        uintptr_t m_me = reinterpret_cast<uintptr_t>(this);
        BaseNode* m_parent;
        T m_val;
    };

    template<typename T> class Link
    {
    public:
        explicit Link(OutPin<T>* left) { m_left = left; }

        const T& val() { return m_left->val(); }
    private:
        OutPin<T>* m_left;
    };

    class BaseNode
    {
    public:
        friend class ImNodeFlow;

        explicit BaseNode(ImVec2 pos) { m_pos = pos; }
        virtual void draw() = 0;
        virtual void resolve(uintptr_t me) {}
    private:
        ImVec2 m_pos;
        ImVec2 m_size;
        bool m_dragged = false;
        ImVec2 m_padding = ImVec2(5.f, 5.f);

        void update(ImVec2 offset);
    };

    template<typename T>
    const T &InPin<T>::val() { if(m_link) return m_link->val(); else return defaultVal; }

    template<typename T>
    const T &OutPin<T>::val() { m_parent->resolve(m_me); return m_val; }

    // -----------------------------------------------------------------------------------------------------------------

    class ImNodeFlow
    {
    public:
        static ImVec2 screen2canvas(const ImVec2&& pos) {return {};}
    public:
        ImNodeFlow() = default;
        explicit ImNodeFlow(const std::string&& name) { m_name = name; }

        void update();
        void resolve();

        template<typename T>
        void pushNode(const ImVec2&& pos)
        {
            static_assert(std::is_base_of<BaseNode, T>::value, "Pushed type is not subclass of BaseNode!");
            m_nodes.emplace_back(std::make_shared<T>(pos));
        }
    private:
        std::string m_name = "FlowGrid";
        ImVec2 m_scroll = ImVec2(0.0f, 0.0f);
        ImVec2 m_offset = ImVec2(0.0f, 0.0f);
        std::vector<std::shared_ptr<BaseNode>> m_nodes;
    };
}
