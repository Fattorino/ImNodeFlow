#include "example_node_editor.hpp"

#include "ImNodeFlow.h"

#include "ImNodeFlow/detail/type_id.h"
#include "imgui.h"

#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <utility>

enum class UserNodeType : uint64_t {
    INVALID, // null
    FSM_STATE,
    FSM_EVENT,
    FSM_START,
    FSM_SINK,
};

class FSMStart : public ImFlow::INodeBehaviour {
public:
    void setup(ImFlow::NodeRef node) final
    {
        node
            .setTitle("FSMStart")
            .setStyle(ImFlow::NodeStyles::red)
            .setAllowInboundLinks(false)
            .setAllowOutboundLinks(true);
    }

    uintptr_t getUserData() final
    {
        return uintptr_t(UserNodeType::FSM_START);
    }

    void draw([[maybe_unused]] ImFlow::NodeRef node) final
    {
        if (ImGui::Button("Trigger")) {
            this->trigger(node);
        }
    }

    void trigger(ImFlow::NodeRef node);
};

class FSMSink : public ImFlow::INodeBehaviour {
public:
    void setup(ImFlow::NodeRef node) final
    {
        node
            .setTitle("FSMSink")
            .setStyle(ImFlow::NodeStyles::red)
            .setAllowInboundLinks(true)
            .setAllowOutboundLinks(false);
    }

    uintptr_t getUserData() final
    {
        return uintptr_t(UserNodeType::FSM_SINK);
    }

    void draw([[maybe_unused]] ImFlow::NodeRef node) final
    {
        ImGui::Text("Sink");
    }

    bool acceptInboundNodeConnection(ImFlow::NodeRef self, ImFlow::NodeRef inbound) final;
};

class FSMState final : public ImFlow::INodeBehaviour {
public:
    explicit FSMState(std::string name)
        : m_name(std::move(name))
    {
    }

    void setup(ImFlow::NodeRef node) final
    {
        node
            .setTitle("FSMState")
            .setStyle(ImFlow::NodeStyles::cyan)
            .setAllowInboundLinks(true)
            .setAllowOutboundLinks(true);
    }

    uintptr_t getUserData() final
    {
        return uintptr_t(UserNodeType::FSM_STATE);
    }

    void draw([[maybe_unused]] ImFlow::NodeRef node) final
    {
        ImGui::Text("Name: %s", m_name.c_str());

        if (isActive()) {
            node.setStyle(ImFlow::NodeStyles::green);
        } else {
            node.setStyle(ImFlow::NodeStyles::cyan);
        }

        m_active = m_activeNext;
    }

    bool acceptInboundNodeConnection(ImFlow::NodeRef self, ImFlow::NodeRef inbound) final;

    void activate() { m_activeNext = true; }
    void deactivate() { m_activeNext = false; }
    bool isActive() const { return m_active; }

private:
    std::string m_name;
    bool m_active = false;
    bool m_activeNext = false;
};

class FSMEvent final : public ImFlow::INodeBehaviour {
public:
    void setup(ImFlow::NodeRef node) final
    {
        node
            .setTitle("FSMEvent")
            .setStyle(ImFlow::NodeStyles::brown)
            .setAllowInboundLinks(true)
            .setAllowOutboundLinks(true);
    }

    void draw([[maybe_unused]] ImFlow::NodeRef node) final
    {
        if (ImGui::Button("Trigger")) {
            this->attemptTrigger(node);
        }
    }

    uintptr_t getUserData() final
    {
        return uintptr_t(UserNodeType::FSM_EVENT);
    }

    bool acceptInboundNodeConnection(ImFlow::NodeRef self, ImFlow::NodeRef inbound) final
    {
        auto* behavior = inbound.getBehavior();
        if (behavior == nullptr) {
            return false;
        }

        uintptr_t user_data = behavior->getUserData();
        return user_data == uintptr_t(UserNodeType::FSM_STATE)
            || user_data == uintptr_t(UserNodeType::FSM_START);
    }

    void attemptTrigger(ImFlow::NodeRef node)
    {
        bool event_will_trigger = false;

        for (auto inbound : node.getInboundConnections()) {
            auto* behavior = inbound.getBehavior();
            if (behavior == nullptr) {
                continue;
            }
            if (behavior->getUserData() == uintptr_t(UserNodeType::FSM_START)) {
                event_will_trigger = true;
                continue;
            }
            if (behavior->getUserData() != uintptr_t(UserNodeType::FSM_STATE)) {
                continue;
            }
            auto* fsm_state = static_cast<FSMState*>(behavior);
            if (fsm_state->isActive()) {
                event_will_trigger = true;
                fsm_state->deactivate();
            }
        }

        if (!event_will_trigger) {
            return;
        }

        for (auto outbound : node.getOutboundConnections()) {
            auto* behavior = outbound.getBehavior();
            if (behavior == nullptr) {
                continue;
            }
            if (behavior->getUserData() != uintptr_t(UserNodeType::FSM_STATE)) {
                continue;
            }
            auto* fsm_state = static_cast<FSMState*>(behavior);
            fsm_state->activate();
        }
    }
};

bool FSMSink::acceptInboundNodeConnection(ImFlow::NodeRef self, ImFlow::NodeRef inbound)
{
    auto* behavior = inbound.getBehavior();
    if (behavior == nullptr) {
        return false;
    }

    auto user_data = behavior->getUserData();

    if (user_data == uintptr_t(UserNodeType::FSM_EVENT)) {
        return true;
    }

    if (user_data != uintptr_t(UserNodeType::FSM_STATE)) {
        return false;
    }

    auto pos = (self.getTLPos() + inbound.getTLPos()) / 2.F;
    auto event_node = self.getContext().emplaceNode<FSMEvent>(pos);
    inbound.linkTo(event_node);
    event_node.linkTo(self);

    return false;
}

bool FSMState::acceptInboundNodeConnection(ImFlow::NodeRef self, ImFlow::NodeRef inbound)
{
    auto* behavior = inbound.getBehavior();
    if (behavior == nullptr) {
        return false;
    }

    auto user_data = behavior->getUserData();
    if (user_data == uintptr_t(UserNodeType::FSM_START)
        || user_data == uintptr_t(UserNodeType::FSM_EVENT)) {
        return true;
    }
    if (behavior->getUserData() != uintptr_t(UserNodeType::FSM_STATE)) {
        return false;
    }

    auto pos = (self.getTLPos() + inbound.getTLPos()) / 2.F;

    auto event_node = self.getContext().emplaceNode<FSMEvent>(pos);
    inbound.linkTo(event_node);
    event_node.linkTo(self);

    return false; // Reject direct connection
}

void FSMStart::trigger(ImFlow::NodeRef node)
{
    for (auto outbound : node.getOutboundConnections()) {
        auto* behavior = outbound.getBehavior();
        if (behavior == nullptr) {
            continue;
        }
        if (behavior->getUserData() != uintptr_t(UserNodeType::FSM_STATE)) {
            continue;
        }
        auto* fsm_state = static_cast<FSMState*>(behavior);
        fsm_state->activate();
    }
}

class NumberGenerator final : public ImFlow::INodeBehaviour {
public:
    void setup(ImFlow::NodeRef node) final
    {
        node.setTitle("Number Generator");
        node.setStyle(ImFlow::NodeStyles::cyan);

        node.addOUT<int>({
            .name = "Out",
            .behaviour = [this](ImFlow::NodeRef node) -> int {
                return this->m_val;
            },
        });
    }

    void draw([[maybe_unused]] ImFlow::NodeRef node) final
    {
        ImGui::SetNextItemWidth(100.f);
        ImGui::SliderInt("##number", &m_val, 0, 100);
    }

private:
    int m_val = 0;
};

class NumberDisplay final : public ImFlow::INodeBehaviour {
public:
    void setup(ImFlow::NodeRef node) final
    {
        node.setTitle("Number Display");
        node.setStyle(ImFlow::NodeStyles::cyan);
        node.addIN<int>({ .name = "In" });
    }

    void draw([[maybe_unused]] ImFlow::NodeRef node) final
    {
        if (node.inPin("In")->isConnected()) {
            ImGui::Text("Value: %d", node.inPin<int>("In")->value());
        } else {
            ImGui::Text("Disconnected");
        }
    }
};

class SimpleSum : public ImFlow::INodeBehaviour {
public:
    void setup(ImFlow::NodeRef node) override
    {
        node.setTitle("Simple sum");
        node.setStyle(ImFlow::NodeStyles::green);

        node.addIN<int>({ .name = "In", .default_value = 0 });
        node.addOUT<int>({
            .name = "Out",
            .behaviour = [this](ImFlow::NodeRef node) -> int {
                return node.inPin<int>("In")->value() + this->m_valB;
            },
        });
    }

    void draw(ImFlow::NodeRef node) override
    {
        ImGui::SetNextItemWidth(100.f);
        ImGui::InputInt("##ValB", &m_valB);
    }

private:
    int m_valB = 0;
};

class MultipleInputAdder : public ImFlow::INodeBehaviour {
public:
    void setup(ImFlow::NodeRef node) override
    {
        node.setTitle("Multiple input adder");
        node.setStyle(ImFlow::NodeStyles::cyan);
        node.addIN<int>({
            .name = "In",
            .strat = ImFlow::PinConnStrat::AllowMultipleConnections,
            .default_value = 0,
            .accumulate_op =
                [](bool first, int init, int new_input) -> int {
                return first ? new_input : init + new_input;
            },
        });
        node.addOUT<int>({
            .name = "Out",
            .behaviour = [](ImFlow::NodeRef node) -> int {
                return node.inPin<int>("In")->value();
            },
        });
    };

    void draw(ImFlow::NodeRef node) override
    {
        ImGui::SetNextItemWidth(100.F);
        ImGui::Text("Result: %d", node.inPin<int>("In")->value());
    }

    std::unique_ptr<ImFlow::INodeBehaviour> clone() override
    {
        return std::make_unique<MultipleInputAdder>();
    }
};

class CollapsingNode : public ImFlow::INodeBehaviour {
public:
    void setup(ImFlow::NodeRef node) override
    {
        node.setTitle("Collapsing node");
        node.setStyle(ImFlow::NodeStyles::red);

        node.addIN<int>({
            .name = "A",
            .style = ImFlow::PinStyles::red,
            .default_value = 0,
        });
        node.addIN<int>({
            .name = "B",
            .style = ImFlow::PinStyles::red,
            .default_value = 0,
        });

        node.addOUT<int>({
            .name = "Out",
            .style = ImFlow::PinStyles::red,
            .renderer = [](ImFlow::PinRef pin) -> void {
                ImGui::TextUnformatted("<OUTPIN>");
            },
            .behaviour = [this](ImFlow::NodeRef node) -> int {
                return node.inPin<int>("A")->value() + node.inPin<int>("B")->value();
            },
        });
    }

    void draw(ImFlow::NodeRef node) override
    {
        ImGui::TextUnformatted("Select me to see more");

        if (node.isSelected()) {
            ImGui::SetNextItemWidth(100.F);
            ImGui::Text(
                "You can only see me when the node is selected!");

            const char* multiplier_label = m_multiplier ? "On" : "Off";
            ImGui::SetNextItemWidth(100.F);
            if (ImGui::BeginCombo("Multiplier", multiplier_label)) {
                if (ImGui::Selectable("Off", !m_multiplier)) {
                    m_multiplier = false;
                }
                if (ImGui::Selectable("On", m_multiplier)) {
                    m_multiplier = true;
                }
                ImGui::EndCombo();
            }
        }

        if (m_multiplier) {
            node.showOUT<int>({
                .name = "Multiplier",
                .style = ImFlow::PinStyles::red,
                .behaviour = [](ImFlow::NodeRef node) -> int {
                    return node.inPin<int>("A")->value() * node.inPin<int>("B")->value();
                },
            });
        }
    }

private:
    bool m_multiplier = false;
};

class ResultNode : public ImFlow::INodeBehaviour {
public:
    void setup(ImFlow::NodeRef node) override
    {
        node.setTitle("Result node");
        node.setStyle(ImFlow::NodeStyles::brown);
        node.addIN<int>({ .name = "A", .default_value = 0 });
        node.addIN<int>({ .name = "B", .default_value = 0 });
    }

    void draw(ImFlow::NodeRef node) override
    {
        ImGui::Text(
            "Result: %d",
            node.inPin<int>("A")->value() + node.inPin<int>("B")->value());
    }
};

class NotGate : public ImFlow::INodeBehaviour {
public:
    void setup(ImFlow::NodeRef node) override
    {
        node.setTitle("Not");
        node.setStyle(ImFlow::NodeStyles::red);
        node.addIN<bool>({
            .name = "IN",
            .strat = ImFlow::PinConnStrat::AllowMultipleConnections,
            .default_value = false,
            .accumulate_op =
                [](bool first, bool init, bool new_input) -> bool {
                return first ? new_input : init | new_input;
            },
        });
        node.addOUT<bool>({
            .name = "OUT",
            .behaviour = [](ImFlow::NodeRef node) -> int {
                return !node.inPin<bool>("IN")->value();
            },
        });
    }

    void draw(ImFlow::NodeRef node) override
    {
        auto inPin = *node.inPin<bool>("IN");
        auto outPin = *node.outPin<bool>("OUT");
        if (inPin.value()) {
            inPin.setStyle(ImFlow::PinStyles::green);
            outPin.setStyle(ImFlow::PinStyles::red);
        } else {
            inPin.setStyle(ImFlow::PinStyles::red);
            outPin.setStyle(ImFlow::PinStyles::green);
        }
    }

    std::unique_ptr<ImFlow::INodeBehaviour> clone() override
    {
        return std::make_unique<NotGate>();
    }
};

NodeEditor::NodeEditor(float width, float height)
{
    mINF.setSize({ width, height });
    mINF.setSize({ width, height });
    mINF.rightClickPopUpContent([this](std::optional<ImFlow::NodeRef> node) -> void {
        if (node.has_value()) {
            auto selected_nodes = mINF.getSelectedNodes();
            uint64_t selected_nodes_except_active
                = selected_nodes.size()
                - (node->isSelected() ? 1 : 0);
            ImGui::Text(
                "you have right-clicked on [%s] (%zu total nodes selected)",
                node->getName().c_str(),
                selected_nodes.size());
            if (selected_nodes_except_active > 0) {
                if (ImGui::Button("Make parent")) {
                    for (auto& selected_node : selected_nodes) {
                        selected_node.setParentNode(*node);
                    }
                    ImGui::CloseCurrentPopup();
                }
            }
            if (node->getParentNode().has_value()) {
                if (ImGui::Button("Detach from parent")) {
                    node->detachFromParent();
                    ImGui::CloseCurrentPopup();
                }
            }
            if (node->hasChildNodes()) {
                if (ImGui::Button("Disband group")) {
                    node->disbandGroup();
                    ImGui::CloseCurrentPopup();
                }
            }
        } else {
            ImVec2 mouse_screen_pos = ImGui::GetMousePosOnOpeningCurrentPopup();
            if (ImGui::TreeNode("Math")) {
                if (ImGui::Button("Number Generator")) {
                    mINF.emplaceNodeAt<NumberGenerator>(mouse_screen_pos);
                    ImGui::CloseCurrentPopup();
                } else if (ImGui::Button("Number Display")) {
                    mINF.emplaceNodeAt<NumberDisplay>(mouse_screen_pos);
                    ImGui::CloseCurrentPopup();
                } else if (ImGui::Button("SimpleSum")) {
                    mINF.emplaceNodeAt<SimpleSum>(mouse_screen_pos);
                    ImGui::CloseCurrentPopup();
                } else if (ImGui::Button("ResultNode")) {
                    mINF.emplaceNodeAt<ResultNode>(mouse_screen_pos);
                    ImGui::CloseCurrentPopup();
                } else if (ImGui::Button("MultipleInputAdder")) {
                    mINF.emplaceNodeAt<MultipleInputAdder>(mouse_screen_pos);
                    ImGui::CloseCurrentPopup();
                } else if (ImGui::Button("CollapsingNode")) {
                    mINF.emplaceNodeAt<CollapsingNode>(mouse_screen_pos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Logic Gates")) {
                if (ImGui::Button("Not Gate")) {
                    mINF.emplaceNodeAt<NotGate>(mouse_screen_pos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("FSM")) {
                if (ImGui::Button("FSM Start")) {
                    mINF.emplaceNodeAt<FSMStart>(mouse_screen_pos);
                    ImGui::CloseCurrentPopup();
                } else if (ImGui::Button("FSM State")) {
                    mINF.emplaceNodeAt<FSMState>(mouse_screen_pos, "new state");
                    ImGui::CloseCurrentPopup();
                } else if (ImGui::Button("FSM Event")) {
                    mINF.emplaceNodeAt<FSMEvent>(mouse_screen_pos);
                    ImGui::CloseCurrentPopup();
                } else if (ImGui::Button("FSM Sink")) {
                    mINF.emplaceNodeAt<FSMSink>(mouse_screen_pos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::TreePop();
            }
        }
    });
    mINF.droppedLinkPopUpContent([this](ImFlow::PinRef pin) -> void {
        if (pin.getType() == ImFlow::PinType::Output
            && pin.getTypeID() == ImFlow::detail::get_type_id<int>()) {
            ImVec2 mouse_screen_pos = ImGui::GetMousePosOnOpeningCurrentPopup();
            if (ImGui::Button("Number Display")) {
                auto node = mINF.emplaceNodeAt<NumberDisplay>(mouse_screen_pos);
                pin.linkTo(*node.inPin("In"));
                ImGui::CloseCurrentPopup();
            }
        } else if (pin.getType() == ImFlow::PinType::Output
            && pin.getTypeID() == ImFlow::detail::get_type_id<bool>()) {
            ImVec2 mouse_screen_pos = ImGui::GetMousePosOnOpeningCurrentPopup();
            if (ImGui::Button("Not Gate")) {
                auto node = mINF.emplaceNodeAt<NotGate>(mouse_screen_pos);
                pin.linkTo(*node.inPin("IN"));
                ImGui::CloseCurrentPopup();
            }
        } else {
            ImGui::CloseCurrentPopup();
        }
    });

    auto n1 = mINF.emplaceNode<SimpleSum>({ 40, 40 });
    auto n2 = mINF.emplaceNode<SimpleSum>({ 40, 150 });
    auto result = mINF.emplaceNode<ResultNode>({ 340, 20 });
    auto multi_adder = mINF.emplaceNode<MultipleInputAdder>({ 360, 140 });

    result.setIndestructable(true);

    n1.setParentNode(result);
    n2.setParentNode(result);
    multi_adder.setParentNode(result);

    n1.linkTo(n2);

    // Add links between nodes
    n1.outPin("Out")->linkTo(*result.inPin("A"));
    n2.outPin("Out")->linkTo(*result.inPin("B"));

    n1.outPin("Out")->linkTo(*multi_adder.inPin("In"));
    n2.outPin("Out")->linkTo(*multi_adder.inPin("In"));

    // Add a collapsing node
    auto collapsingNode = mINF.emplaceNode<CollapsingNode>({ 330, 230 });

    n1.outPin("Out")->linkTo(*collapsingNode.inPin("A"));
    n2.outPin("Out")->linkTo(*collapsingNode.inPin("B"));

    auto fsm_state0 = mINF.emplaceNode<FSMState>({ 700, 100 }, "S0");
    auto fsm_state1 = mINF.emplaceNode<FSMState>({ 820, 100 }, "S1");
    auto fsm_state2 = mINF.emplaceNode<FSMState>({ 700, 180 }, "S2");
    auto fsm_state3 = mINF.emplaceNode<FSMState>({ 820, 180 }, "S3");
    auto fsm_state4 = mINF.emplaceNode<FSMState>({ 700, 250 }, "S4");

    // An example of how to make a digital circuit for a clock using only
    // not gates.
    // If this flashes too fast, the frame rate may be too high. Check you
    // have VSync enabled, or as a last resource, add more gates to slow
    // down the clock.
    std::vector<ImFlow::NodeRef> notGateChain;

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            notGateChain.emplace_back(mINF.emplaceNode<NotGate>({
                float(50 + x * 90),
                float(350 + y * 50),
            }));
        }
    }

    for (int i = 0; i < notGateChain.size(); i++) {
        auto& curr_gate = notGateChain.at(i);
        auto& next_gate = notGateChain.at((i + 1) % notGateChain.size());

        curr_gate.outPin("OUT")->linkTo(*next_gate.inPin("IN"));

        // This is required pre-processing to stabilize the clock. Because
        // each gate only has influence on the next gate on each iteration,
        // if they all start in their default state, they will all flip
        // into an ON state, and the next iteration, they will all flip
        // into an OFF state.
        //
        // So, when we create a connection between pins, we can
        // programmatically propagate the signal just to stabilize the
        // clock. This only needs to be done once, here.
        curr_gate.calculate_outputs();
        curr_gate.prime_outputs();
        next_gate.calculate_inputs();
        next_gate.prime_inputs();
    }
}

void NodeEditor::set_size(ImVec2 d) { mINF.setSize(d); }

void NodeEditor::draw() { mINF.update(); }
