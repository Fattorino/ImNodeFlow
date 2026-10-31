#ifndef IMNODEFLOW_DETAIL_INPIN_H
#define IMNODEFLOW_DETAIL_INPIN_H
#include "ImNodeFlow/fwd.h"
#pragma once

#include "ImNodeFlow/PinInfo.h"
#include "ImNodeFlow/callback_types.h"
#include "ImNodeFlow/detail/IPin.h"
#include "ImNodeFlow/detail/fwd.h"

#include <cstdint>

namespace ImFlow::detail {

template <typename T>
struct in_pin_t final : public IPin {
    T emptyVal;
    T val;
    T valNext;
    in_pin_accumulator_t<T> accumulatorOp;

    explicit in_pin_t(InPinInfo<T>& info) noexcept
        : emptyVal { info.default_value }
        , val { info.default_value }
        , accumulatorOp {
            info.accumulate_op
                ? std::move(info.accumulate_op)
                : [](bool first_pin, T init, T const& incoming_value) -> T {
                return incoming_value;
            },
        }
    {
    }

    [[nodiscard]] uint64_t getTypeID() const final
    {
        return get_type_id<T>();
    }

    void calculate(PinRef self) final
    {
        T next_value = this->emptyVal;

        next_value = this->emptyVal;

        bool first = true;
        auto connections = self.getConnections();
        for (auto& pin : connections) {
            auto out_pin = pin.toOut<T>();
            next_value = this->accumulatorOp(
                first,
                next_value,
                out_pin.value());
            first = false;
        }

        this->valNext = next_value;
    }

    void prime() final
    {
        this->val = this->valNext;
    }
};

} // namespace ImFlow::detail

#endif // IMNODEFLOW_DETAIL_INPIN_H
