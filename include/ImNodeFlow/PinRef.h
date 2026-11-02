#ifndef IMNODEFLOW_PINREF_H
#define IMNODEFLOW_PINREF_H
#include <cassert>
#include <string>
#pragma once

#include "ImNodeFlow/PinStyle.h"
#include "ImNodeFlow/PinType.h"
#include "ImNodeFlow/PinUID.h"
#include "ImNodeFlow/callback_types.h"
#include "ImNodeFlow/detail/fwd.h"
#include "ImNodeFlow/fwd.h"

#include "imgui.h"

#include <functional>
#include <type_traits>
#include <vector>

namespace ImFlow {

template <bool TIsConst = false>
struct PinRefLocalDTO {
    using context_type = std::conditional_t<
        TIsConst, const ImNodeFlow, ImNodeFlow>;
    using pin_base_type = std::conditional_t<
        TIsConst, const detail::pin_base_t, detail::pin_base_t>;

    context_type* ctx;
    pin_base_type* pin;
};

static_assert(std::is_trivially_copyable_v<PinRefLocalDTO<false>>);
static_assert(std::is_trivially_copyable_v<PinRefLocalDTO<true>>);

/**
 * @brief Generic base class for pins
 */
template <bool TIsConst>
class PinRefImpl {
public:
    using context_type = std::conditional_t<
        TIsConst, const ImNodeFlow, ImNodeFlow>;
    using pin_base_type = std::conditional_t<
        TIsConst, const detail::pin_base_t, detail::pin_base_t>;

    explicit PinRefImpl(context_type& ctx, pin_base_type& pin)
        : m_ctx(ctx)
        , m_pin(pin)
    {
    }

    explicit PinRefImpl(PinRefLocalDTO<TIsConst> dto)
        : m_ctx(*dto.ctx)
        , m_pin(*dto.pin)
    {
        assert(dto.ctx != nullptr);
        assert(dto.pin != nullptr);
    }

    virtual ~PinRefImpl() noexcept = default;

    [[nodiscard]] PinRefLocalDTO<TIsConst> serialize() const
    {
        return PinRefLocalDTO<TIsConst> {
            .ctx = &m_ctx.get(),
            .pin = &m_pin.get(),
        };
    }

    /**
     * @brief Safely casts a base Pin into the underlying InPin, if
     * possible. Crashes the program otherwise.
     *
     * @return Reference to the underlying InPin<T> if possible. Crash
     * otherwise.
     */
    template <typename T>
    InPinRef<T, TIsConst> toIn();

    /**
     * @brief Safely casts a base Pin into the underlying OutPin, if
     * possible. Crashes the program otherwise.
     *
     * @return Reference to the underlying OutPin<T> if possible. Crash
     * otherwise.
     */
    template <typename T>
    OutPinRef<T, TIsConst> toOut();

    /**
     * @brief Main loop of the pin
     * @details Updates position, hovering and dragging status, and renders the pin. Must be called each frame.
     * mouse is hovering over the pin
     */
    void update()
        requires(!TIsConst);

    /**
     * @brief Custom render function to override Pin appearance
     * @param r Function or lambda expression with new ImGui rendering
     */
    PinRefImpl& renderer(pin_rendering_handler_t r)
        requires(!TIsConst);

    [[nodiscard]] bool canLink(ConstPinRef other) const;

    /**
     * @brief Creates a new link between this pin and another pin
     * @param other Reference of the pin we want to connect with
     */
    void linkTo(PinRef other)
        requires(!TIsConst);

    /**
     * @brief Delete all link reference
     */
    void deleteAllLinks()
        requires(!TIsConst);

    /**
     * @brief Get connected status
     * @return [TRUE] if the pin is connected
     */
    [[nodiscard]] bool isConnected() const;

    [[nodiscard]] bool isHovered() const;

    [[nodiscard]] std::vector<PinRefImpl<TIsConst>> getConnections() const;

    /**
     * @brief Get pin's UID
     * @return Unique identifier of the pin
     */
    [[nodiscard]] PinUID getUid() const;

    /**
     * @brief Get pin's name
     * @return Const reference to pin's name
     */
    [[nodiscard]] const std::string& getName() const;

    /**
     * @brief Get pin's position
     * @return Const reference to pin's position in grid coordinates
     */
    [[nodiscard]] const ImVec2& getPos() const;

    /**
     * @brief Get pin's hit-box size
     * @return Const reference to pin's hit-box size
     */
    [[nodiscard]] const ImVec2& getSize() const;

    /**
     * @brief Get pin's parent node
     * @return Generic type pointer to pin's parent node. (Node that contains it)
     */
    [[nodiscard]] NodeRefImpl<TIsConst> getParent() const;

    /**
     * @brief Get pin's type
     * @return The pin type. Either Input or Output
     */
    [[nodiscard]] PinType getType() const;

    /**
     * @brief Get pin's data type (aka: \<T>)
     * @return String containing unique information identifying the data type
     */
    [[nodiscard]] uint64_t getTypeID() const;

    /**
     * @brief Get pin's style
     * @return Reference to pin's style
     */
    const PinStyle& getStyle() const;

    /**
     * @brief Sets pin's style
     */
    void setStyle(PinStyle style)
        requires(!TIsConst);

    /**
     * @brief Specify if connections from an output on the same node are allowed
     * @param state New state of the flag
     */
    void allowSameNodeConnections(bool state)
        requires(!TIsConst);

    /**
     * @brief Specify if connections from an output on the same node are allowed
     * @param state New state of the flag
     */
    [[nodiscard]] bool allowsSameNodeConnections() const;

    /**
     * @brief Get pin's link attachment point (socket)
     * @return Grid coordinates to the attachment point between the link and the pin's socket
     */
    ImVec2 pinPoint() const;

    /**
     * @brief Calculate pin's width pre-rendering
     * @return The with of the pin once it will be rendered
     */
    float calcWidth() const;

    /**
     * @brief Set pin's position
     * @param pos Position in screen coordinates
     */
    void setPos(ImVec2 pos)
        requires(!TIsConst);

    /**
     * @brief Enable/disable automatic socket hitbox extension
     * @param enabled If true, socket area will be included in pin hitbox
     */
    void setSocketHitboxEnabled(bool enabled)
        requires(!TIsConst);

    /**
     * @brief When the context calls this method, the pins should calculate
     * their next value, but not apply it. This allows every pin, during
     * the calculation step, to query what every other pin's output value
     * was in the closing iteration step.
     * @internal This function is for internal use only
     */
    void calculate()
        requires(!TIsConst);

    /**
     * @brief When the context calls this method, the pins should update
     * their outputs to the computed value in calculate().
     *
     * During this step, pin's are no longer allowed to query any other
     * part of the flow graph, since their state is undefined.
     * @internal This function is for internal use only
     */
    void prime()
        requires(!TIsConst);

    /**
     * @internal For internal use only
     */
    void destroy()
        requires(!TIsConst);

    /**
     * @internal For internal use only
     */
    operator ConstPinRef() const
    {
        return ConstPinRef { m_ctx, m_pin };
    }

    /**
     * @internal For internal use only
     */
    operator pin_base_type&() const
    {
        return m_pin.get();
    }

    /**
     * @internal For internal use only
     */
    ImNodeFlow& getContext() const
        requires(!TIsConst);

    /**
     * @internal For internal use only
     */
    ImNodeFlow const& getContext() const;

    detail::IPin& getDetail() const
        requires(!TIsConst);

    detail::IPin const& getDetail() const;

    [[nodiscard]] constexpr bool operator==(ConstPinRef const& other) const noexcept
    {
        return &m_pin.get() == &static_cast<detail::pin_base_t const&>(other);
    }

protected:
    std::reference_wrapper<context_type> m_ctx;
    std::reference_wrapper<pin_base_type> m_pin;
};

/**
 * @brief Input specific pin
 * @details Derived from the generic class Pin. The input pin owns the link pointer.
 * @tparam T Data type handled by the pin
 */
template <class T, bool TIsConst>
class InPinRef : public PinRefImpl<TIsConst> {
public:
    using underlyin_type = std::conditional_t<
        TIsConst,
        const detail::in_pin_t<T>,
        detail::in_pin_t<T>>;

    /**
     * @brief Input pin constructor
     * @param ctx Reference to the ImNodeFlow context
     * @param name Name of the pin
     * @param parent Pointer to the Node containing the pin
     * @param defReturn Default return value when the pin is not connected
     * @param style Style of the pin
     */
    explicit InPinRef(PinRefImpl<TIsConst> pin_ref, underlyin_type& pin)
        : PinRefImpl<TIsConst>(pin_ref)
        , m_in_pin(pin)
    {
    }

    /**
     * @brief Get value carried by the connected link
     * @return Reference to the value of the connected OutPin. Or the default value if not connected
     */
    const T& value() const;

private:
    std::reference_wrapper<underlyin_type> m_in_pin;
};

/**
 * @brief Output specific pin
 * @details Derived from the generic class Pin. The output pin handles the logic.
 * @tparam T Data type handled by the pin
 */
template <class T, bool TIsConst>
class OutPinRef final : public PinRefImpl<TIsConst> {
public:
    using underlying_type = std::conditional_t<
        TIsConst,
        const detail::out_pin_t<T>,
        detail::out_pin_t<T>>;

    explicit OutPinRef(PinRefImpl<TIsConst> pin_ref, underlying_type& out_pin)
        : PinRefImpl<TIsConst>(pin_ref)
        , m_out_pin(out_pin)
    {
    }

    ~OutPinRef() override = default;

    /**
     * @brief Get output value
     * @return Const reference to the internal value of the pin
     */
    const T& value() const;

    /**
     * @brief Set logic to calculate output value
     * @details Used to define the pin behaviour. This is what gets the data from the parent's inputs, and applies the needed logic.
     * @param func Function or lambda expression used to calculate output value
     */
    OutPinRef<T, TIsConst>& behaviour(out_pin_behaviour_t<T> func)
        requires(!TIsConst);

private:
    std::reference_wrapper<underlying_type> m_out_pin;
};

extern template class PinRefImpl<false>;
extern template class PinRefImpl<true>;

} // namespace ImFlow

#include "./PinRef.inl"

#endif // IMNODEFLOW_PINREF_H
