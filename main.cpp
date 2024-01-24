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
        addIN<int>("intouno", 0);
        addIN<int>("intoduo", 0);
        addOUT<int>("outoa");
        addOUT<int>("outob");
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

class CD : public BaseNode
{
public:
    explicit CD(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf)
    {
        addOUT<std::string>("str_out");
    }

    void draw() override
    {
        ImGui::Text("String Spitter");
        ImGui::InputText("##ToSpit", &m_ss);
    }

    void resolve(uintptr_t me) override
    {
        if (me == outs(0).me())
        {
            outs<std::string>(0) << m_ss.c_str();
        }
    }
private:
    std::string m_ss;
};

class Pri : public BaseNode
{
public:
    explicit Pri(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf)
    {
        addIN<int>("inn", 0);
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
        addIN<std::string>("Str", "Not Connected");
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

// TODO: right-click pop-up callback
// TODO: Allow only link between same data type

void foo()
{
    printf_s("DROPPED ME!!\n");
}

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

    bool done = false;
    while (!done)
    {
        IGH.loop(&done);
    }

    IGH.end();
    return 0;
}
