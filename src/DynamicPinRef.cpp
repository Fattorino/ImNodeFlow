#include "./DynamicPinRef.h"

#include "ImNodeFlow/PinRef.h"

#include "./dynamic_pin.h"

namespace ImFlow::detail {

template <bool TIsConst>
DynamicPinRef<TIsConst>::DynamicPinRef(context_type& ctx, dynamic_pin_type& dynamic_pin)
    : m_ctx(ctx)
    , m_pin(dynamic_pin)
{
}

template <bool TIsConst>
void DynamicPinRef<TIsConst>::destroy()
    requires(!TIsConst)
{
    this->pin().destroy();
}

template <bool TIsConst>
void DynamicPinRef<TIsConst>::refresh()
    requires(!TIsConst)
{
    m_pin.get().ttl = 1;
}

template <bool TIsConst>
void DynamicPinRef<TIsConst>::update()
    requires(!TIsConst)
{
    auto& pin = m_pin.get();

    pin.ttl = pin.ttl > 0 ? pin.ttl - 1 : pin.ttl;
    this->pin().update();
}

template <bool TIsConst>
PinRefImpl<TIsConst> DynamicPinRef<TIsConst>::pin() const
{
    return PinRefImpl<TIsConst> { m_ctx, m_pin.get().pin };
}

template <bool TIsConst>
bool DynamicPinRef<TIsConst>::isStale() const
{
    return m_pin.get().ttl == 0;
}

template class DynamicPinRef<false>;
template class DynamicPinRef<true>;

} // namespace ImFlow::detail
