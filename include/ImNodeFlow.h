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

        explicit BaseNode(ImVec2 pos) { m_pos = pos; }

        virtual void draw() = 0;
        virtual void resolve(uintptr_t me) {}

        template<typename T>
        void addIN(const char* name)
        {
            m_ins.emplace_back(std::make_shared<InPin<T>>(name));
        }

        template<typename T>
        void addOUT(const char* name, BaseNode* parent)
        {
            m_outs.emplace_back(std::make_shared<OutPin<T>>(name, parent));
        }

        template<typename T>
        std::shared_ptr<InPin<T>> ins(int i) { return std::dynamic_pointer_cast<InPin<T>>(m_ins[i]); }
        auto outs(int i) { return m_outs[i]; }
    private:
        ImVec2 m_pos;
        ImVec2 m_size;
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
        explicit Pin(const char*  name) { m_name = name; }
        virtual uintptr_t me() = 0;
        virtual void draw() = 0;
    protected:
        std::string m_name;
    };

    template<class T> class InPin : public Pin
    {
    public:
        explicit InPin(const char* name) : Pin(name) {}

        void setLink(Link<T>* link) { m_link = link; }

        uintptr_t me() override { return m_me; }
        const T& val();

        void draw() override;
    private:
        uintptr_t m_me = reinterpret_cast<uintptr_t>(this);
        Link<T>* m_link = nullptr;
        const T defaultVal = 0;
    };

    template<class T>
    const T &InPin<T>::val() { if(m_link) return m_link->val(); return defaultVal; }

    template<class T>
    void InPin<T>::draw()
    {
        ImGui::Text(m_name.c_str());
    }

    template<class T> class OutPin : public Pin
    {
    public:
        explicit OutPin(const char* name, BaseNode* parent) : Pin(name) { m_parent = parent; }

        uintptr_t me() override { return m_me; }
        const T& val();
        OutPin& operator<<(const T&& val) { m_val = val; return *this; }

        void draw() override;
    private:
        uintptr_t m_me = reinterpret_cast<uintptr_t>(this);
        BaseNode* m_parent = nullptr;
        T m_val;
    };

    template<class T>
    const T &OutPin<T>::val() { m_parent->resolve(m_me); return m_val; }

    template<class T>
    void OutPin<T>::draw()
    {
        ImGui::Text(m_name.c_str());
    }

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
