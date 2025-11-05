#include "ImguiMainPage.h"

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
    if (ImGui::InputTextWithHint("file", "<.obj .ply .xyz>", cFileName, nMaxFileNameLength, ImGuiInputTextFlags_EnterReturnsTrue)) {
    }
    ImGui::SameLine();
    if (ImGui::Button("Open File")) {
        nfdresult_t result = NFD_OpenDialog(""/*"obj,ply,xyz,csv"*/, nullptr, &cFileName);
        if (result == NFD_OKAY) {
        }
    }

    if (ImGui::Button("Switch Scene")) {
        OsgManager::getInstance()->switchScene();
    }

    if (ImGui::Button("reset scene")) {
        OsgManager::getInstance()->clear();
    }

    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("show file model"))
        {
            if (ImGui::Button("calc normal")) {
                OsgManager::getInstance()->calcObjNormal(cFileName);
            }
            if (ImGui::Button("combine all obj")) {
                std::string sPath = "D:/dataset/lab/lab_level_pcd/mapall_frame/";
                OsgManager::getInstance()->combineAllObj(sPath, "./", "mapall.obj");
            }
            if (ImGui::Button("transform all obj"))
            {
                OsgManager::getInstance()->transformObj("D:/dataset/global_frame_obj/", "pose_scanner_leica.csv", "E:/pythonProjects/puma-lidar-modify/data/global-frame/transform/");
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

}
