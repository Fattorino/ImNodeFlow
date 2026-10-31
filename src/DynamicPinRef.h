#pragma once

#include "ImNodeFlow/detail/fwd.h"
#include "ImNodeFlow/fwd.h"

#include <functional>
#include <type_traits>

namespace ImFlow::detail {

/**
 * @brief A dynamic pin is a pin with a temporary lifetime, and will be marked
 * for deletion if not refreshed before every update
 */
template <bool TIsConst = false>
class DynamicPinRef {
public:
    using context_type = std::conditional_t<TIsConst, const ImNodeFlow, ImNodeFlow>;
    using dynamic_pin_type = std::conditional_t<TIsConst, const detail::dynamic_pin_t, detail::dynamic_pin_t>;

    DynamicPinRef(context_type& ctx, dynamic_pin_type& dynamic_pin);

    void destroy()
        requires(!TIsConst);
    void refresh()
        requires(!TIsConst);
    void update()
        requires(!TIsConst);
    [[nodiscard]] PinRefImpl<TIsConst> pin() const;
    [[nodiscard]] bool isStale() const;

private:
    std::reference_wrapper<context_type> m_ctx;
    std::reference_wrapper<dynamic_pin_type> m_pin;
};

extern template class DynamicPinRef<false>;
extern template class DynamicPinRef<true>;

} // namespace ImFlow::detail
