#include "ImguiMainPage.h"

#include "osgManager.h"
#include "nativefiledialog/nfd.h"

#include "HG_GDAL.h"

void pickCbFunc(const osg::Vec3& vPos, void* pUser) {
	OsgManager::getInstance()->showPick(vPos);
}

ImguiMainPage::ImguiMainPage() {
    cFileName = new char[nMaxFileNameLength];
}

ImguiMainPage::ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandler> pCameraHandler) {
    pviewer = &viewer;
    m_pCameraHandler = pCameraHandler;
    OsgManager::getInstance()->setViewer(viewer);

    m_pPicker = new PickHandler();
    m_pPicker->setCallback(pickCbFunc, nullptr);
    viewer.addEventHandler(m_pPicker);

    cFileName = new char[nMaxFileNameLength];
    memset(cFileName, 0, nMaxFileNameLength);
}

ImguiMainPage::~ImguiMainPage() {
    pviewer = nullptr;
    if (cFileName != nullptr) {
        delete[]cFileName;
    }
}

void ImguiMainPage::drawUi() {
    ImGui::Begin("GDAL");
    
    if (ImGui::InputTextWithHint("file", "<.obj .ply .xyz>", cFileName, nMaxFileNameLength, ImGuiInputTextFlags_EnterReturnsTrue)) {}
    if (ImGui::Button("Open File")) {
        nfdresult_t result = NFD_OpenDialog("bin"/*"obj,ply,xyz,csv"*/, nullptr, &cFileName);
        if (result == NFD_OKAY) {
        }
    }

    if (ImGui::Button("Switch Scene")) {
        OsgManager::getInstance()->switchScene();
    }
    if (ImGui::Button("Reset Scene")) {
        m_pCameraHandler->reset();
    }

    ImGui::Checkbox("Back Scene To World Center", &m_pCameraHandler->m_bBack2WorldCenter);
    if (m_pCameraHandler->m_bBack2WorldCenter) {
        m_pCameraHandler->back2WorldCenter();
        m_pCameraHandler->m_bBack2WorldCenter = false;
    }

    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("GIS functions")) {
            if (ImGui::Button("translate geometric system")) {
                std::string sFileName = std::string(cFileName);
                if (sFileName.length() > 0) {
                    
                }
            }
            if (ImGui::Button("test gdal")) {
                HG_GDAL::convertModel("/home/channy/Documents/projects/OsgObjects/bin/tmp.osgb", "output_tiles");
            
                double val;
                OGRSpatialReference inRs,outRs;
                OGRCoordinateTransformation *poCT = OGRCreateCoordinateTransformation( &inRs, &outRs );
                HG_GDAL::Init(poCT, &val);
            }


            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}
