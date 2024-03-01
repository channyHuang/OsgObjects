#pragma once

#include "commonOsg/OsgImguiHandler.h"

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
    const int MAXN_FILE_LEN = 1024;
};
