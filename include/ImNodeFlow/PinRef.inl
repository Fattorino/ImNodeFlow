#pragma once

#include "ImNodeFlow/PinRef.h"

#include "ImNodeFlow/PinType.h"
#include "ImNodeFlow/detail/fwd.h"
#include "ImNodeFlow/detail/type_id.h"

#include <cassert>

namespace ImFlow {

template <bool TIsConst>
template <typename T>
InPinRef<T, TIsConst>
PinRefImpl<TIsConst>::toIn()
{
    assert(this->getType() == PinType::Input && this->getTypeID() == detail::get_type_id<T>());

    return InPinRef<T, TIsConst> {
        *this,
        static_cast<detail::in_pin_t<T>&>(this->getDetail())
    };
}

template <bool TIsConst>
template <typename T>
OutPinRef<T, TIsConst>
PinRefImpl<TIsConst>::toOut()
{
    assert(this->getType() == PinType::Output && this->getTypeID() == detail::get_type_id<T>());

    return OutPinRef<T, TIsConst> {
        *this,
        static_cast<detail::out_pin_t<T>&>(this->getDetail())
    };
}

// -----------------------------------------------------------------------------------------------------------------
// IN PIN

template <typename T, bool TIsConst>
const T&
InPinRef<T, TIsConst>::value() const
{
    return m_in_pin.get().val;
}

// -----------------------------------------------------------------------------------------------------------------
// OUT PIN

template <typename T, bool TIsConst>
const T&
OutPinRef<T, TIsConst>::value() const
{
    return m_out_pin.get().val;
}

template <typename T, bool TIsConst>
OutPinRef<T, TIsConst>& OutPinRef<T, TIsConst>::behaviour(out_pin_behaviour_t<T> func)
    requires(!TIsConst)
{
    auto& pin = m_out_pin.get();
    pin.behaviour = std::move(func);
    return *this;
}

} // namespace ImFlow
