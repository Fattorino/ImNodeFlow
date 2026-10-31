#ifndef IMNODEFLOW_INCLUDE_DETAIL_FWD_H
#define IMNODEFLOW_INCLUDE_DETAIL_FWD_H
#pragma once

namespace ImFlow::detail {

class IPin;

struct dynamic_pin_t;
struct editor_context_t;
struct node_group_arena_t;
struct node_group_t;
struct node_link_t;
struct node_t;
struct pin_base_t;
struct pin_link_t;
struct root_node_group_t;

template <typename T>
struct in_pin_t;
template <typename T>
struct out_pin_t;

} // namespace ImFlow::detail

#endif // IMNODEFLOW_INCLUDE_DETAIL_FWD_H
