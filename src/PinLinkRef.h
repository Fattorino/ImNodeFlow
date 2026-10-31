#pragma once

#include "ImNodeFlow/Context.h"
#include "ImNodeFlow/detail/fwd.h"
#include "ImNodeFlow/fwd.h"

#include "./pin_link.h"

#include <functional>
#include <type_traits>

namespace ImFlow::detail {

template <bool TIsConst = false>
class PinLinkRef {
public:
    using context_type = std::conditional_t<TIsConst, const ImNodeFlow, ImNodeFlow>;
    using link_type = std::conditional_t<TIsConst, const pin_link_t, pin_link_t>;

    PinLinkRef(context_type& ctx, link_type& link);

    [[nodiscard]] PinRefImpl<TIsConst> outPin() const;
    [[nodiscard]] PinRefImpl<TIsConst> inPin() const;

    [[nodiscard]] bool isAlive() const;

    void connect(PinRef& out_pin, PinRef& in_pin)
        requires(!TIsConst);

    void destroy()
        requires(!TIsConst);

    void update()
        requires(!TIsConst);

private:
    std::reference_wrapper<context_type> m_ctx;
    std::reference_wrapper<link_type> m_link;
};

extern template class PinLinkRef<false>;
extern template class PinLinkRef<true>;

} // namespace ImFlow::detail
