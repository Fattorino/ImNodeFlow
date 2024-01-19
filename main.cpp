#include <iostream>
#include <cmath>
#include <vector>
#include <ImGuiHandler.h>
#include <ImNodeFlow.h>

using namespace ImFlow;

ImNodeFlow inf;

class AB : public BaseNode
{
public:
    explicit AB(ImVec2 pos) : BaseNode(pos) {}

    void draw() override
    {
        ImGui::BeginGroup();
        in_a.draw();
        in_b.draw();
        ImGui::EndGroup();
        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::Text("ASDDJHGFDSA");
        ImGui::EndGroup();
        ImGui::SameLine();

        ImGui::BeginGroup();
        out_1.draw();
        out_2.draw();
        ImGui::EndGroup();
    }
    void resolve(uintptr_t me) override
    {
        if (me == out_1.me())
        {
            out_1 << in_a.val() + m_slider;
        }
        else if (me == out_2.me())
        {
            out_2 << in_b.val() * m_slider;
        }
    }
private:
    InPin<int> in_a = InPin<int>("ina");
    InPin<int> in_b = InPin<int>("inb");
    OutPin<int> out_1 = OutPin<int>("out1", this);
    OutPin<int> out_2 = OutPin<int>("out2", this);

    int m_slider = 2;
};

class Pri : public BaseNode
{
public:
    explicit Pri(ImVec2 pos) : BaseNode(pos) {}

    void draw() override
    {
        in.draw();
        ImGui::Text("%d", in.val());
    }
private:
    InPin<int> in = InPin<int>("in");
};

class DemoWindow : public appLayer
{
public:
    void update() override
    {
        ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
        ImGui::Begin("FlowGrid Test");
        inf.update();
        ImGui::End();
    }
};

// TODO: Implement Pin functionality and links and link creation and link drawing

int main()
{
    if (!IGH.init("Example", 1300, 800))
        return 1;

    ImGui::GetIO().IniFilename = nullptr;

    IGH.pushLayer<DemoWindow>();
    IGH.setActiveWin(0);

    inf.pushNode<AB>(ImVec2(0,0));
    inf.pushNode<Pri>(ImVec2(100,0));

    bool done = false;
    while (!done)
    {
        IGH.loop(&done);
    }

    IGH.end();
    return 0;
}
