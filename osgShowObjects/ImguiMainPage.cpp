#include "ImguiMainPage.h"

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>

GLuint textureID;

#include "commonOsg/commonOsg.h"

#include "nativefiledialog/nfd.h"

ImguiMainPage::ImguiMainPage() {
    
}

ImguiMainPage::ImguiMainPage(osgViewer::Viewer& viewer, osg::ref_ptr< CameraHandler> pCameraHandler) {
    pviewer = &viewer;
    m_pCameraHandler = pCameraHandler;
    OsgManager::getInstance()->setViewer(viewer);
    cFileName = new char[nMaxFileNameLength];
    memset(cFileName, 0, nMaxFileNameLength);
    cTexturePath = new char[nMaxFileNameLength];
    memset(cTexturePath, 0, nMaxFileNameLength);

    picker = new PickDistanceHandler();
    viewer.addEventHandler(picker);
}

ImguiMainPage::~ImguiMainPage() {
    pviewer = nullptr;
    if (cFileName != nullptr) {
        delete[]cFileName;
    }
    if (cTexturePath != nullptr) {
        delete[]cTexturePath;
    }
}

void ImguiMainPage::drawUi() {
    ImGui::Begin("osg show objects");
    if (ImGui::InputTextWithHint("file", "<.obj .ply .xyz>", cFileName, nMaxFileNameLength, ImGuiInputTextFlags_EnterReturnsTrue)) {
    }
    ImGui::SameLine();
    if (ImGui::Button("Open File")) {
        nfdresult_t result = NFD_OpenDialog(""/*"obj,ply,xyz,csv"*/, nullptr, &cFileName);
        if (result == NFD_OKAY) {

        }
    }
    if (ImGui::InputTextWithHint("path", "", cTexturePath, nMaxFileNameLength, ImGuiInputTextFlags_EnterReturnsTrue)) {
    }
    ImGui::SameLine();
    if (ImGui::Button("Open Path")) {
        nfdresult_t result = NFD_OpenDialog(""/*"obj,ply,xyz,csv"*/, nullptr, &cTexturePath);
        if (result == NFD_OKAY) {

        }
    }

    if (ImGui::Button("Switch Scene")) {
        OsgManager::getInstance()->switchScene();
    }
    ImGui::Checkbox("Picker valid?", &picker->valid);

    ImGui::Checkbox("Rotate By Axis", &m_pCameraHandler->bRotateByAxis);
    if (m_pCameraHandler->bRotateByAxis) {
        ImGui::Text("Rotate by axis: 0-x, 1-y, 2-z");
        ImGui::SliderInt("Axis x y z", &m_pCameraHandler->axis, 0, 3);
    }

    if (ImGui::Button("clear pick")) {
        OsgManager::getInstance()->clearPick();
    }
    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("show obj models")) // show obj models
        {
            if (ImGui::Button("show model using glsl")) {
                sFileName = std::string(cFileName);
                if (sFileName.length() <= 0) sFileName = "D:/dataset/r3live-lab-res/sub_mesh.obj";
                OsgManager::getInstance()->showModelObj(sFileName, std::string(cTexturePath), false);
            }

            if (ImGui::Button("show model in osg (prefer)")) {
                sFileName = std::string(cFileName);
                if (sFileName.length() <= 0) sFileName = "D:/dataset/r3live-lab-res/sub_mesh.obj";
                OsgManager::getInstance()->showModelObj(sFileName, std::string(cTexturePath));
            }

            if (ImGui::Button("show model osgb with Lod")) {
                sFileName = std::string(cFileName);
                if (sFileName.length() <= 0) sFileName = "D:/dataset/r3live-lab-res/sub_mesh.obj";
                OsgManager::getInstance()->showModelOsgbLOD(sFileName, std::string(cTexturePath));
            }

            if (ImGui::Button("show model using assimp")) {
                sFileName = std::string(cFileName);
                if (sFileName.length() <= 0) sFileName = "D:/dataset/r3live-lab-res/sub_mesh.obj";
                OsgManager::getInstance()->showModelUsingAssimp(sFileName, std::string(cTexturePath));
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Other functions")) {
            if (ImGui::Button("calc area")) {
                float res = calcObjProjectArea(cFileName, cTexturePath);
                std::cout << "area " << res << std::endl;
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("format transform (obj 2 osgt)")) {
            if (ImGui::Button("import obj and transform")) {
                sFileName = std::string(cFileName);
                if (sFileName.length() <= 0) sFileName = "D:/dataset/r3live-lab-res/sub_mesh.obj";
                OsgManager::getInstance()->obj2osgt(sFileName);
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
