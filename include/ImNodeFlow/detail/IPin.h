#ifndef IMNODEFLOW_DETAIL_IPIN_H
#define IMNODEFLOW_DETAIL_IPIN_H
#pragma once

#include "ImNodeFlow/detail/fwd.h"
#include "ImNodeFlow/fwd.h"

#include <cstdint>

namespace ImFlow::detail {

class IPin {
public:
    virtual ~IPin() = default;

    [[nodiscard]] virtual uint64_t getTypeID() const = 0;
    virtual void calculate(PinRef self) = 0;
    virtual void prime() = 0;
};

} // namespace ImFlow::detail

#endif // IMNODEFLOW_DETAIL_IPIN_H
