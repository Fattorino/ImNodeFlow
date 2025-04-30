/**
 * This is a minimal implementation of the Simple Sum example
 */
#pragma once

#include "ImNodeFlow.h"

/* The simple sum basic node */
class SimpleSum : public ImFlow::BaseNode
{

public:
    SimpleSum()
    {
        setTitle("Simple sum");
        setStyle(ImFlow::NodeStyle::green());
        ImFlow::BaseNode::addIN<int>("In", 0, ImFlow::ConnectionFilter::SameType());
        ImFlow::BaseNode::addOUT<int>("Out", nullptr)->behaviour([this](){ return getInVal<int>("In") + m_valB; });
    }

    void draw() override
    {
        if(ImFlow::BaseNode::isSelected()) {
          ImGui::SetNextItemWidth(100.f);
          ImGui::InputInt("##ValB", &m_valB);
          ImGui::Button("Hello");
        }
    }

private:
    int m_valB = 0;
};

/* Node editor that sets up the grid to place nodes */
struct NodeEditor : ImFlow::BaseNode
{
    ImFlow::ImNodeFlow mINF;
    NodeEditor(float d, std::size_t r)
    : BaseNode()
    {
        mINF.setSize({d,d});
        if(r > 0) {
          mINF.addNode<SimpleSum>({0,0});
          mINF.addNode<SimpleSum>({10,10});
        }
    }

    void set_size(ImVec2 d)
    {
        mINF.setSize(d);
    }

    void draw() override
    {
        mINF.update();
    }
};

/* Create a node editor with width and height */
NodeEditor neditor(500, 1500);
