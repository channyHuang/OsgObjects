#include "ImguiMainPage.h"

#include "osgManager.h"

ImguiMainPage::ImguiMainPage() {
    
}

ImguiMainPage::ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandler> pCameraHandler) {
    pviewer = &viewer;
    m_pCameraHandler = pCameraHandler;
    OsgManager::getInstance()->setViewer(viewer);
    //cFileName = new char[nMaxFileNameLength];
    //memset(cFileName, 0, nMaxFileNameLength);

    picker = new PickHandler();
    viewer.addEventHandler(picker);
}

ImguiMainPage::~ImguiMainPage() {
    pviewer = nullptr;
}

void ImguiMainPage::drawUi() {
    ImGui::Begin("3d reconstruction");
    if (ImGui::Button("Switch Scene")) {
        OsgManager::getInstance()->switchScene();
    }
    ImGui::Checkbox("Rotate By Axis", &m_pCameraHandler->bRotateByAxis);
    if (m_pCameraHandler->bRotateByAxis) {
        ImGui::Text("Rotate by axis: 0-x, 1-y, 2-z");
        ImGui::SliderInt("Axis x y z", &m_pCameraHandler->axis, 0, 3);
    }
    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("test1"))
        {

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
