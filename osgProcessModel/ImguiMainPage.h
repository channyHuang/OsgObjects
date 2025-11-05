#pragma once

#include <osgViewer/Viewer>
#include <osg/ref_ptr>

#include "nativefiledialog/nfd.h"
#include "commonFunc/signalSlots.h"
#include "osgManager.h"

#include "commonOsg/OsgImguiHandler.h"
#include "commonOsg/osgPickHandler.h"
#include "commonOsg/osgCameraHandler.h"
#include "osgManager.h"


class ImguiMainPage : public OsgImGuiHandler {
public:
    ImguiMainPage();
    ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandler> pCameraHandler);
    ~ImguiMainPage();

protected:
    void drawUi();

private:
    osg::ref_ptr<osgViewer::Viewer> pviewer = nullptr;
    osg::ref_ptr< CameraHandler> m_pCameraHandler = nullptr;
    PickHandler* m_pPicker = nullptr;


    const int nMaxFileNameLength = 128;
    clock_t startTime; // ms
    
    char* cFileName;

    float value[3] = { 0 };
    int nNumBalls = 1;
};
