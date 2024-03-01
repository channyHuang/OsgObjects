#include "ImguiMainPage.h"

#include "osgManager.h"
#include "parseBag.h"

GLuint textureID;

ImguiMainPage::ImguiMainPage() {
    
}

ImguiMainPage::ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandler> pCameraHandler) {
    pviewer = &viewer;
    m_pCameraHandler = pCameraHandler;
    OsgManager::getInstance()->setViewer(viewer);
}

ImguiMainPage::~ImguiMainPage() {
    pviewer = nullptr;
}

void ImguiMainPage::drawUi() {
    ImGui::Begin("osg parse bag");
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
        if (ImGui::BeginTabItem("parse bag"))
        {
            //std::string fileName = "D:/dataset/lab/c2_lvi/20230630-obs-lvi.bag";
            //std::string fileName = "D:/dataset/lab/c2_lvi/20230607lvi.bag";
            std::string fileName = "D:/dataset/lab/c2_lvi/20230630-object-lvi.bag";

            if (ImGui::Button("parse header")) {
                ParseBag::getInstance()->parseBag(fileName);
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
