#include "pin_drawing_utils.h"

#include "ImNodeFlow/PinRef.h"
#include "ImNodeFlow/PinType.h"
#include "ImNodeFlow/fwd.h"

#include "./imgui_bezier_items.h"
#include "./pin_base.h"

#include "imgui.h"

namespace ImFlow::detail {

void draw_bezier_from_pin_to_pos(
    ImDrawList* draw_list,
    ConstPinRef pin,
    ImVec2 pos)
{
    // Links drag-out
    if (pin.getType() == PinType::Output) {
        detail::smart_bezier(
            draw_list,
            pin.pinPoint(),
            pos,
            pin.getStyle().color,
            pin.getStyle().extra.link_dragged_thickness);
    } else {
        detail::smart_bezier(
            draw_list,
            pos,
            pin.pinPoint(),
            pin.getStyle().color,
            pin.getStyle().extra.link_dragged_thickness);
    }
}

} // namespace ImFlow::detail
