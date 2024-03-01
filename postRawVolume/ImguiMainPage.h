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
    void load(std::string sFile);

private:
    osg::ref_ptr<osgViewer::Viewer> pviewer = nullptr;
    char* cFileName;
    const int nMaxFileNameLength = 128;

    float thresMin = 0, thresMax = 0;
    int cascade = 0;
};
