#pragma once

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/ref_ptr>

#include "commonOsg/OsgImguiHandler.h"
#include "commonOsg/osgPickHandler.h"
#include "commonOsg/osgCameraHandler.h"

class ImguiMainPage : public OsgImGuiHandler {
public:
    ImguiMainPage();
    ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandler> pCameraHandler);
    ~ImguiMainPage();

protected:
    void drawUi();

private:
    char* cFileName;
    const int nMaxFileNameLength = 128;
    osg::ref_ptr<osgViewer::Viewer> pviewer = nullptr;
    osg::ref_ptr< CameraHandler> m_pCameraHandler = nullptr;

    bool bShowImage = false;
    int nFunIdx = -1;
    std::vector<std::string> vMenuStr = { "fun1", "fun2", "fun3" };
    std::vector<ImVec4> vMenuColors = { ImVec4(1.f, 0.f, 0.f, 1.f), ImVec4(1.f, 0.f, 0.f, 1.f), ImVec4(1.f, 0.f, 0.f, 1.f) };

    PickHandler* m_pPicker = nullptr;
};
