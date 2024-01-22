#include <iostream>
#include <cmath>
#include <vector>
#include <ImGuiHandler.h>
#include <ImNodeFlow.h>

using namespace ImFlow;

ImNodeFlow INF("f");

class AB : public BaseNode
{
public:
    explicit AB(const char* name, ImVec2 pos, InfInterface* inf) : BaseNode(name, pos, inf)
    {
        addIN<int>("intouno", this);
        addIN<int>("intoduo", this);
        addOUT<int>("outoa", this);
        addOUT<int>("outob", this);
    }

    void draw() override
    {
        ImGui::Text("ASDDJHGFDSA");
        ImGui::Combo("Test", &m_slider, "Ciao\0come\0stai\0\0");
    }

    void resolve(uintptr_t me) override
    {
        if (me == outs(0).me())
        {
            outs<int>(0) << ins<int>(0).val() + m_slider;
        }
        else if (me == outs(1).me())
        {
            outs<int>(1) << ins<int>(0).val() * m_slider;
        }
    }
private:
    int m_slider = 0;
};

class Pri : public BaseNode
{
public:
    explicit Pri(const char* name, ImVec2 pos, InfInterface* inf) : BaseNode(name, pos, inf)
    {
        addIN<int>("inn", this);
    }

    void draw() override
    {
        ImGui::Text("%d", ins<int>(0).val());
    }
private:
};

class DemoWindow : public appLayer
{
public:
    void update() override
    {
        ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
        ImGui::Begin("FlowGrid Test");
        INF.update();
        ImGui::End();
    }
};

// TODO: Handle link drop-off, link creation, post drop-off link rendering

int main()
{
    if (!IGH.init("Example", 1300, 800))
        return 1;

    ImGui::GetIO().IniFilename = nullptr;

    IGH.pushLayer<DemoWindow>();
    IGH.setActiveWin(0);

    INF.addNode<AB>("ABABABA", ImVec2(0, 0));
    INF.addNode<Pri>("Printer ONE", ImVec2(100, 0));

    bool done = false;
    while (!done)
    {
        IGH.loop(&done);
    }

    IGH.end();
    return 0;
}
