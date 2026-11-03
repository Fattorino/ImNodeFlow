#pragma once

#include "ImNodeFlow/NodeRef.h"

#include "ImNodeFlow/PinInfo.h"
#include "ImNodeFlow/PinRef.h"
#include "ImNodeFlow/PinType.h"
#include "ImNodeFlow/detail/in_pin.h"
#include "ImNodeFlow/detail/out_pin.h"

namespace ImFlow {

template <bool TIsConst>
template <typename T>
InPinRef<T, TIsConst>
NodeRefImpl<TIsConst>::addIN(InPinInfo<T> pin_info)
    requires(!TIsConst)
{
    auto new_pin = this->addIN_impl(
        BasePinInfo {
            .name = std::move(pin_info.name),
            .allow_same_node_connections = pin_info.allow_same_node_connections,
            .style = std::move(pin_info.style),
            .strat = pin_info.strat,
            .renderer = std::move(pin_info.renderer),
        },
        std::make_unique<detail::in_pin_t<T>>(pin_info));

    return new_pin.template toIn<T>();
}

template <bool TIsConst>
template <typename T>
const T&
NodeRefImpl<TIsConst>::showIN(InPinInfo<T> pin_info)
    requires(!TIsConst)
{
    auto pin = tempInPin(pin_info.name);
    if (pin.has_value()) {
        // TODO: Update pin with new styles from pin_info
        return pin->template toIn<T>().value();
    }

    auto new_pin = this->showIN_impl(
        m_node.get().ctx,
        *this,
        PinType::Input,
        BasePinInfo {
            .name = std::move(pin_info.name),
            .allow_same_node_connections = pin_info.allow_same_node_connections,
            .style = std::move(pin_info.style),
            .strat = pin_info.strat,
            .renderer = std::move(pin_info.renderer),
        },
        std::make_unique<detail::in_pin_t<T>>(pin_info));

    return new_pin.template toIn<T>().value();
}

template <bool TIsConst>
template <typename T>
OutPinRef<T, TIsConst>
NodeRefImpl<TIsConst>::addOUT(OutPinInfo<T> pin_info)
    requires(!TIsConst)
{
    auto new_pin = this->addOUT_impl(
        BasePinInfo {
            .name = std::move(pin_info.name),
            .allow_same_node_connections = pin_info.allow_same_node_connections,
            .style = std::move(pin_info.style),
            .strat = pin_info.strat,
            .renderer = std::move(pin_info.renderer),
        },
        std::make_unique<detail::out_pin_t<T>>(pin_info));

    return new_pin.template toOut<T>();
}

template <bool TIsConst>
template <typename T>
void NodeRefImpl<TIsConst>::showOUT(OutPinInfo<T> pin_info)
    requires(!TIsConst)
{
    auto pin = this->tempOutPin(pin_info.name);
    if (pin.has_value()) {
        // TODO: Update pin with new styles from pin_info
        pin->template toOut<T>().behaviour(std::move(pin_info.behaviour));
        return;
    }

    auto new_pin = this->showOUT_impl(
        BasePinInfo {
            .name = std::move(pin_info.name),
            .allow_same_node_connections = pin_info.allow_same_node_connections,
            .style = std::move(pin_info.style),
            .strat = pin_info.strat,
            .renderer = std::move(pin_info.renderer),
        },
        std::make_unique<detail::out_pin_t<T>>(pin_info));
}

template <bool TIsConst>
template <typename T>
std::optional<InPinRef<T, TIsConst>>
NodeRefImpl<TIsConst>::inPin(const PinUID& uid)
{
    auto pin = this->inPin(uid);
    if (!pin.has_value()) {
        return std::nullopt;
    }

    return pin->template toIn<T>();
}

template <bool TIsConst>
template <typename T>
std::optional<InPinRef<T, TIsConst>>
NodeRefImpl<TIsConst>::inPin(std::string_view name)
{
    auto pin = this->inPin(name);
    if (!pin.has_value()) {
        return std::nullopt;
    }

    return pin->template toIn<T>();
}

template <bool TIsConst>
template <typename T>
std::optional<OutPinRef<T, TIsConst>>
NodeRefImpl<TIsConst>::outPin(const PinUID& uid)
{
    auto pin = this->outPin(uid);
    if (!pin.has_value()) {
        return std::nullopt;
    }

    return pin->template toOut<T>();
}

template <bool TIsConst>
template <typename T>
std::optional<OutPinRef<T, TIsConst>>
NodeRefImpl<TIsConst>::outPin(std::string_view name)
{
    auto pin = this->outPin(name);
    if (!pin.has_value()) {
        return std::nullopt;
    }

    return pin->template toOut<T>();
}

} // namespace ImFlow
