#include "ImguiMainPage.h"

#include "osgManager.h"
#include "MeshPostProcessing.h"

GLuint textureID;

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
    ImGui::Begin("3d reconstruction");
    
    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    {
        if (ImGui::InputTextWithHint("file", "<.obj .ply .xyz>", cFileName, MAXN_FILE_LEN, ImGuiInputTextFlags_EnterReturnsTrue)) {
        }
        ImGui::SameLine();
        if (ImGui::Button("Open File")) {
            nfdresult_t result = NFD_OpenDialog(""/*"obj,ply,xyz,csv"*/, nullptr, &cFileName);
            if (result == NFD_OKAY) {

            }
        }
        
        if (ImGui::BeginTabItem("Unwrap"))
        {
            if (ImGui::Button("show charts")) {
                MeshPostProcessing::getInstance()->sFileName = "../data/ply_ext/scene_dense_mesh_03-02-01.ply"; //cFileName;
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
        ImGui::EndTabBar();
    }
    ImGui::End();
}
