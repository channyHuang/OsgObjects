#pragma once

#include "OsgImguiHandler.h"

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/ref_ptr>

class ImguiMainPage : public OsgImGuiHandler {
public:
    ImguiMainPage();
    ImguiMainPage(osgViewer::Viewer& viewer);
    ~ImguiMainPage();

protected:
    void drawUi();

private:
    osg::ref_ptr<osgViewer::Viewer> pviewer = nullptr;

    char* cFileName;
    const int nMaxFileNameLength = 128;
    bool bShowImage = false;
    int nFunIdx = -1;
    std::vector<std::string> vMenuStr = { "fun1", "fun2", "fun3" };
    std::vector<ImVec4> vMenuColors = { ImVec4(1.f, 0.f, 0.f, 1.f), ImVec4(1.f, 0.f, 0.f, 1.f), ImVec4(1.f, 0.f, 0.f, 1.f) };
};
