#include <iostream>
#include <cmath>
#include <vector>
#include <ImGuiHandler.h>
#include <ImNodeFlow.h>
#include <imgui_stdlib.h>

using namespace ImFlow;

ImNodeFlow INF;

class AB : public BaseNode
{
public:
    explicit AB(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf)
    {
        addIN<int>("intouno", 0, ConnectionFilter_Int);
        addOUT<int>("int", ConnectionFilter_Int)
                ->behaviour([this](){ return ins<int>(0) + m_slider; });
        addOUT<int>("dummy", ConnectionFilter_Int)
                ->behaviour([this](){ return ins<int>(0) + m_slider * 2; });
    }

    void draw() override
    {
        ImGui::Text("ASDDJHGFDSA");
        ImGui::SetNextItemWidth(120.0f);
        ImGui::SliderInt("##SLSLSL", &m_slider, 0, 200);
    }
private:
    int m_slider = 0;
};

class Somma : public BaseNode
{
public:
    explicit Somma(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf)
    {
        addIN<int>("A", 0, ConnectionFilter_Int);
        addIN<int>("B", 0, ConnectionFilter_Int);
        addOUT<int>("C", ConnectionFilter_Int)
                ->behaviour([this](){ return ins<int>(0) + ins<int>(1); });
    }

    void draw() override
    {
        ImGui::Text("A + B = C");
    }
private:
};

class CD : public BaseNode
{
public:
    explicit CD(const std::string& name, ImVec2 pos, ImNodeFlow* inf) : BaseNode(name, pos, inf)
    {
        addOUT<std::string>("str_out", ConnectionFilter_String)
                ->behaviour([this](){ return m_ss; });
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
    }

    void draw() override
    {
        ImGui::Text("%d", ins<int>(0));
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
        ImGui::Text("%s", ins<std::string>(0).c_str());
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

// TODO: [POLISH] Collision solver to bring first node on foreground to avoid clipping
// TODO: [EXTRA]  Custom renderers for Pins (with lambdas I think)

void foo(Pin* dragged)
{
    if (dragged->kind() == PinKind_Output)
    {
        if (ImGui::Selectable("Sommatore"))
        {
            auto n = INF.dropNode<Somma>("Sommatore", ImGui::GetWindowPos());
            INF.createLink(dragged, n->ins(0));
        }
    }
    else
    {
        if (ImGui::Selectable("Sommatore"))
        {
            auto n = INF.dropNode<Somma>("Sommatore", ImGui::GetWindowPos());
            INF.createLink(n->outs(0), dragged);
        }
        if (ImGui::Selectable("AB thingy"))
        {
            auto n = INF.dropNode<AB>("AB", ImGui::GetWindowPos());
            INF.createLink(n->outs(0), dragged);
        }
    }
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

    INF.addNode<Somma>("Sommatore", ImVec2(100, 100));

    INF.addNode<CD>("CC", ImVec2(0, 100));
    INF.addNode<StrPri>("String Printer", ImVec2(500, 300));

    INF.rightClickPopUpContent([]() {
        if (ImGui::Selectable("AB"))
        {
            printf_s("AOOOO!\n");
        }
    });
    INF.droppedLinkPopUpContent(foo, ImGuiKey_LeftShift);

    bool done = false;
    while (!done)
    {
        IGH.loop(&done);
    }

    IGH.end();
    return 0;
}
