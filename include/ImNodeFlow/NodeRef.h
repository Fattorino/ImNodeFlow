#ifndef IMNODEFLOW_NODEREF_H
#define IMNODEFLOW_NODEREF_H
#pragma once

#include "ImNodeFlow/NodeUID.h"
#include "ImNodeFlow/PinInfo.h"
#include "ImNodeFlow/PinUID.h"
#include "ImNodeFlow/detail/fwd.h"
#include "ImNodeFlow/fwd.h"

#include <optional>
#include <string_view>
#include <type_traits>

namespace ImFlow {

/**
 * @brief Parent class for custom nodes
 * @details Main class from which custom nodes can be created. All interactions with the main grid are handled internally.
 */
template <bool TIsConst>
class NodeRefImpl {
public:
    using node_type = std::conditional_t<TIsConst, const detail::node_t, detail::node_t>;
    using context_type = std::conditional_t<TIsConst, const ImNodeFlow, ImNodeFlow>;

    /**
     * @brief NodeRef's ctor
     * @param internal_node Reference to the referred to internal node
     */
    explicit NodeRefImpl(context_type& ctx, node_type& internal_node);

    /**
     * @brief Main loop of the node
     * @details Updates position, hovering and selected status, and renders the node. Must be called each frame.
     */
    void update()
        requires(!TIsConst);

    /**
     * @brief Add an Input to the node
     * @details Will add an Input pin to the node with the given name and data type.
     * In this case the name of the pin will also be its UID.
     *The UID must be unique only in the context of the current node's inputs.

     * @tparam T Type of the data the pin will handle
     * @param pin_info User given options to the InPin during construction
     * @return Reference to the newly added pin. Do not save this
     * reference. Use the reference to query the pin's UID and save that
     * instead.
     */
    template <typename T>
    InPinRef<T, TIsConst> addIN(InPinInfo<T> pin_info)
        requires(!TIsConst);

    /**
     * @brief Remove input pin
     * @param uid Unique identifier of the pin
     */
    void dropIN(const PinUID& uid)
        requires(!TIsConst);

    /**
     * @brief Remove all input pin's with the associated name
     * @param name Pin's name filter
     */
    void dropIN(std::string_view name)
        requires(!TIsConst);

    /**
     * @brief Show a temporary input pin
     * @details Will show an input pin with the given name.
     * The pin is created the first time showIN is called and kept alive as long as showIN is called each frame.
     * In this case the name of the pin will also be its UID.
     * The UID must be unique only in the context of the current node's inputs.
     *
     * @tparam T Type of the data the pin will handle
     * @param pin_info User given options to the InPin during construction
     * @return Const reference to the value of the connected link for the current frame of defReturn
     */
    template <typename T>
    const T& showIN(InPinInfo<T> pin_info)
        requires(!TIsConst);

    /**
     * @brief Add an Output to the node
     * @details Must be called in the node constructor. WIll add an Output pin to the node with the given name and data type.
     * In this case the name of the pin will also be its UID.
     * The UID must be unique only in the context of the current node's outputs.
     *
     * @tparam T Type of the data the pin will handle
     * @param pin_info User given options to the InPin during construction
     * @return Reference to the newly added pin. Do not save this
     * reference. Use the reference to query the pin's UID and save that
     * instead.
     */
    template <typename T>
    OutPinRef<T, TIsConst> addOUT(OutPinInfo<T> pin_info)
        requires(!TIsConst);

    /**
     * @brief Remove output pin
     * @param uid Unique identifier of the pin
     */
    void dropOUT(const PinUID& uid)
        requires(!TIsConst);

    /**
     * @brief Remove all output pins with a matching name
     * @param name The name to filter for
     */
    void dropOUT(std::string_view name)
        requires(!TIsConst);

    /**
     * @brief Show a temporary output pin
     * @details Will show an output pin with the given name.
     * The pin is created the first time showOUT is called and kept alive as long as showOUT is called each frame.
     * In this case the name of the pin will also be its UID.
     * The UID must be unique only in the context of the current node's outputs.
     *
     * @tparam T Type of the data the pin will handle
     * @param pin_info User given options to the InPin during construction
     */
    template <typename T>
    void showOUT(OutPinInfo<T> pin_info)
        requires(!TIsConst);

    /**
     * @brief Get generic reference to input pin
     * @param uid Unique identifier of the pin
     * @return Pointer to the pin, if it exists
     */
    std::optional<PinRef<TIsConst>> inPin(const PinUID& uid);

    /**
     * @brief Get generic reference to input pin with the specified name
     * @param uid Unique identifier of the pin
     * @return Pointer to the pin, if it exists
     */
    std::optional<PinRef<TIsConst>> inPin(std::string_view name);

    std::optional<PinRef<TIsConst>> tempInPin(const PinUID& uid);

    std::optional<PinRef<TIsConst>> tempInPin(std::string_view name);

    /**
     * @brief Get generic reference to output pin
     * @param uid Unique identifier of the pin
     * @return Pointer to the pin, if it exists
     */
    std::optional<PinRef<TIsConst>> outPin(const PinUID& uid);

    /**
     * @brief Get generic reference to output pin of the same name
     * @param name Unique identifier of the pin
     * @return Pointer to the pin, if it exists
     */
    std::optional<PinRef<TIsConst>> outPin(std::string_view name);

    std::optional<PinRef<TIsConst>> tempOutPin(const PinUID& uid);

    std::optional<PinRef<TIsConst>> tempOutPin(std::string_view name);

    /**
     * @brief Get generic reference to input pin
     * @tparam T Type of the data the pin will handle
     * @param uid Unique identifier of the pin
     * @return Pointer to the pin, if it exists
     */
    template <typename T>
    std::optional<InPinRef<T, TIsConst>>
    inPin(const PinUID& uid);

    /**
     * @brief Get generic reference to input pin with the specified name
     * @tparam T Type of the data the pin will handle
     * @param uid Unique identifier of the pin
     * @return Pointer to the pin, if it exists
     */
    template <typename T>
    std::optional<InPinRef<T, TIsConst>>
    inPin(std::string_view name);

    /**
     * @brief Get generic reference to output pin
     * @tparam T Type of the data the pin will handle
     * @param uid Unique identifier of the pin
     * @return Pointer to the pin, if it exists
     */
    template <typename T>
    std::optional<OutPinRef<T, TIsConst>>
    outPin(const PinUID& uid);

    /**
     * @brief Get generic reference to output pin of the same name
     * @tparam T Type of the data the pin will handle
     * @param name Unique identifier of the pin
     * @return Pointer to the pin, if it exists
     */
    template <typename T>
    std::optional<OutPinRef<T, TIsConst>>
    outPin(std::string_view name);

    /**
     * @brief Delete itself
     */
    void destroy()
        requires(!TIsConst);

    /*
     * @brief Get if node must be deleted
     */
    [[nodiscard]] bool toDestroy() const;

    /**
     * @brief Get hovered status
     * @return [TRUE] if the mouse is hovering the node
     */
    bool isHovered() const;

    /**
     * @brief Get node's UID
     * @return NodeRef's unique identifier
     */
    [[nodiscard]] NodeUID getUID() const;

    /**
     * @brief Get node name
     * @return Const reference to the node's name
     */
    const std::string& getName() const;

    /**
     * @brief Get node size
     * @return Const reference to the node's size
     */
    const ImVec2& getSize() const;

    /**
     * @brief Get node size
     * @return Const reference to the node's size
     */
    const ImVec2& getFullSize() const;

    /**
     * @brief Get node position
     * @return Const reference to the node's position
     */
    const ImVec2& getPos() const;

    /**
     * @brief Get node position
     * @return Const reference to the node's position
     */
    ImVec2 getTLPos() const;

    /**
     * @brief Get node's style
     * @return Shared pointer to the node's style
     */
    const NodeStyle& getStyle() const;

    [[nodiscard]] bool isActivated() const;

    [[nodiscard]] std::optional<NodeGroupArenaRef<TIsConst>> getChildArena() const;

    [[nodiscard]] bool anyChildActivated() const;

    [[nodiscard]] bool hasChildNodes() const;

    /**
     * @brief Get selected status
     * @return [TRUE] if the node is selected
     */
    [[nodiscard]] bool isSelected() const;

    /**
     * @brief Get Indestructable status
     * @return [TRUE] if the node is Indestructable
     */
    [[nodiscard]] bool isIndestructable() const;

    /**
     * @brief Set node's uid
     * @param uid Node's unique identifier
     */
    NodeRefImpl& setUID(NodeUID uid)
        requires(!TIsConst);

    /**
     * @brief Set node's name
     * @param name New title
     */
    NodeRefImpl& setTitle(const std::string& title)
        requires(!TIsConst);

    /**
     * @brief Set node's position
     * @param pos Position in grid coordinates
     */
    NodeRefImpl& setPos(const ImVec2& pos)
        requires(!TIsConst);

    /**
     * @brief Set node's style
     * @param style New style
     */
    NodeRefImpl& setStyle(NodeStyle style)
        requires(!TIsConst);

    /**
     * @brief Set node's style
     * @param style New style
     */
    NodeRefImpl& setIndestructable(bool indestruct)
        requires(!TIsConst);

    /**
     * @brief Set selected status
     * @param state New selected state
     *
     * Status only updates when prepareNextFrame() is called
     */
    NodeRefImpl& setSelected(bool state)
        requires(!TIsConst);

    /**
     * @brief Set provisionally selected status
     * @param state New provisionally selected state
     *
     * Status only updates when prepareNextFrame() is called
     */
    NodeRefImpl& setProvisionallySelected(bool state)
        requires(!TIsConst);

    std::optional<NodeRefImpl<TIsConst>> getParentNode() const;

    void detachFromParent()
        requires(!TIsConst);

    void setParentNode(std::optional<NodeRef> new_parent_node)
        requires(!TIsConst);

    void disbandGroup()
        requires(!TIsConst);

    /**
     * @brief Update the isSelected status of the node
     */
    void prepareNextFrame()
        requires(!TIsConst);

    /**
     * @brief Have every pin calculate their next output value
     */
    void calculate_inputs()
        requires(!TIsConst);

    /**
     * @brief Have every pin calculate their next output value
     */
    void calculate_outputs()
        requires(!TIsConst);

    /**
     * @brief Have every pin commit their calculated values as their new
     * output value
     */
    void prime_inputs()
        requires(!TIsConst);

    /**
     * @brief Have every pin commit their calculated values as their new
     * output value
     */
    void prime_outputs()
        requires(!TIsConst);

    operator NodeRefImpl<true>() const
    {
        return NodeRefImpl<true> { m_ctx, m_node };
    }

    /**
     * FIXME: Remove internal methods from public API
     * @internal For internal use only
     */
    operator node_type&()
    {
        return m_node.get();
    }

    [[nodiscard]] constexpr bool operator==(const NodeRefImpl& other) const noexcept
    {
        return &m_node.get() == &other.m_node.get();
    }

private:
    PinRef<TIsConst>
    addIN_impl(BasePinInfo info, std::unique_ptr<detail::IPin> pin_impl)
        requires(!TIsConst);

    PinRef<TIsConst>
    showIN_impl(BasePinInfo info, std::unique_ptr<detail::IPin> pin_impl)
        requires(!TIsConst);

    PinRef<TIsConst>
    addOUT_impl(BasePinInfo info, std::unique_ptr<detail::IPin> pin_impl)
        requires(!TIsConst);

    PinRef<TIsConst>
    showOUT_impl(BasePinInfo info, std::unique_ptr<detail::IPin> pin_impl)
        requires(!TIsConst);

    std::reference_wrapper<context_type> m_ctx;
    std::reference_wrapper<node_type> m_node;
};

extern template class NodeRefImpl<false>;
extern template class NodeRefImpl<true>;

} // namespace ImFlow

#include "./NodeRef.inl"

#endif // IMNODEFLOW_NODEREF_H
