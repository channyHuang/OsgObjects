#include "ImguiMainPage.h"

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>

GLuint textureID;

#include "commonOsg/commonOsg.h"

#include "nativefiledialog/nfd.h"

void pickCbFunc(const osg::Vec3& vPos, void* pUser) {
	OsgManager::getInstance()->showPick(vPos);
}

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

    m_pPicker = new PickHandler();
    m_pPicker->setCallback(pickCbFunc, nullptr);
    viewer.addEventHandler(m_pPicker);
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
    // common functions
    if (ImGui::Button("Switch Scene")) {
        OsgManager::getInstance()->switchScene();
    }

    // select file or folder 文件选择框
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
    if (ImGui::Button("Reset Scene")) {
        m_pCameraHandler->reset();
    }
    if (ImGui::Button("Switch Scene")) {
        OsgManager::getInstance()->switchScene();
    }
    ImGui::Checkbox("Picker valid?", &m_pPicker->m_bCheckHit);

    ImGui::Checkbox("Back Scene To World Center", &m_pCameraHandler->m_bBack2WorldCenter);
    if (m_pCameraHandler->m_bBack2WorldCenter) {
        m_pCameraHandler->back2WorldCenter();
        m_pCameraHandler->m_bBack2WorldCenter = false;
    }
    ImGui::SliderFloat("step", &m_pCameraHandler->m_fStepScale, 0.01f, 3.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

    if (ImGui::Button("clear pick")) {
        OsgManager::getInstance()->clearPick();
    }
    if (ImGui::BeginTabBar("Functions", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("show models")) // show obj models
        {
            if (ImGui::Button("show model using TinyObj")) {
                sFileName = std::string(cFileName);
                if (sFileName.length() > 0) {
                    OsgManager::getInstance()->readObjTinyobj(sFileName, std::string(cTexturePath), false);
                }
            }

            if (ImGui::Button("show model using osg")) {
                sFileName = std::string(cFileName);
                if (sFileName.length() > 0) {
                    OsgManager::getInstance()->readNode(sFileName);
                }
            }

            if (ImGui::Button("show model osgb with Lod")) {
                sFileName = std::string(cFileName);
                if (sFileName.length() <= 0) sFileName = "D:/dataset/r3live-lab-res/sub_mesh.obj";
                OsgManager::getInstance()->readOsgbLOD(sFileName, std::string(cTexturePath));
            }

            if (ImGui::Button("show model in folder")) {
                sFileName = std::string(cFileName);
                if (sFileName.length() <= 0) sFileName = "/home/channy/Documents/datasets_recon/dolphin_wall/tex_mesh";
                OsgManager::getInstance()->loadPlyFolder(sFileName);
                printf("show folder %s!\n", sFileName.c_str());                
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Other functions")) {
            if (ImGui::Button("calc area")) {
                // float res = calcObjProjectArea(cFileName, cTexturePath);
                // std::cout << "area " << res << std::endl;
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("format transform (obj 2 osgt)")) {
            if (ImGui::Button("import obj and transform")) {
                sFileName = std::string(cFileName);
                if (sFileName.length() > 0) {
                    OsgManager::getInstance()->obj2osgt(sFileName);
                }
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
