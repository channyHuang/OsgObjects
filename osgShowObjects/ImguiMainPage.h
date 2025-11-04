#pragma once

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/ref_ptr>

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
    char* cFileName;
    char* cTexturePath = "./";
    osg::ref_ptr<osgViewer::Viewer> pviewer = nullptr;
    osg::ref_ptr< CameraHandler> m_pCameraHandler = nullptr;
    std::string sFileName = "../data/ply_ext/scene_dense_mesh_03-02-01.ply";
    const int nMaxFileNameLength = 128;

    PickHandler* m_pPicker = nullptr;
};
