#include "ImguiMainPage.h"

#include "osgManager.h"


#include "commonOsg/commonOsg.h"
#include "nativefiledialog/nfd.h"

void pickCbFunc(const osg::Vec3& vPos, void* pUser) {
	OsgManager::getInstance()->showPick(vPos);
}

ImguiMainPage::ImguiMainPage() {
    
}

ImguiMainPage::ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandler> pCameraHandler) {
    pviewer = &viewer;
    m_pCameraHandler = pCameraHandler;
    OsgManager::getInstance()->setViewer(viewer);
    cFileName = new char[nMaxFileNameLength];
    memset(cFileName, 0, nMaxFileNameLength);
    cTexturePath = new char[nMaxFileNameLength];
    memset(cTexturePath, 0, nMaxFileNameLength);

    m_pPicker = new PickHandler();
    m_pPicker->setCallback(pickCbFunc, nullptr);
    viewer.addEventHandler(m_pPicker);
}

ImguiMainPage::~ImguiMainPage() {
    pviewer = nullptr;
    if (cFileName != nullptr) {
        delete[]cFileName;
    }
    if (cTexturePath != nullptr) {
        delete[]cTexturePath;
    }
}

void ImguiMainPage::drawUi() {
    ImGui::Begin("recon frame");
    if (ImGui::Button("Switch Scene")) {
        OsgManager::getInstance()->switchScene();
    }
    // select file or folder 文件选择框
    if (ImGui::InputTextWithHint("file", "<.obj .ply .xyz>", cFileName, nMaxFileNameLength, ImGuiInputTextFlags_EnterReturnsTrue)) {
    }
    ImGui::SameLine();
    if (ImGui::Button("Open File")) {
        nfdresult_t result = NFD_OpenDialog(""/*"obj,ply,xyz,csv"*/, nullptr, &cFileName);
        if (result == NFD_OKAY) {

        }
    }
    if (ImGui::Button("Reset Scene")) {
        m_pCameraHandler->reset();
    }
    ImGui::Checkbox("Back Scene To World Center", &m_pCameraHandler->m_bBack2WorldCenter);
    if (m_pCameraHandler->m_bBack2WorldCenter) {
        m_pCameraHandler->back2WorldCenter();
        m_pCameraHandler->m_bBack2WorldCenter = false;
    }
    ImGui::SliderFloat("step", &m_pCameraHandler->m_fStepScale, 0.01f, 3.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("recon frame"))
        {
            if (ImGui::Button("recon one frame only")) {
                OsgManager::getInstance()->reconFrameDataset();
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
