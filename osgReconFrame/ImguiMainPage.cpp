#include "ImguiMainPage.h"

#include "osgManager.h"

ImguiMainPage::ImguiMainPage() {
    
}

ImguiMainPage::ImguiMainPage(osgViewer::Viewer& viewer) {
    pviewer = &viewer;

    OsgManager::getInstance()->setViewer(viewer);
}

ImguiMainPage::~ImguiMainPage() {
    pviewer = nullptr;
}

void ImguiMainPage::drawUi() {
    ImGui::Begin("3d reconstruction");
    if (ImGui::Button("Switch Scene")) {
    }
    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("recon frame"))
        {
            if (ImGui::Button("showDataset")) {
                bShowDataset = !bShowDataset;
            }
            if (ImGui::Button("show")) {
                OsgManager::getInstance()->show("E:/frame");
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    if (bShowDataset) {

    }

}
