#pragma once

#include <imgui.h>
#include <imgui_internal.h>

inline static void CopyIOEvents(ImGuiContext* src, ImGuiContext* dst, ImVec2 origin, float scale)
{
    dst->InputEventsQueue = src->InputEventsTrail;
    for (ImGuiInputEvent& e : dst->InputEventsQueue) {
        if (e.Type == ImGuiInputEventType_MousePos) {
            e.MousePos.PosX = (e.MousePos.PosX - origin.x) / scale;
            e.MousePos.PosY = (e.MousePos.PosY - origin.y) / scale;
        }
    }
}

inline static void AppendDrawData(ImDrawList* src, ImVec2 origin, float scale)
{
    // TODO optimize if vtx_start == 0 || if idx_start == 0
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const int vtx_start = dl->VtxBuffer.size();
    const int idx_start = dl->IdxBuffer.size();
    dl->VtxBuffer.resize(dl->VtxBuffer.size() + src->VtxBuffer.size());
    dl->IdxBuffer.resize(dl->IdxBuffer.size() + src->IdxBuffer.size());
    dl->CmdBuffer.reserve(dl->CmdBuffer.size() + src->CmdBuffer.size());
    dl->_VtxWritePtr = dl->VtxBuffer.Data + vtx_start;
    dl->_IdxWritePtr = dl->IdxBuffer.Data + idx_start;
    const ImDrawVert* vtx_read = src->VtxBuffer.Data;
    const ImDrawIdx* idx_read = src->IdxBuffer.Data;
    for (int i = 0, c = src->VtxBuffer.size(); i < c; ++i) {
        dl->_VtxWritePtr[i].uv = vtx_read[i].uv;
        dl->_VtxWritePtr[i].col = vtx_read[i].col;
        dl->_VtxWritePtr[i].pos = vtx_read[i].pos * scale + origin;
    }
    for (int i = 0, c = src->IdxBuffer.size(); i < c; ++i) {
        dl->_IdxWritePtr[i] = idx_read[i] + vtx_start;
    }
    for (auto cmd : src->CmdBuffer) {
        cmd.IdxOffset += idx_start;
        //ASSERT(cmd.VtxOffset == 0)
        cmd.ClipRect.x = cmd.ClipRect.x * scale + origin.x;
        cmd.ClipRect.y = cmd.ClipRect.y * scale + origin.y;
        cmd.ClipRect.z = cmd.ClipRect.z * scale + origin.x;
        cmd.ClipRect.w = cmd.ClipRect.w * scale + origin.y;
        dl->CmdBuffer.push_back(cmd);
    }

    dl->_VtxCurrentIdx += src->VtxBuffer.size();
    dl->_VtxWritePtr = dl->VtxBuffer.Data + dl->VtxBuffer.size();
    dl->_IdxWritePtr = dl->IdxBuffer.Data + dl->IdxBuffer.size();
}

struct ViewPortConfig
{
    ImVec2 size = {0.f, 0.f};
    ImU32 color = IM_COL32_WHITE;
    bool zoom_enabled = true;
    float zoom_smoothness = 0.f;
    float default_zoom = 1.f;
    ImGuiKey reset_zoom_key = ImGuiKey_R;
    ImGuiMouseButton scroll_button = ImGuiMouseButton_Middle;
};

class ViewPort
{
public:
    ~ViewPort();
    void begin();
    void end();
    [[nodiscard]] float scale() const { return m_scale; }
    [[nodiscard]] const ImVec2& origin() const { return m_origin; }
    [[nodiscard]] bool hovered() const { return m_hovered; }
    [[nodiscard]] const ImVec2& scroll() const { return m_scroll; }
private:
    ViewPortConfig m_config;

    ImVec2 m_origin;
    ImGuiContext* m_ctx = nullptr;
    ImGuiContext* m_original_ctx = nullptr;

    bool m_anyWindowHovered = false;
    bool m_anyItemActive = false;
    bool m_hovered = false;

    float m_scale = m_config.default_zoom;
    ImVec2 m_scroll = {0.f, 0.f};
};

inline ViewPort::~ViewPort()
{
    if (m_ctx) ImGui::DestroyContext(m_ctx);
}

inline void ViewPort::begin()
{
    ImGui::PushID(this);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, m_config.color);
    ImGui::BeginChild("view_port", m_config.size, 0, ImGuiWindowFlags_NoMove);
    ImGui::PopStyleColor();

    ImVec2 size = ImGui::GetContentRegionAvail();
    m_origin = ImGui::GetCursorScreenPos();
    m_original_ctx = ImGui::GetCurrentContext();
    const ImGuiStyle& orig_style = ImGui::GetStyle();
    if (!m_ctx) m_ctx = ImGui::CreateContext(ImGui::GetIO().Fonts);
    ImGui::SetCurrentContext(m_ctx);
    ImGuiStyle& new_style = ImGui::GetStyle();
    new_style = orig_style;

    CopyIOEvents(m_original_ctx, m_ctx, m_origin, m_scale);

    ImGui::GetIO().DisplaySize = size / m_scale;
    ImGui::GetIO().ConfigInputTrickleEventQueue = false;
    ImGui::NewFrame();
}

inline void ViewPort::end()
{
    m_anyWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
    m_anyItemActive = ImGui::IsAnyItemActive();

    ImGui::Render();

    ImDrawData* draw_data = ImGui::GetDrawData();

    ImGui::SetCurrentContext(m_original_ctx);
    m_original_ctx = nullptr;

    for (int i = 0; i < draw_data->CmdListsCount; ++i)
        AppendDrawData(draw_data->CmdLists[i], m_origin, m_scale);

    m_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && !m_anyWindowHovered;

    // Zooming
    if (m_config.zoom_enabled && m_hovered && ImGui::GetIO().MouseWheel != 0.f)
    {
        m_scale += ImGui::GetIO().MouseWheel / 16;
        m_scale = m_scale < 0.3f ? 0.3f : m_scale;
        m_scale = m_scale > 2.f ? 2.f : m_scale;
    }

    // Zoom reset
    if (ImGui::IsKeyPressed(m_config.reset_zoom_key, false))
        m_scale = m_config.default_zoom;

    // Scrolling
    if (m_hovered && !m_anyItemActive && ImGui::IsMouseDragging(m_config.scroll_button, 0.f))
        m_scroll = m_scroll + ImGui::GetIO().MouseDelta;



    ImGui::EndChild();
    ImGui::PopID();
}
