#ifndef IM_NODE_FLOW
#define IM_NODE_FLOW
#pragma once

#include <string>
#include <utility>
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>
#include <functional>
#include <imgui.h>
#include "../src/imgui_bezier_math.h"

namespace ImFlow
{

    typedef void (*VoidCallback)();

    inline void smart_bezier(const ImVec2& p1, const ImVec2& p2, ImU32 color, float thickness);
    inline bool smart_bezier_collider(const ImVec2& p, const ImVec2& p1, const ImVec2& p2, float radius);

    template<typename T> class InPin;
    template<typename T> class OutPin;
    class Pin; class BaseNode;
    class ImNodeFlow;

    enum PinKind
    {
        PinKind_Input,
        PinKind_Output
    };

    typedef int ConnectionFilter;
    enum ConnectionFilter_
    {
        ConnectionFilter_None   = 0,
        ConnectionFilter_Int    = 1 << 1,
        ConnectionFilter_Float  = 1 << 2,
        ConnectionFilter_Double = 1 << 3,
        ConnectionFilter_String = 1 << 4,
        ConnectionFilter_MakeCustom = 1 << 5,
        ConnectionFilter_Numbers = ConnectionFilter_Int | ConnectionFilter_Float | ConnectionFilter_Double
    };

    // -----------------------------------------------------------------------------------------------------------------
    // LINK

    class Link
    {
    public:
        explicit Link(Pin* left, Pin* right, ImNodeFlow* inf) :m_left(left), m_right(right), m_inf(inf) {}

        void update();

        [[nodiscard]] Pin* left() const { return m_left; }
        [[nodiscard]] Pin* right() const { return m_right; }

        [[nodiscard]] bool hovered() const { return m_hovered; }
        [[nodiscard]] bool selected() const { return m_selected; }

        void selected(bool state) { m_selected = state; }
    private:
        ImNodeFlow* m_inf;
        Pin* m_left;
        Pin* m_right;
        bool m_hovered = false;
        bool m_selected = false;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // HANDLER

    struct InfColors
    {
        ImU32 pin_bg = IM_COL32(23, 16, 16, 0);
        ImU32 pin_hovered = IM_COL32(100, 100, 255, 70);
        ImU32 pin_border = IM_COL32(255, 255, 255, 0);
        ImU32 pin_point = IM_COL32(255, 255, 240, 230);

        ImU32 link = IM_COL32(230, 230, 200, 230);
        ImU32 drag_out_link = IM_COL32(230, 230, 200, 230);
        ImU32 link_selected_outline = IM_COL32(80, 20, 255, 200);

        ImU32 node_bg = IM_COL32(97, 103, 122, 100);
        ImU32 node_header = IM_COL32(23, 16, 16, 150);
        ImColor node_header_title = ImColor(255, 246, 240, 255);
        ImU32 node_border = IM_COL32(100, 100, 100, 255);
        ImU32 node_selected_border = IM_COL32(170, 190, 205, 230);

        ImU32 background = IM_COL32(44, 51, 51, 255);
        ImU32 grid = IM_COL32(200, 200, 200, 40);
        ImU32 subGrid = IM_COL32(200, 200, 200, 10);
    };

    struct InfStyler
    {
        ImVec2 pin_padding = ImVec2(3.f, 1.f);
        float pin_radius = 8.f;
        float pin_border_thickness = 1.f;
        float pin_point_radius = 3.5f;

        float link_thickness = 2.6f;
        float link_hovered_thickness = 3.5f;
        float link_selected_outline_thickness = 0.5f;
        float drag_out_link_thickness = 2.f;

        ImVec4 node_padding = ImVec4(9.f, 6.f, 9.f, 2.f); // Left Top Right Bottom
        float node_radius = 8.f;
        float node_border_thickness = 1.f;
        float node_border_selected_thickness = 2.f;

        float grid_size = 50.f;
        float grid_subdivisions = 5.f;

        InfColors colors;
    };

    class ImNodeFlow
    {
    private:
        static int m_instances;
    public:
        ImNodeFlow() { m_name = "FlowGrid" + std::to_string(m_instances); m_instances++; }
        explicit ImNodeFlow(std::string name) :m_name(std::move(name)) { m_instances++; }
        explicit ImNodeFlow(const char* name) :m_name(name) { m_instances++; }

        void update();

        template<typename T>
        void addNode(const std::string& name, const ImVec2& pos);
        template<typename T>
        T* dropNode(const std::string& name, const ImVec2& pos);
        int nodesCount() { return m_nodes.size(); }

        void createLink(Pin* left, Pin* right);

        void droppedLinkPopUpContent(std::function<void(Pin* dragged)> content, ImGuiKey key = ImGuiKey_None) { m_droppedLinkPopUp = std::move(content); m_droppedLinkPupUpComboKey = key; }
        void rightClickPopUpContent(std::function<void()> content) { m_rightClickPopUp = std::move(content); }

        [[nodiscard]] bool draggingNode() const { return m_draggingNode; }
        InfStyler& style() { return m_style; }

        void draggingNode(bool state) { m_draggingNodeNext = state; }
        void hovering(Pin* hovering) { m_hovering = hovering; }

        ImVec2 canvas2screen(const ImVec2& p);
        ImVec2 screen2canvas(const ImVec2& p);
    private:
        bool on_selected_node();
        bool on_free_space();
    private:
        std::string m_name;
        ImVec2 m_pos;
        ImVec2 m_scroll = ImVec2(0, 0);

        std::vector<std::shared_ptr<BaseNode>> m_nodes;
        std::vector<std::weak_ptr<Link>> m_links;

        std::function<void()> m_rightClickPopUp;
        std::function<void(Pin* dragged)> m_droppedLinkPopUp;
        ImGuiKey m_droppedLinkPupUpComboKey;
        Pin* m_droppedLinkLeft = nullptr;

        bool m_draggingNode = false, m_draggingNodeNext = false;
        Pin* m_hovering = nullptr;
        Pin* m_dragOut = nullptr;

        InfStyler m_style;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // BASE NODE

    class BaseNode
    {
    public:
        explicit BaseNode(std::string name, ImVec2 pos, ImNodeFlow* inf)
            :m_name(std::move(name)), m_pos(pos), m_inf(inf)
        {
            m_paddingTL = {m_inf->style().node_padding.x, m_inf->style().node_padding.y};
            m_paddingBR = {m_inf->style().node_padding.z, m_inf->style().node_padding.w};
        }

        void update(ImVec2& offset);
        virtual void draw() = 0;

        template<typename T>
        void addIN(std::string name, T defReturn, ConnectionFilter filter = ConnectionFilter_None);
        template<typename T>
        [[nodiscard]] OutPin<T>* addOUT(std::string name, ConnectionFilter filter = ConnectionFilter_None);

        template<typename T>
        const T& ins(int i);
        Pin* ins(int i) { return m_ins[i].get(); }
        Pin* outs(int i) { return m_outs[i].get(); }

        bool hovered();
        void selected(bool state) { m_selected = state; }

        const std::string& name() { return m_name; }
        const ImVec2& size() { return  m_size; }
        const ImVec2& pos() { return  m_pos; }
        [[nodiscard]] bool selected() const { return m_selected; }
    private:
        std::string m_name;
        ImVec2 m_pos, m_posOld = m_pos;
        ImVec2 m_size;
        ImNodeFlow* m_inf;
        bool m_selected = false;
        bool m_dragged = false;
        ImVec2 m_paddingTL;
        ImVec2 m_paddingBR;

        std::vector<std::shared_ptr<Pin>> m_ins;
        std::vector<std::shared_ptr<Pin>> m_outs;
    };

    // -----------------------------------------------------------------------------------------------------------------
    // PINS

    class Pin
    {
    public:
        explicit Pin(std::string name, ConnectionFilter filter, PinKind kind, BaseNode* parent, ImNodeFlow* inf)
            :m_name(std::move(name)), m_filter(filter), m_kind(kind), m_parent(parent), m_inf(inf) {}

        virtual void update() = 0;

        virtual void createLink(Pin* left) {}
        virtual void setLink(std::shared_ptr<Link>& link) {}
        virtual void deleteLink() {}
        virtual std::weak_ptr<Link> getLink() { return std::weak_ptr<Link>{}; }

        const std::string& name() { return m_name; }
        [[nodiscard]] const ImVec2& pos() { return m_pos; }
        [[nodiscard]] const ImVec2& size() { return m_size; }
        virtual ImVec2 pinPoint() = 0;
        float calcWidth() { return ImGui::CalcTextSize(m_name.c_str()).x; }
        BaseNode* parent() { return m_parent; }
        PinKind kind() { return m_kind; }
        [[nodiscard]] ConnectionFilter filter() const { return m_filter; }

        void pos(ImVec2 pos) { m_pos = pos; }
    protected:
        std::string m_name;
        ImVec2 m_pos = ImVec2(0, 0);
        ImVec2 m_size = ImVec2(10, 10);
        PinKind m_kind;
        ConnectionFilter m_filter;
        BaseNode* m_parent = nullptr;
        ImNodeFlow* m_inf;
    };


    template<class T> class InPin : public Pin
    {
    public:
        explicit InPin(const std::string& name, ConnectionFilter filter, PinKind kind, BaseNode* parent, T defReturn, ImNodeFlow* inf)
            :Pin(name, filter, kind, parent, inf), m_emptyVal(defReturn) {}

        void update() override;

        const T& val();

        void createLink(Pin* left) override;
        void deleteLink() override { m_link.reset(); }
        std::weak_ptr<Link> getLink() override { return m_link; }

        ImVec2 pinPoint() override { return m_pos + ImVec2(-m_inf->style().node_padding.z, m_size.y / 2); }
    private:
        std::shared_ptr<Link> m_link;
        T m_emptyVal;
    };


    template<class T> class OutPin : public Pin
    {
    public:
        explicit OutPin(const std::string& name, ConnectionFilter filter, PinKind kind, BaseNode* parent, ImNodeFlow* inf)
            :Pin(name, filter, kind, parent, inf) {}

        ~OutPin() { if (!m_link.expired()) m_link.lock()->right()->deleteLink(); }

        void update() override;

        void setLink(std::shared_ptr<Link>& link) override { m_link = link; }

        const T& val();
        void behaviour(std::function<T()> func) { m_behaviour = std::move(func); }

        ImVec2 pinPoint() override { return m_pos + ImVec2(m_size.x + m_inf->style().node_padding.z, m_size.y / 2); }
    private:
        std::weak_ptr<Link> m_link;
        std::function<T()> m_behaviour;
        T m_val;
    };
}

#include "../src/ImNodeFlow.inl"

#endif
