#pragma once

#include <chrono>

#include <osgViewer/Viewer>
#include <osg/ref_ptr>

#include "commonFunc/signalSlots.h"
#include "nativefiledialog/nfd.h"
#include "commonOsg/OsgImguiHandler.h"
#include "commonOsg/osgPickHandler.h"
#include "commonOsg/osgCameraHandler.h"
#include "osgManager.h"

#include "generator/terraingenerator_roblox.h"

#include "CameraHandler.h"

class ImguiMainPage : public OsgImGuiHandler {
public:
    ImguiMainPage();
    // ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandler> pCameraHandler);
    ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandlerTerrain> pCameraHandler);
    ~ImguiMainPage();

protected:
    void drawUi();

// signals
    SignalSlot::Signal<void(Notification_Event)> notify;
    
private:
    const int nMaxFileNameLength = 128;
    bool m_bFirstLoad = true;
    std::vector<GLuint> vTextureID;
    std::map<std::string, GLuint> mapTextureID; 
    std::string sSelectedTex;

    osg::ref_ptr<osgViewer::Viewer> pViewer = nullptr;
    osg::ref_ptr< CameraHandlerTerrain> m_pCameraHandler = nullptr;
    PickHandler* m_pPicker = nullptr;
    
    std::chrono::time_point<std::chrono::steady_clock> stLastAutoUpdateTime; // ms
    
    Vector3 vCenter = Vector3(0);

    char* pFileName;

    float value[3] = { 0 };
    int nNumBalls = 1;

    TerrainGenerator_Roblox::BiomesParam m_stBiomeParams;
};
