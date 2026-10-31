#ifndef IMNODEFLOW_DETAIL_OUTPIN_H
#define IMNODEFLOW_DETAIL_OUTPIN_H
#pragma once

#include "ImNodeFlow/NodeRef.h"
#include "ImNodeFlow/PinInfo.h"
#include "ImNodeFlow/PinRef.h"
#include "ImNodeFlow/callback_types.h"
#include "ImNodeFlow/detail/IPin.h"
#include "ImNodeFlow/detail/fwd.h"
#include "ImNodeFlow/detail/type_id.h"
#include "ImNodeFlow/fwd.h"

#include <cstdint>

namespace ImFlow::detail {

template <typename T>
struct out_pin_t final : public IPin {
    out_pin_behaviour_t<T> behaviour;
    T val;
    T valNext;

    explicit out_pin_t(
        OutPinInfo<T>& info) noexcept
        : behaviour { std::move(info.behaviour) }
        , val { info.default_value }
        , valNext { info.default_value }
    {
    }

    [[nodiscard]] uint64_t getTypeID() const final
    {
        return get_type_id<T>();
    }

    void calculate(PinRef self) final
    {
        this->valNext = this->behaviour(self.getParent());
    }

    void prime() final
    {
        this->val = this->valNext;
    }
};

} // namespace ImFlow::detail

#endif // IMNODEFLOW_DETAIL_OUTPIN_H
