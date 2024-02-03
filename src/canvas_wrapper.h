#pragma once

#include <imgui.h>
#include <imgui_internal.h>

using namespace ImGui;

#ifndef ASSERT
#if defined _MSC_VER && !defined __clang__
#define ASSERT(x) __assume(x)
#else
#define ASSERT(x) { false ? (void)(x) : (void)0; }
#endif
#endif

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
    ImDrawList* dl = GetWindowDrawList();
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

struct IMGUI_API Canvas
{
    ~Canvas();
    void begin(ImU32 color);
    void end();
    [[nodiscard]] bool hovered() const { return m_hovered; }
    [[nodiscard]] float scale() const { return m_scale; }
    [[nodiscard]] const ImVec2& origin() const { return m_origin; }

    ImVec2 m_origin;
    ImVec2 m_size = ImVec2(0, 0);
    float m_scale = 1.f;
    ImGuiContext* m_ctx = nullptr;
    ImGuiContext* m_original_ctx = nullptr;
    bool m_hovered = false;
};

inline Canvas::~Canvas()
{
    if (m_ctx) DestroyContext(m_ctx);
}

inline void Canvas::begin(ImU32 color)
{
    m_size = GetContentRegionAvail();
    m_origin = GetCursorScreenPos();
    m_original_ctx = GetCurrentContext();
    const ImGuiStyle& orig_style = GetStyle();
    if (!m_ctx) m_ctx = CreateContext(GetIO().Fonts);
    SetCurrentContext(m_ctx);
    ImGuiStyle& new_style = GetStyle();
    new_style = orig_style;

    CopyIOEvents(m_original_ctx, m_ctx, m_origin, m_scale);

    GetIO().DisplaySize = m_size / m_scale;
    GetIO().ConfigInputTrickleEventQueue = false;
    NewFrame();

    SetNextWindowPos(ImVec2(0, 0));
    SetNextWindowSize(m_size / m_scale);
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
    Begin("canvas_wrapper", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse);
    PopStyleVar(2);
    PopStyleColor();
}

inline void Canvas::end()
{
    m_hovered = IsWindowHovered();
    End();
    Render();

    ImDrawData* draw_data = GetDrawData();

    SetCurrentContext(m_original_ctx);
    m_original_ctx = nullptr;

    for (int i = 0; i < draw_data->CmdListsCount; ++i)
        AppendDrawData(draw_data->CmdLists[i], m_origin, m_scale);

    if (m_hovered && GetIO().MouseWheel != 0.f)
    {
        m_scale += GetIO().MouseWheel / 16;
        m_scale = m_scale < 0.3f ? 0.3f : m_scale;
        m_scale = m_scale > 1.5f ? 1.5f : m_scale;
    }
}
