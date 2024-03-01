#pragma once

#include "nativefiledialog/nfd.h"

#include "OsgImguiHandler.h"
#include "commonFunc/signalSlots.h"

#include <osgViewer/Viewer>
#include <osg/ref_ptr>

#include "osgManager.h"


class ImguiMainPage : public OsgImGuiHandler {
public:
    ImguiMainPage();
    ImguiMainPage(osgViewer::Viewer& viewer);
    ~ImguiMainPage();

protected:
    void drawUi();

private:
    const int nMaxFileNameLength = 128;

    osg::ref_ptr<osgViewer::Viewer> pviewer = nullptr;
    clock_t startTime; // ms
    
    char* cFileName;

    float value[3] = { 0 };
    int nNumBalls = 1;
};
