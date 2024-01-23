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
    explicit AB(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf)
    {
        addIN<int>("intouno", this);
        addIN<int>("intoduo", this);
        addOUT<int>("outoa", this);
        addOUT<int>("outob", this);
    }

    void draw() override
    {
        ImGui::Text("ASDDJHGFDSA");
        ImGui::SliderInt("##SLSLSL", &m_slider, 0, 200);
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
    explicit Pri(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf)
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

        ImGui::Begin("S");
        static int stt = 0;
        if (ImGui::Combo("##ffff", &stt, "Classic\0Dark\0Light\0Brown\0Foggy\0\0"))
            IGH.setStyle(static_cast<IGH_Style>(stt));
        ImGui::End();
    }
};

// FIXME: dragAllowed() needs same fix as idLinking() because I block other Nodes from dragging out a link
// FIXME: data out only flows to last Link instead of all
// TODO: Empty Link drop-off callback, right-click pop-up callback

int main()
{
    if (!IGH.init("Example", 1300, 800))
        return 1;

    ImGui::GetIO().IniFilename = nullptr;

    IGH.pushLayer<DemoWindow>();
    IGH.setActiveWin(0);

    ImGui::GetStyle().ItemSpacing;

    INF.addNode<AB>("AA", ImVec2(0, 0));
    INF.addNode<AB>("BB", ImVec2(0, 100));
    INF.addNode<Pri>("Printer ONE", ImVec2(500, 0));
    INF.addNode<Pri>("Printer TWO", ImVec2(500, 100));
    INF.addNode<Pri>("Printer THREE", ImVec2(500, 200));

    bool done = false;
    while (!done)
    {
        IGH.loop(&done);
    }

    IGH.end();
    return 0;
}
