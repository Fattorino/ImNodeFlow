#pragma once

#include "ImNodeFlow.h"

class SimpleSum : public ImFlow::BaseNode {
public:
    SimpleSum() {
        setTitle("Simple sum");
        setStyle(ImFlow::NodeStyle::green());
        ImFlow::BaseNode::addIN<int>("In", 0, ImFlow::ConnectionFilter::SameType());
        ImFlow::BaseNode::addOUT<int>("Out", nullptr)->behaviour([this]() { return getInVal<int>("In") + m_valB; });
    }

    void draw() override {
        ImGui::SetNextItemWidth(100.f);
        ImGui::InputInt("##ValB", &m_valB);
    }

private:
    int m_valB = 0;
};

class CollapsingNode : public ImFlow::BaseNode {
public:
    CollapsingNode() {
        setTitle("Collapsing node");
        setStyle(ImFlow::NodeStyle::red());
        ImFlow::BaseNode::addIN<int>("A", 0, ImFlow::ConnectionFilter::SameType());
        ImFlow::BaseNode::addIN<int>("B", 0, ImFlow::ConnectionFilter::SameType());
        ImFlow::BaseNode::addOUT<int>("Out", nullptr)->behaviour([this]() { return getInVal<int>("A") + getInVal<int>("B"); });
    }

    void draw() override {
        if(ImFlow::BaseNode::isSelected()) {
            ImGui::SetNextItemWidth(100.f);
            ImGui::Text("You can only see me when the node is selected!");
        }
    }

};

class ResultNode : public ImFlow::BaseNode {
public:
    ResultNode() {
        setTitle("Result node");
        setStyle(ImFlow::NodeStyle::brown());
        ImFlow::BaseNode::addIN<int>("A", 0, ImFlow::ConnectionFilter::SameType());
        ImFlow::BaseNode::addIN<int>("B", 0, ImFlow::ConnectionFilter::SameType());
    }

    void draw() override {
        ImGui::Text("Result: %d", getInVal<int>("A") + getInVal<int>("B"));
    }

};


/* Node editor that sets up the grid to place nodes */
struct NodeEditor : ImFlow::BaseNode {
    ImFlow::ImNodeFlow mINF;

    NodeEditor(float d, std::size_t r) : BaseNode() {
        mINF.setSize({d, d});
        if (r > 0) {
            auto n1 = mINF.addNode<SimpleSum>({40, 40});
            auto n2 = mINF.addNode<SimpleSum>({40, 150});
            auto result = mINF.addNode<ResultNode>({250, 80});

            // Add links between nodes
            n1->outPin("Out")->createLink(result->inPin("A"));
            n2->outPin("Out")->createLink(result->inPin("B"));


            // Add a collapsing node
            auto collapsingNode = mINF.addNode<CollapsingNode>({300, 300});
            
        }
    }

    void set_size(ImVec2 d) {
        mINF.setSize(d);
    }

    void draw() override {
        mINF.update();
    }
};

