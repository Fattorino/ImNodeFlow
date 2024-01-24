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

typedef void (*VoidCallback)();


namespace ImFlow
{
    inline void smart_bezier(const ImVec2& p1, const ImVec2& p2, ImU32 color, float thickness);
    inline bool smart_bezier_collider(const ImVec2& p1, const ImVec2& p2);

    template<typename T> class InPin;
    template<typename T> class OutPin;
    class Pin; class BaseNode;

    // -----------------------------------------------------------------------------------------------------------------
    // LINK

    class Link
    {
    public:
        explicit Link(uintptr_t left, uintptr_t right) :m_left(left), m_right(right) {}

        void draw();

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
        //ImNodeFlow() { m_infInterface = new InfInterface(*this); }
        explicit ImNodeFlow(std::string name) :m_name(std::move(name)) {}

        void update();
        void resolve();

        template<typename T>
        void addNode(const std::string& name, const ImVec2&& pos);

        void createLink(uintptr_t left, uintptr_t right);
        std::vector<std::shared_ptr<Link>>& links() { return m_links; }

        void setDroppedLinkCallback(VoidCallback callback) { m_droppedLinkCallback = callback; }

        void draggingNode(bool state) { m_draggingNode = state; }
        void hovering(Pin* hovering) { m_hovering = hovering; }
    private:
        std::string m_name;
        ImVec2 m_scroll = ImVec2(0, 0);

        std::vector<std::shared_ptr<BaseNode>> m_nodes;
        std::vector<std::shared_ptr<Link>> m_links;
        VoidCallback m_droppedLinkCallback;

        bool m_draggingNode = false;
        Pin* m_hovering = nullptr;
        Pin* m_dragOut = nullptr;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // BASE NODE

    class BaseNode
    {
    public:
        friend class ImNodeFlow;
        friend class Link;

        explicit BaseNode(std::string name, ImVec2 pos, ImNodeFlow* inf)
            :m_name(std::move(name)), m_pos(pos), m_inf(inf) {}

        virtual void draw() = 0;
        virtual void resolve(uintptr_t me) {}

        template<typename T>
        void addIN(std::string name, T defReturn);
        template<typename T>
        void addOUT(std::string name);

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
        ImVec2 m_padding = ImVec2(5.f, 5.f);

        std::vector<std::shared_ptr<Pin>> m_ins;
        std::vector<std::shared_ptr<Pin>> m_outs;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // PINS

    enum PinKind
    {
        PinKind_Input,
        PinKind_Output
    };

    class Pin
    {
    public:
        explicit Pin(std::string name, PinKind kind, BaseNode* parent, ImNodeFlow* inf)
            :m_name(std::move(name)), m_kind(kind), m_parent(parent), m_inf(inf) {}

        virtual uintptr_t me() = 0;
        virtual void update() = 0;
        virtual void draw() = 0;
        virtual void setLink(std::shared_ptr<Link>& link) {}
        virtual std::weak_ptr<Link> getLink() { return std::weak_ptr<Link>{}; }

        [[nodiscard]] const ImVec2& pos() { return m_pos; }
        [[nodiscard]] const ImVec2& size() { return m_size; }
        const std::string& name() { return m_name; }
        BaseNode* parent() { return m_parent; }
        PinKind kind() { return m_kind; }

        void pos(ImVec2 pos) { m_pos = pos; }
    protected:
        std::string m_name;
        ImVec2 m_pos = ImVec2(0, 0);
        ImVec2 m_size = ImVec2(10, 10);
        PinKind m_kind;
        BaseNode* m_parent = nullptr;
        ImNodeFlow* m_inf;
        bool m_dragging = false;
    };


    template<class T> class InPin : public Pin
    {
    public:
        explicit InPin(const std::string& name, PinKind kind, BaseNode* parent, T defReturn, ImNodeFlow* inf)
            :Pin(name, kind, parent, inf), m_emptyVal(defReturn) {}

        void update() override;
        void draw() override;
        uintptr_t me() override { return m_me; }

        const T& val();

        void setLink(std::shared_ptr<Link>& link) override { m_link = link; }
        std::weak_ptr<Link> getLink() override { return m_link; }
    private:
        uintptr_t m_me = reinterpret_cast<uintptr_t>(this);
        std::weak_ptr<Link> m_link;
        T m_emptyVal;
    };


    template<class T> class OutPin : public Pin
    {
    public:
        explicit OutPin(const std::string& name, PinKind kind, BaseNode* parent, ImNodeFlow* inf)
            :Pin(name, kind, parent, inf) {}

        void update() override;
        void draw() override;
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
