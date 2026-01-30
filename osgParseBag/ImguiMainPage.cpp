#include "ImguiMainPage.h"

#include "osgManager.h"
#include "parseBag.h"
#include "nativefiledialog/nfd.h"
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

    // if (ImGui::InputTextWithHint("file", "<.bag>", cFileName, nMaxFileNameLength, ImGuiInputTextFlags_EnterReturnsTrue)) {}
    // if (ImGui::Button("Open File")) {
    //     nfdresult_t result = NFD_OpenDialog("bag", nullptr, &cFileName);
    //     if (result == NFD_OKAY) {
    //     }
    // }

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

    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("parse bag"))
        {
            //std::string fileName = "D:/dataset/lab/c2_lvi/20230630-obs-lvi.bag";
            std::string fileName = "/home/channy/Documents/datasets/dataset_reconstruct/rosbag/20230607lvi.bag";
            // std::string fileName = "D:/dataset/lab/c2_lvi/20230630-object-lvi.bag";

            if (ImGui::Button("parse header")) {
                ParseBag::getInstance()->parseBag(fileName);
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
