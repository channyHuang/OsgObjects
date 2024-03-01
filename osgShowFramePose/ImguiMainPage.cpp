#include "ImguiMainPage.h"

#include "osgManager.h"

GLuint textureID;

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
    ImGui::Begin("colmap recon camera params");
    if (ImGui::Button("Switch Scene")) {
    }
    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("show"))
        {
            if (ImGui::Button("showFrame")) {
                OsgManager::getInstance()->showFrame();
            }
            if (ImGui::Button("showPointAndCamera")) {
                OsgManager::getInstance()->showPointAndCamera();
            }
            if (ImGui::Button("showGrid")) {
                OsgManager::getInstance()->showGrid();
            }
            if (ImGui::Button("combine")) {
                OsgManager::getInstance()->combineCamera();
            }
            if (ImGui::Button("combineAndWrite")) {
                OsgManager::getInstance()->combineAndWrite();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("test show model"))
        {
            

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}
