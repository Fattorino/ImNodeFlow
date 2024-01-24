#ifndef IM_NODE_FLOW
#define IM_NODE_FLOW
#pragma once

#include <string>
#include <utility>
#include <vector>
#include <cmath>
#include <memory>
#include <imgui.h>
#include "../src/imgui_bezier_math.h"

namespace ImFlow
{
    template<typename T> class InPin;
    template<typename T> class OutPin;
    class Pin; class BaseNode;

    // -----------------------------------------------------------------------------------------------------------------
    // LINK

    class Link
    {
    public:
        explicit Link(uintptr_t left, uintptr_t right) :m_left(left), m_right(right) {}

        [[nodiscard]] uintptr_t left() const { return m_left; }
        [[nodiscard]] uintptr_t right() const { return m_right; }
        [[nodiscard]] bool selected() const { return m_selected; }

        void selected(bool state) { m_selected = state; }
    private:
        uintptr_t m_left;
        uintptr_t m_right;
        bool m_selected = false;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // HANDLER

    class ImNodeFlow
    {
    public:
        friend class InfInterface;

        //ImNodeFlow() { m_infInterface = new InfInterface(*this); }
        explicit ImNodeFlow(std::string name) :m_name(std::move(name)) {}

        void update();
        void resolve();

        template<typename T>
        void addNode(const std::string& name, const ImVec2&& pos);

        void createLink(uintptr_t left, uintptr_t right);
        std::vector<std::shared_ptr<Link>>& links() { return m_links; }

        [[nodiscard]] bool dragAllowed() const { return m_dragAllowed; }
        [[nodiscard]] bool isLinking() const { return m_isLinking; }

        void dragAllowed(bool state) { m_dragAllowedNext = state; }
        void isLinking(bool state) { m_isLinkingNext = state; }

        uintptr_t& pinTarget() { return m_pinTarget; }
    private:
        std::string m_name;
        ImVec2 m_scroll = ImVec2(0, 0);
        bool m_dragAllowed = true;
        bool m_dragAllowedNext = true;
        bool m_isLinking = false;
        bool m_isLinkingNext = false;
        uintptr_t m_pinTarget = 0;

        std::vector<std::shared_ptr<BaseNode>> m_nodes;
        std::vector<std::shared_ptr<Link>> m_links;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // BASE NODE

    class BaseNode
    {
    public:
        friend class ImNodeFlow;
        friend class Link;

        explicit BaseNode(std::string name, ImVec2 pos, ImNodeFlow* inf) :m_name(std::move(name)), m_pos(pos), m_inf(inf) {}

        virtual void draw() = 0;
        virtual void resolve(uintptr_t me) {}

        template<typename T>
        void addIN(const char* name, BaseNode* parent);
        template<typename T>
        void addOUT(const char* name, BaseNode* parent);

        Pin& ins(int i) { return *m_ins[i]; }
        Pin& outs(int i) { return *m_outs[i]; }
        template<typename T>
        InPin<T>& ins(int i);
        template<typename T>
        OutPin<T>& outs(int i);

        const std::string& name() { return m_name; }
        ImVec2& padding() { return m_padding; }

    private:
        void update(ImVec2& offset);
    private:
        ImNodeFlow* m_inf;
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
        explicit Pin(std::string name, BaseNode* parent, ImNodeFlow* inf) :m_name(std::move(name)), m_parent(parent), m_inf(inf) {}

        virtual uintptr_t me() = 0;
        virtual void update() = 0;
        virtual void setLink(std::shared_ptr<Link>& link) {}

        [[nodiscard]] const ImVec2& pos() { return m_pos; }
        [[nodiscard]] const ImVec2& size() { return m_size; }
        const std::string& name() { return m_name; }
        BaseNode* parent() { return m_parent; }

        void pos(ImVec2 pos) { m_pos = pos; }
    protected:
        ImNodeFlow* m_inf;
        ImVec2 m_pos = ImVec2(0, 0);
        ImVec2 m_size = ImVec2(10, 10);
        std::string m_name;
        BaseNode* m_parent = nullptr;
        bool m_dragging = false;
    };


    template<class T> class InPin : public Pin
    {
    public:
        explicit InPin(const std::string& name, BaseNode* parent, ImNodeFlow* inf) : Pin(name, parent, inf) {}

        void update() override;
        uintptr_t me() override { return m_me; }

        const T& val();

        void setLink(std::shared_ptr<Link>& link) override { m_link = link; }
    private:
        uintptr_t m_me = reinterpret_cast<uintptr_t>(this);
        std::weak_ptr<Link> m_link;
        const T defaultVal = 0;
    };


    template<class T> class OutPin : public Pin
    {
    public:
        explicit OutPin(const std::string& name, BaseNode* parent, ImNodeFlow* inf) : Pin(name, parent, inf) {}

        void update() override;
        uintptr_t me() override { return m_me; }

        const T& val();

        OutPin& operator<<(const T&& val) { m_val = val; return *this; }
    private:
        uintptr_t m_me = reinterpret_cast<uintptr_t>(this);
        T m_val;
    };
}

#include "../src/ImNodeFlow.inl"

#endif
