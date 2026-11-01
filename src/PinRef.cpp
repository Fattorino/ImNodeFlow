#include "ImNodeFlow/PinRef.h"

#include "ImNodeFlow/Context.h"
#include "ImNodeFlow/PinConnStrat.h"
#include "ImNodeFlow/PinType.h"
#include "ImNodeFlow/detail/IPin.h"
#include "ImNodeFlow/detail/fwd.h"

#include "./PinLinkRef.h"
#include "./imgui_compat.h"
#include "./pin_base.h"
#include "./pin_drawing_utils.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <format>
#include <string>
#include <utility>
#include <vector>

namespace ImFlow {

namespace {

    ImRect
    getSocketHitBounds(ConstPinRef pin, float expand_radius = -1.0F)
    {
        if (expand_radius < 0.0f) {
            expand_radius = pin.getStyle().socket_hovered_radius;
        }

        ImVec2 center = pin.pinPoint();
        ImVec2 tl = center - ImVec2(expand_radius, expand_radius);
        ImVec2 br = center + ImVec2(expand_radius, expand_radius);

        return { tl, br };
    }

    bool drawSocket(
        ConstPinRef pin,
        bool* hovered = nullptr,
        ImVec2* socketPos = nullptr)
    {
        float text_height = ImGui::GetTextLineHeight();
        float radius = pin.getStyle().socket_radius;
        float maxRadius = std::max(
            pin.getStyle().socket_radius,
            std::max(
                pin.getStyle().socket_hovered_radius,
                pin.getStyle().socket_connected_radius));
        ImU32 color = pin.getStyle().color;
        int shape = pin.getStyle().socket_shape;
        float thickness = pin.getStyle().socket_thickness;
        bool circle_filled = false;

        if (ConstPinRef { pin }.isConnected()) {
            circle_filled = true;
            radius = pin.getStyle().socket_connected_radius;
        } else if (pin.isHovered()) {
            radius = pin.getStyle().socket_hovered_radius;
        }

        const ImGuiPayload* payload = ImGui::GetDragDropPayload();
        if (payload != nullptr && payload->IsDataType("PIN_REF_LOCAL_DTO")) {
            PinRefLocalDTO<> dto { };
            IM_ASSERT(payload->DataSize == sizeof(dto));
            std::memcpy(&dto, payload->Data, sizeof(dto));
            PinRef source_pin { dto };
            bool is_compatible = pin.canLink(source_pin);
            if (is_compatible) {
                radius *= 1.25F;
            } else {
                color = ImGui::ColorConvertFloat4ToU32(ImVec4(0.4F, 0.4F, 0.4F, 0.5F));
            }
        }

        ImVec2 socket_size { maxRadius * 2.0F, maxRadius * 2.0F };
        ImVec2 item_size { maxRadius * 2.0F, text_height };
        ImVec2 point = ImGui::GetCursorPos() + item_size / 2.0F;

        // advance cursor
        ImGui::ItemSize(item_size);

        ImRect bb {
            point - socket_size / 2.0F,
            point + socket_size / 2.0F,
        };

        bool is_hovered = false;
        bool is_pressed = false;

        if (ImGui::ItemAdd(bb, ImGui::GetID("##pinSocket"))) {
            is_pressed = ImGui::ButtonBehavior(
                bb,
                ImGui::GetID("##pinSocket"),
                &is_hovered,
                nullptr,
                ImGuiButtonFlags_PressedOnClick);

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            if (circle_filled) {
                draw_list->AddCircleFilled(point, radius, color, shape);
            } else {
                draw_list->AddCircle(point, radius, color, shape, thickness);
            }
        }

        if (hovered != nullptr) {
            *hovered = is_hovered;
        }
        if (socketPos != nullptr) {
            *socketPos = point;
        }

        return is_pressed;
    }

    void
    drawDecoration(detail::pin_base_t const& pin)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        if (pin.isHovered) {
            draw_list->AddRectFilled(
                pin.pos - pin.style.extra.padding,
                pin.pos + pin.size + pin.style.extra.padding,
                pin.style.extra.bg_hover_color,
                pin.style.extra.bg_radius);
        } else {
            draw_list->AddRectFilled(
                pin.pos - pin.style.extra.padding,
                pin.pos + pin.size + pin.style.extra.padding,
                pin.style.extra.bg_color,
                pin.style.extra.bg_radius);
        }

        compat::AddRect(
            draw_list,
            pin.pos - pin.style.extra.padding,
            pin.pos + pin.size + pin.style.extra.padding,
            pin.style.extra.border_color,
            pin.style.extra.bg_radius,
            pin.style.extra.border_thickness);
    }

    void drawDragDropPinSourceAndTarget(
        PinRef pin,
        bool* out_source_active = nullptr,
        bool* out_target_active = nullptr)
    {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
            auto dto = pin.serialize();
            auto type_id = pin.getTypeID();
            std::string payload_id = "PIN_REF_LOCAL_DTO_" + std::to_string(type_id);
            ImGui::SetDragDropPayload(payload_id.c_str(), &dto, sizeof(dto));
            ImGui::EndDragDropSource();

            if (out_source_active != nullptr) {
                *out_source_active = true;
            }
        }

        if (ImGui::BeginDragDropTarget()) {
            auto type_id = pin.getTypeID();
            std::string payload_id = "PIN_REF_LOCAL_DTO_" + std::to_string(type_id);
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payload_id.c_str(),
                ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
            if (payload != nullptr) {
                PinRefLocalDTO<> dto { };
                IM_ASSERT(payload->DataSize == sizeof(dto));
                std::memcpy(&dto, payload->Data, sizeof(dto));
                pin.linkTo(PinRef { dto });
            }

            ImGui::EndDragDropTarget();

            if (out_target_active != nullptr) {
                *out_target_active = true;
            }
        }
    }

} // anonymous namespace

template <bool TIsConst>
void PinRefImpl<TIsConst>::update()
    requires(!TIsConst)
{
    detail::pin_base_t& pin = m_pin.get();

    ImGui::PushID(&pin);

    bool socket_hovered = false;
    bool socket_pressed = false;
    bool drag_drop_source_active = false;
    bool drag_drop_target_active = false;

    if (this->getType() == PinType::Output) {
        this->setPos(ImGui::GetCursorPos());
        ImGui::BeginGroup();
        pin.renderer(*this);
        ImGui::EndGroup();
        pin.size = ImGui::GetItemRectSize();
        drawDecoration(pin);
        ImGui::SameLine(0.f, 0.f);
        socket_pressed = drawSocket(*this, &socket_hovered, &pin.pinPoint);
        drawDragDropPinSourceAndTarget(*this, &drag_drop_source_active, &drag_drop_target_active);
    } else if (this->getType() == PinType::Input) {
        socket_pressed = drawSocket(*this, &socket_hovered, &pin.pinPoint);
        drawDragDropPinSourceAndTarget(*this, &drag_drop_source_active, &drag_drop_target_active);
        ImGui::SameLine(0.f, 0.f);
        this->setPos(ImGui::GetCursorPos());
        ImGui::BeginGroup();
        pin.renderer(*this);
        ImGui::EndGroup();
        pin.size = ImGui::GetItemRectSize();
        drawDecoration(pin);
    }

    // here, decoration is everything except the pin's socket
    bool decoration_hovered = false;
    bool decoration_pressed = false;

    ImRect decoration_bb { pin.pos, pin.pos + pin.size };
    ImGuiID decoration_id = ImGui::GetID("##pinDecor");
    if (ImGui::ItemAdd(decoration_bb, decoration_id)) {
        decoration_pressed = ImGui::ButtonBehavior(
            ImRect { pin.pos, pin.pos + pin.size },
            decoration_id,
            &decoration_hovered,
            nullptr,
            ImGuiButtonFlags_PressedOnClick);
    }

    drawDragDropPinSourceAndTarget(*this, &drag_drop_source_active, &drag_drop_target_active);

    if (drag_drop_source_active) {
        detail::draw_bezier_from_pin_to_pos(
            ImGui::GetForegroundDrawList(),
            *this,
            ImGui::GetMousePos());
    }

    bool is_hovering = decoration_hovered
        || socket_hovered
        || drag_drop_target_active;
    bool is_pressed = decoration_pressed
        || socket_pressed;

    pin.isHovered = is_hovering;

    ImGui::PopID();
}

template <bool TIsConst>
PinRefImpl<TIsConst>&
PinRefImpl<TIsConst>::renderer(pin_rendering_handler_t r)
    requires(!TIsConst)
{
    auto& base = m_pin.get();
    base.renderer = std::move(r);
    return *this;
}

template <bool TIsConst>
bool PinRefImpl<TIsConst>::canLink(ConstPinRef other) const
{
    auto& pin = m_pin.get();

    if (&m_ctx.get() != &other.getContext()) {
        return false;
    }

    return m_ctx.get().canLink(*this, other);
}

template <bool TIsConst>
void PinRefImpl<TIsConst>::linkTo(PinRef other)
    requires(!TIsConst)
{
    auto& base = m_pin.get();
    auto& other_base = other.m_pin.get();

    if (&m_ctx.get() != &other.m_ctx.get()) {
        // Cannot link pins across different contexts
        return;
    }

    m_ctx.get().createLink(*this, other);
}

template <bool TIsConst>
void PinRefImpl<TIsConst>::deleteAllLinks()
    requires(!TIsConst)
{
    auto& pin = m_pin.get();
    auto links = std::move(pin.links);
    for (auto& l : links) {
        detail::PinLinkRef { m_ctx, *l }.destroy();
    }
}

template <bool TIsConst>
bool PinRefImpl<TIsConst>::isConnected() const
{
    auto& base = m_pin.get();
    return !base.links.empty();
}

template <bool TIsConst>
bool PinRefImpl<TIsConst>::isHovered() const
{
    auto& base = m_pin.get();
    return !base.isHovered;
}

template <bool TIsConst>
std::vector<PinRefImpl<TIsConst>>
PinRefImpl<TIsConst>::getConnections() const
{
    std::vector<PinRefImpl<TIsConst>> result;

    auto& pin = this->m_pin.get();
    result.reserve(pin.links.size());

    for (auto& l : pin.links) {
        // Not connected for some reason?
        if (l == nullptr) {
            continue;
        }

        detail::PinLinkRef<TIsConst> link { m_ctx, *l };

        switch (pin.type) {
        case PinType::Input:
            result.emplace_back(link.outPin());
            break;

        case PinType::Output:
            result.emplace_back(link.inPin());
            break;

        default:
            assert(false && "Detected memory corruption");
            break;
        }
    }

    return result;
}

template <bool TIsConst>
PinUID PinRefImpl<TIsConst>::getUid() const
{
    return m_pin.get().uid;
}

template <bool TIsConst>
const std::string& PinRefImpl<TIsConst>::getName() const
{
    return m_pin.get().name;
}

template <bool TIsConst>
const ImVec2& PinRefImpl<TIsConst>::getPos() const
{
    return m_pin.get().pos;
}

template <bool TIsConst>
const ImVec2& PinRefImpl<TIsConst>::getSize() const
{
    return m_pin.get().size;
}

template <bool TIsConst>
NodeRefImpl<TIsConst> PinRefImpl<TIsConst>::getParent() const
{
    return NodeRefImpl<TIsConst>(m_ctx, *m_pin.get().parent);
}

template <bool TIsConst>
PinType PinRefImpl<TIsConst>::getType() const
{
    return m_pin.get().type;
}

template <bool TIsConst>
uint64_t PinRefImpl<TIsConst>::getTypeID() const
{
    return m_pin.get().underlying_pin->getTypeID();
}

template <bool TIsConst>
const PinStyle& PinRefImpl<TIsConst>::getStyle() const
{
    return m_pin.get().style;
}

template <bool TIsConst>
void PinRefImpl<TIsConst>::setStyle(PinStyle style)
    requires(!TIsConst)
{
    m_pin.get().style = std::move(style);
}

template <bool TIsConst>
void PinRefImpl<TIsConst>::allowSameNodeConnections(bool state)
    requires(!TIsConst)
{
    m_pin.get().allowSelfConnection = state;
}

template <bool TIsConst>
bool PinRefImpl<TIsConst>::allowsSameNodeConnections() const
{
    return m_pin.get().allowSelfConnection;
}

template <bool TIsConst>
ImVec2 PinRefImpl<TIsConst>::pinPoint() const
{
    return m_pin.get().pinPoint;
}

template <bool TIsConst>
float PinRefImpl<TIsConst>::calcWidth() const
{
    return ImGui::CalcTextSize(m_pin.get().name.c_str()).x;
}

template <bool TIsConst>
void PinRefImpl<TIsConst>::setPos(ImVec2 pos)
    requires(!TIsConst)
{
    m_pin.get().pos = pos;
}

template <bool TIsConst>
void PinRefImpl<TIsConst>::setSocketHitboxEnabled(bool enabled)
    requires(!TIsConst)
{
    m_pin.get().socketHitboxEnabled = enabled;
}

template <bool TIsConst>
void PinRefImpl<TIsConst>::calculate()
    requires(!TIsConst)
{
    m_pin.get().underlying_pin->calculate(*this);
}

template <bool TIsConst>
void PinRefImpl<TIsConst>::prime()
    requires(!TIsConst)
{
    m_pin.get().underlying_pin->prime();
}

template <bool TIsConst>
void PinRefImpl<TIsConst>::destroy()
    requires(!TIsConst)
{
    this->deleteAllLinks();
}

template <bool TIsConst>
ImNodeFlow&
PinRefImpl<TIsConst>::getContext() const
    requires(!TIsConst)
{
    return m_ctx.get();
}

template <bool TIsConst>
ImNodeFlow const&
PinRefImpl<TIsConst>::getContext() const
{
    return m_ctx.get();
}

template <bool TIsConst>
detail::IPin&
PinRefImpl<TIsConst>::getDetail() const
    requires(!TIsConst)
{
    return *m_pin.get().underlying_pin;
}

template <bool TIsConst>
detail::IPin const&
PinRefImpl<TIsConst>::getDetail() const
{
    return *m_pin.get().underlying_pin;
}

template class PinRefImpl<false>;
template class PinRefImpl<true>;

} // namespace ImFlow
