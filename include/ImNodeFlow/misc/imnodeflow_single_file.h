// ImNodeFlow: single-file wrapper include
//
// Do this:
//     #define IMNODEFLOW_IMPLEMENTATION
// Before you include this file in *one* C++ file to create the implementation.

#ifndef IMNODEFLOW_SINGLE_FILE_H
#define IMNODEFLOW_SINGLE_FILE_H
#pragma once

// NOLINTBEGIN (bugprone-suspicious-include)
#ifdef IMNODEFLOW_IMPLEMENTATION
#include "./../../../src/Context.cpp"
#include "./../../../src/DynamicPinRef.cpp"
#include "./../../../src/IContainedContext.cpp"
#include "./../../../src/INodeBehaviour.cpp"
#include "./../../../src/NodeGroupArenaRef.cpp"
#include "./../../../src/NodeLinkRef.cpp"
#include "./../../../src/NodeRef.cpp"
#include "./../../../src/PinLinkRef.cpp"
#include "./../../../src/PinRef.cpp"
#include "./../../../src/imgui_bezier_items.cpp"
#include "./../../../src/imgui_bezier_widgets.cpp"
#include "./../../../src/pin_base.cpp"
#include "./../../../src/pin_drawing_utils.cpp"
#include "./../../../src/uid_helpers.cpp"
#endif // IMNODEFLOW_IMPLEMENTATION
// NOLINTEND

#endif // IMNODEFLOW_SINGLE_FILE_H
