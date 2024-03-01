#include "ImguiMainPage.h"

#include "osgManager.h"
#include "MeshPostProcessing.h"

GLuint textureID;

ImguiMainPage::ImguiMainPage() {
    cFileName = new char[MAXN_FILE_LEN];
    memset(cFileName, 0, MAXN_FILE_LEN);
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
    
    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    {
        //if (ImGui::InputTextWithHint("file", "<.obj .ply .xyz>", cFileName, MAXN_FILE_LEN, ImGuiInputTextFlags_EnterReturnsTrue)) {
        //}
        //ImGui::SameLine();
        
        if (ImGui::BeginTabItem("Unwrap"))
        {
            if (ImGui::Button("show charts")) {
                MeshPostProcessing::getInstance()->sFileName = "E:/projects/osgReconMesh/tmp.obj"; //cFileName;
                MeshPostProcessing::getInstance()->unwrap();

                if (MeshPostProcessing::getInstance()->bProcessingFinish)
                {
                    textureID = OsgManager::getInstance()->genTexture(MeshPostProcessing::getInstance()->chartsTextureData.data(), MeshPostProcessing::getInstance()->uvWidth, MeshPostProcessing::getInstance()->uvHeight);
                }
            }

            if (MeshPostProcessing::getInstance()->bProcessingFinish) {
                ImTextureID my_tex_id = (GLuint*)textureID;
                float my_tex_w = MeshPostProcessing::getInstance()->uvWidth;
                float my_tex_h = MeshPostProcessing::getInstance()->uvHeight;
                ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h));
            }

            ImGui::EndTabItem();
        }
    }
    ImGui::End();
}
