#pragma once

#include "ImNodeFlow/detail/fwd.h"

namespace ImFlow::detail {

struct node_link_t {
    detail::node_t* source_node = nullptr;
    detail::node_t* target_node = nullptr;
    bool marked_for_deletion = false;
    bool hovered = false;
    bool selected = false;
};

} // namespace ImFlow::detail
