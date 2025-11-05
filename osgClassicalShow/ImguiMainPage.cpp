#include "ImguiMainPage.h"

#include "osgManager.h"

void pickCbFunc(const osg::Vec3& vPos, void* pUser) {
	OsgManager::getInstance()->showPick(vPos);
}

ImguiMainPage::ImguiMainPage() {
    
}

ImguiMainPage::ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandler> pCameraHandler) {
    pviewer = &viewer;
    m_pCameraHandler = pCameraHandler;
    OsgManager::getInstance()->setViewer(viewer);

    m_pPicker = new PickHandler();
    m_pPicker->setCallback(pickCbFunc, nullptr);
    viewer.addEventHandler(m_pPicker);
}

ImguiMainPage::~ImguiMainPage() {
    pviewer = nullptr;
}

void ImguiMainPage::drawUi() {
    ImGui::Begin("title");
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
    ImGui::SliderFloat("step", &m_pCameraHandler->m_fStepScale, 0.01f, 3.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

    if (ImGui::BeginTabBar("tabbar title", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("tabitem title"))
        {
            if (ImGui::Button("show box")) {
                OsgManager::getInstance()->showBox();
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
