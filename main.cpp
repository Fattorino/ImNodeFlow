#include <iostream>
#include <cmath>
#include <vector>
#include <ImGuiHandler.h>
#include <ImNodeFlow.h>
#include <imgui_stdlib.h>

using namespace ImFlow;

ImNodeFlow INF("f");

class AB : public BaseNode
{
public:
    explicit AB(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf)
    {
        addIN<int>("intouno", 0, ConnectionFilter_Int);
        addOUT<int>("int", ConnectionFilter_Int);

        outs<int>(0).behaviour([this](){ return ins<int>(0).val() + m_slider; });
    }

    void draw() override
    {
        ImGui::Text("ASDDJHGFDSA");
        ImGui::PushItemWidth(120.0f);
        ImGui::SliderInt("##SLSLSL", &m_slider, 0, 200);
    }
private:
    int m_slider = 0;
};

class CD : public BaseNode
{
public:
    explicit CD(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf)
    {
        addOUT<std::string>("str_out", ConnectionFilter_String);
        outs<std::string>(0).behaviour([this](){ return m_ss; });
    }

    void draw() override
    {
        ImGui::Text("String Spitter");
        ImGui::PushItemWidth(120.0f);
        ImGui::InputText("##ToSpit", &m_ss);
    }
private:
    std::string m_ss;
};

class Pri : public BaseNode
{
public:
    explicit Pri(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf)
    {
        addIN<int>("INT", 0, ConnectionFilter_Int);
        addIN<int>("FLOAT", 0, ConnectionFilter_Float);
        addIN<int>("NUMBERS", 0, ConnectionFilter_Numbers);
        addIN<int>("ANY", 0);
    }

    void draw() override
    {
        ImGui::Text("%d", ins<int>(0).val());
    }
private:
};

class StrPri : public BaseNode
{
public:
    explicit StrPri(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf)
    {
        addIN<std::string>("Str", "Not Connected", ConnectionFilter_String);
    }

    void draw() override
    {
        ImGui::Text("%s", ins<std::string>(0).val().c_str());
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

void foo()
{
    printf_s("DROPPED ME!!\n");
}
void loo()
{
    printf_s("RIGHT CLICK!!\n");
}

// FIXME: I can scroll down if nodes go under the bottom
// TODO: Make it so you only select the thing on top and not also everything that might be underneath
// TODO: Optimize code by removing some loops over Links list (by for example keeping a second list of the selected ones so I dont have to search the full list every frame

int main()
{
    if (!IGH.init("Example", 1300, 800))
        return 1;

    ImGui::GetIO().IniFilename = nullptr;

    IGH.pushLayer<DemoWindow>();
    IGH.setActiveWin(0);

    INF.addNode<AB>("AA", ImVec2(0, 0));
    INF.addNode<AB>("BB", ImVec2(0, 100));
    INF.addNode<Pri>("Printer ONE", ImVec2(500, 0));
    INF.addNode<Pri>("Printer TWO", ImVec2(500, 100));
    INF.addNode<Pri>("Printer THREE", ImVec2(500, 200));

    INF.addNode<CD>("CC", ImVec2(0, 100));
    INF.addNode<StrPri>("String Printer", ImVec2(500, 300));

    INF.setDroppedLinkCallback(foo);
    INF.setRightClickCallback(loo);

    bool done = false;
    while (!done)
    {
        IGH.loop(&done);
    }

    IGH.end();
    return 0;
}
