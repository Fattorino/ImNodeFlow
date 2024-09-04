#pragma once

#include "ImNodeFlow.h"

using namespace ImFlow;

class SimpleSum : public BaseNode
{

public:
    SimpleSum()
    {
        setTitle("Simple sum");
        setStyle(NodeStyle::green());
        BaseNode::addIN<int>("In", 0, ConnectionFilter::SameType());
        BaseNode::addOUT<int>("Out", nullptr)->behaviour([this](){ return getInVal<int>("In") + m_valB; });
    }

    void draw() override
    {
        if(BaseNode::isSelected()) {
          ImGui::SetNextItemWidth(100.f);
          ImGui::InputInt("##ValB", &m_valB);
          ImGui::Button("Hello");
        }
    }

private:
    int m_valB = 0;
};

struct NodeEditor : ImFlow::BaseNode
{
    ImFlow::ImNodeFlow mINF;
    NodeEditor(float d, std::size_t r)
    : BaseNode()
    {
        setTitle("glhf");
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

NodeEditor neditor(500, 1500);

