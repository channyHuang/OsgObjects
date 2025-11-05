#include "ImguiMainPage.h"

#include "osgManager.h"

void pickCbFunc(const osg::Vec3& vPos, void* pUser) {
	OsgManager::getInstance()->showPick(vPos);
}

ImguiMainPage::ImguiMainPage() {}

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
    ImGui::Begin("colmap recon camera params");
    if (ImGui::Button("Switch Scene")) {
        OsgManager::getInstance()->switchScene();
    }

    // if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    // {
    //     if (ImGui::BeginTabItem("show"))
    //     {
    //         if (ImGui::Button("showFrame")) {
    //             OsgManager::getInstance()->showFrame();
    //         }
    //         if (ImGui::Button("showPointAndCamera")) {
    //             OsgManager::getInstance()->showPointAndCamera();
    //         }
    //         if (ImGui::Button("showGrid")) {
    //             OsgManager::getInstance()->showGrid();
    //         }
    //         if (ImGui::Button("combine")) {
    //             OsgManager::getInstance()->combineCamera();
    //         }
    //         if (ImGui::Button("combineAndWrite")) {
    //             OsgManager::getInstance()->combineAndWrite();
    //         }
    //         ImGui::EndTabItem();
    //     }

    //     if (ImGui::BeginTabItem("test show model"))
    //     {
    //         ImGui::EndTabItem();
    //     }

    //     ImGui::EndTabBar();
    // }
    
    ImGui::End();
}
